#include <iostream>
using namespace std;

template <typename Type, int Ord>
struct Node {
  bool is_leaf;
  int num_keys;
  Type key[Ord];

  Node(bool leaf = false) {
    is_leaf = leaf;
    num_keys = 0;
  }

  virtual ~Node() {
  }
};

template <typename Type, int Ord>
struct LeafNode : public Node<Type, Ord> {
  LeafNode() : Node<Type, Ord>(true) {
  }
};

template <typename Type, int Ord>
struct InternalNode : public Node<Type, Ord> {
  Node<Type, Ord>* child[Ord + 1];

  InternalNode() : Node<Type, Ord>(false) {
    for (int i = 0; i < Ord + 1; i++) {
      child[i] = nullptr;
    }
  }

  virtual ~InternalNode() {
  }
};

template <typename Type, int Ord>
class BTree {
  static_assert(Ord >= 3, "BTree order must be at least 3");

private:
  Node<Type, Ord>* root;

  int getMinKeys() const {
    return (Ord + 1) / 2 - 1;
  }

  bool searchHelper(Node<Type, Ord>* node, const Type& key) const {
    if (node == nullptr) {
      return false;
    }

    int i = 0;
    while (i < node->num_keys && key > node->key[i]) {
      i++;
    }

    if (i < node->num_keys && key == node->key[i]) {
      return true;
    }

    if (node->is_leaf) {
      return false;
    }

    auto internal = static_cast<InternalNode<Type, Ord>*>(node);
    return searchHelper(internal->child[i], key);
  }

  void displayDFSHelper(Node<Type, Ord>* node) const {
    if (node == nullptr) {
      return;
    }

    if (node->is_leaf) {
      for (int i = 0; i < node->num_keys; i++) {
        cout << node->key[i] << " ";
      }
    } else {
      auto internal = static_cast<InternalNode<Type, Ord>*>(node);
      for (int i = 0; i < node->num_keys; i++) {
        displayDFSHelper(internal->child[i]);
        cout << node->key[i] << " ";
      }
      displayDFSHelper(internal->child[node->num_keys]);
    }
  }

  void splitNode(Node<Type, Ord>* node, Type& promotedKey, Node<Type, Ord>*& rightChild) {
    int midIndex = Ord / 2;
    promotedKey = node->key[midIndex];

    if (node->is_leaf) {
      LeafNode<Type, Ord>* newLeaf = new LeafNode<Type, Ord>();
      int newKeysCount = 0;

      for (int i = midIndex + 1; i < Ord; i++) {
        newLeaf->key[newKeysCount] = node->key[i];
        newKeysCount++;
      }

      newLeaf->num_keys = newKeysCount;
      node->num_keys = midIndex;
      rightChild = newLeaf;
    } else {
      InternalNode<Type, Ord>* newInternal = new InternalNode<Type, Ord>();
      auto internal = static_cast<InternalNode<Type, Ord>*>(node);
      int newKeysCount = 0;

      for (int i = midIndex + 1; i < Ord; i++) {
        newInternal->key[newKeysCount] = internal->key[i];
        newInternal->child[newKeysCount] = internal->child[i];
        newKeysCount++;
      }

      newInternal->child[newKeysCount] = internal->child[Ord];
      newInternal->num_keys = newKeysCount;
      internal->num_keys = midIndex;
      rightChild = newInternal;
    }
  }

  bool insertHelper(Node<Type, Ord>* node, const Type& key, Type& promotedKey, Node<Type, Ord>*& rightChild) {
    if (node->is_leaf) {
      int i = node->num_keys - 1;
      while (i >= 0 && node->key[i] > key) {
        node->key[i + 1] = node->key[i];
        i--;
      }

      node->key[i + 1] = key;
      node->num_keys++;

      if (node->num_keys == Ord) {
        splitNode(node, promotedKey, rightChild);
        return true;
      }
      return false;
    } else {
      auto internal = static_cast<InternalNode<Type, Ord>*>(node);
      int i = 0;
      while (i < node->num_keys && key > internal->key[i]) {
        i++;
      }

      Type pKey;
      Node<Type, Ord>* rChild = nullptr;
      bool wasSplit = insertHelper(internal->child[i], key, pKey, rChild);

      if (wasSplit) {
        int j = internal->num_keys - 1;
        while (j >= i) {
          internal->key[j + 1] = internal->key[j];
          internal->child[j + 2] = internal->child[j + 1];
          j--;
        }

        internal->key[i] = pKey;
        internal->child[i + 1] = rChild;
        internal->num_keys++;

        if (internal->num_keys == Ord) {
          splitNode(internal, promotedKey, rightChild);
          return true;
        }
      }
      return false;
    }
  }

  Type getPredecessor(Node<Type, Ord>* node) {
    Node<Type, Ord>* curr = node;
    while (!curr->is_leaf) {
      auto internal = static_cast<InternalNode<Type, Ord>*>(curr);
      curr = internal->child[curr->num_keys];
    }
    return curr->key[curr->num_keys - 1];
  }

  void borrowFromLeft(InternalNode<Type, Ord>* parent, int idx) {
    auto child = parent->child[idx];
    auto leftSibling = parent->child[idx - 1];

    for (int i = child->num_keys - 1; i >= 0; i--) {
      child->key[i + 1] = child->key[i];
    }

    if (!child->is_leaf) {
      auto cInternal = static_cast<InternalNode<Type, Ord>*>(child);
      auto lInternal = static_cast<InternalNode<Type, Ord>*>(leftSibling);
      for (int i = child->num_keys; i >= 0; i--) {
        cInternal->child[i + 1] = cInternal->child[i];
      }
      cInternal->child[0] = lInternal->child[leftSibling->num_keys];
    }

    child->key[0] = parent->key[idx - 1];
    parent->key[idx - 1] = leftSibling->key[leftSibling->num_keys - 1];

    child->num_keys++;
    leftSibling->num_keys--;
  }

  void borrowFromRight(InternalNode<Type, Ord>* parent, int idx) {
    auto child = parent->child[idx];
    auto rightSibling = parent->child[idx + 1];

    child->key[child->num_keys] = parent->key[idx];

    if (!child->is_leaf) {
      auto cInternal = static_cast<InternalNode<Type, Ord>*>(child);
      auto rInternal = static_cast<InternalNode<Type, Ord>*>(rightSibling);
      cInternal->child[child->num_keys + 1] = rInternal->child[0];

      for (int i = 0; i < rightSibling->num_keys - 1; i++) {
        rInternal->child[i] = rInternal->child[i + 1];
      }
      rInternal->child[rightSibling->num_keys - 1] = rInternal->child[rightSibling->num_keys];
    }

    parent->key[idx] = rightSibling->key[0];

    for (int i = 0; i < rightSibling->num_keys - 1; i++) {
      rightSibling->key[i] = rightSibling->key[i + 1];
    }

    child->num_keys++;
    rightSibling->num_keys--;
  }

  void merge(InternalNode<Type, Ord>* parent, int idx) {
    auto leftChild = parent->child[idx];
    auto rightChild = parent->child[idx + 1];

    leftChild->key[leftChild->num_keys] = parent->key[idx];
    leftChild->num_keys++;

    for (int i = 0; i < rightChild->num_keys; i++) {
      leftChild->key[leftChild->num_keys] = rightChild->key[i];
      leftChild->num_keys++;
    }

    if (!leftChild->is_leaf) {
      auto lInternal = static_cast<InternalNode<Type, Ord>*>(leftChild);
      auto rInternal = static_cast<InternalNode<Type, Ord>*>(rightChild);
      int childInsertIdx = leftChild->num_keys - rightChild->num_keys;

      for (int i = 0; i <= rightChild->num_keys; i++) {
        lInternal->child[childInsertIdx + i] = rInternal->child[i];
      }
    }

    for (int i = idx; i < parent->num_keys - 1; i++) {
      parent->key[i] = parent->key[i + 1];
      parent->child[i + 1] = parent->child[i + 2];
    }

    parent->num_keys--;
    delete rightChild;
  }

  void handleUnderflow(InternalNode<Type, Ord>* parent, int idx) {
    int minK = getMinKeys();

    if (idx > 0 && parent->child[idx - 1]->num_keys > minK) {
      borrowFromLeft(parent, idx);
    } else if (idx < parent->num_keys && parent->child[idx + 1]->num_keys > minK) {
      borrowFromRight(parent, idx);
    } else {
      if (idx < parent->num_keys) {
        merge(parent, idx);
      } else {
        merge(parent, idx - 1);
      }
    }
  }

  bool deleteHelper(Node<Type, Ord>* node, const Type& k) {
    int idx = 0;
    while (idx < node->num_keys && k > node->key[idx]) {
      idx++;
    }

    if (node->is_leaf) {
      if (idx < node->num_keys && node->key[idx] == k) {
        for (int i = idx; i < node->num_keys - 1; i++) {
          node->key[i] = node->key[i + 1];
        }
        node->num_keys--;
        return true;
      }
      return false;
    } else {
      auto internal = static_cast<InternalNode<Type, Ord>*>(node);
      bool deleted = false;

      if (idx < node->num_keys && node->key[idx] == k) {
        Type predKey = getPredecessor(internal->child[idx]);
        node->key[idx] = predKey;
        deleted = deleteHelper(internal->child[idx], predKey);
      } else {
        deleted = deleteHelper(internal->child[idx], k);
      }

      if (deleted) {
        if (internal->child[idx]->num_keys < getMinKeys()) {
          handleUnderflow(internal, idx);
        }
      }
      return deleted;
    }
  }

  void clearMemory(Node<Type, Ord>* node) {
    if (node == nullptr) {
      return;
    }

    if (!node->is_leaf) {
      auto internal = static_cast<InternalNode<Type, Ord>*>(node);
      for (int i = 0; i <= node->num_keys; i++) {
        clearMemory(internal->child[i]);
      }
    }
    delete node;
  }

public:
  BTree() {
    root = nullptr;
  }

  BTree(const BTree&) = delete;
  BTree& operator=(const BTree&) = delete;

  ~BTree() {
    clearMemory(root);
  }

  void insert(const Type& key) {
    if (root == nullptr) {
      root = new LeafNode<Type, Ord>();
      root->key[0] = key;
      root->num_keys = 1;
      return;
    }

    Type promotedKey;
    Node<Type, Ord>* rightChild = nullptr;
    bool wasSplit = insertHelper(root, key, promotedKey, rightChild);

    if (wasSplit) {
      InternalNode<Type, Ord>* newRoot = new InternalNode<Type, Ord>();
      newRoot->key[0] = promotedKey;
      newRoot->child[0] = root;
      newRoot->child[1] = rightChild;
      newRoot->num_keys = 1;
      root = newRoot;
    }
  }

  bool search(const Type& key) const {
    return searchHelper(root, key);
  }

  void displayDFS() const {
    displayDFSHelper(root);
    cout << endl;
  }

  void remove(const Type& key) {
    while (root != nullptr && search(key)) {
      deleteHelper(root, key);

      if (root->num_keys == 0) {
        Node<Type, Ord>* oldRoot = root;
        if (root->is_leaf) {
          root = nullptr;
        } else {
          auto internal = static_cast<InternalNode<Type, Ord>*>(root);
          root = internal->child[0];
        }
        delete oldRoot;
      }
    }
  }
};
int main() {
  BTree<int, 3> intTree;

  intTree.insert(10);
  intTree.insert(20);
  intTree.insert(5);
  intTree.insert(6);
  intTree.insert(12);
  intTree.insert(30);
  intTree.insert(7);
  intTree.insert(17);

  intTree.insert(10);
  intTree.insert(10);
  intTree.insert(6);

  cout << "=== Integer B-Tree ===" << endl;
  cout << "Initial Traversal: ";
  intTree.displayDFS();

  cout << "Search 6 (Exists): " << (intTree.search(6) ? "Found" : "Not Found") << endl;
  cout << "Search 10 (Duplicate Exists): " << (intTree.search(10) ? "Found" : "Not Found") << endl;
  cout << "Search 99 (Not Exists): " << (intTree.search(99) ? "Found" : "Not Found") << endl;

  intTree.remove(12);
  cout << "After removing non-duplicate key 12: ";
  intTree.displayDFS();

  intTree.remove(6);
  cout << "After removing duplicate key 6 (All instances): ";
  intTree.displayDFS();

  intTree.remove(10);
  cout << "After removing duplicate key 10 (All instances): ";
  intTree.displayDFS();

  intTree.remove(99);
  cout << "After attempting to remove non-existing key 99: ";
  intTree.displayDFS();

  BTree<char, 3> charTree;

  charTree.insert('M');
  charTree.insert('K');
  charTree.insert('A');
  charTree.insert('F');
  charTree.insert('Z');
  charTree.insert('P');

  charTree.insert('K');
  charTree.insert('K');
  charTree.insert('F');

  cout << "\n=== Char B-Tree ===" << endl;
  cout << "Initial Traversal: ";
  charTree.displayDFS();

  cout << "Search 'F' (Exists): " << (charTree.search('F') ? "Found" : "Not Found") << endl;
  cout << "Search 'K' (Duplicate Exists): " << (charTree.search('K') ? "Found" : "Not Found") << endl;
  cout << "Search 'X' (Not Exists): " << (charTree.search('X') ? "Found" : "Not Found") << endl;

  charTree.remove('P');
  cout << "After removing non-duplicate key 'P': ";
  charTree.displayDFS();

  charTree.remove('K');
  cout << "After removing duplicate key 'K' (All instances): ";
  charTree.displayDFS();

  charTree.remove('X');
  cout << "After attempting to remove non-existing key 'X': ";
  charTree.displayDFS();

  cout<<endl;

  BTree<int, 3> uniqueTree;

  uniqueTree.insert(10);
  uniqueTree.insert(20);
  uniqueTree.insert(5);
  uniqueTree.insert(6);
  uniqueTree.insert(12);
  uniqueTree.insert(30);
  uniqueTree.insert(7);
  uniqueTree.insert(17);

  cout << "=== Unique Keys Integer B-Tree ===" << endl;
  cout << "Initial Traversal: ";
  uniqueTree.displayDFS();

  cout << "Search 12 (Exists): " << (uniqueTree.search(12) ? "Found" : "Not Found") << endl;
  cout << "Search 99 (Not Exists): " << (uniqueTree.search(99) ? "Found" : "Not Found") << endl;

  uniqueTree.remove(6);
  cout << "After removing leaf key 6: ";
  uniqueTree.displayDFS();

  uniqueTree.remove(20);
  cout << "After removing internal key 20: ";
  uniqueTree.displayDFS();

  uniqueTree.remove(99);
  cout << "After attempting to remove non-existing key 99: ";
  uniqueTree.displayDFS();

  BTree<char, 3> uniqueCharTree;

  uniqueCharTree.insert('M');
  uniqueCharTree.insert('K');
  uniqueCharTree.insert('A');
  uniqueCharTree.insert('F');
  uniqueCharTree.insert('Z');
  uniqueCharTree.insert('P');
  uniqueCharTree.insert('B');
  uniqueCharTree.insert('R');

  cout << "\n=== Unique Keys Char B-Tree ===" << endl;
  cout << "Initial Traversal: ";
  uniqueCharTree.displayDFS();

  cout << "Search 'K' (Exists): " << (uniqueCharTree.search('K') ? "Found" : "Not Found") << endl;
  cout << "Search 'X' (Not Exists): " << (uniqueCharTree.search('X') ? "Found" : "Not Found") << endl;

  uniqueCharTree.remove('P');
  cout << "After removing leaf key 'P': ";
  uniqueCharTree.displayDFS();

  uniqueCharTree.remove('K');
  cout << "After removing internal key 'K': ";
  uniqueCharTree.displayDFS();

  uniqueCharTree.remove('X');
  cout << "After attempting to remove non-existing key 'X': ";
  uniqueCharTree.displayDFS();
  return 0;
}