#include <iostream>
using namespace std;

template <typename Type, int Ord>
struct Node {
  bool is_leaf;
  int num_keys;
  Type key[Ord];

  Node(bool leaf = false){
    is_leaf = leaf;
    num_keys = 0;
  }

  virtual ~Node(){
  }
};

template <typename Type, int Ord>
struct LeafNode : public Node<Type, Ord> {
  LeafNode() : Node<Type, Ord>(true){
   }
}

template <typename Type, int Ord>
struct InternalNode : public Node<Type, Ord> {
  Node<Type,Ord>* child[Ord + 1];

  InternalNode() : Node<Type, Ord>(false){
    for (int i = 0; i < Ord + 1; i++) {
      child[i] = nullptr;
    }
  }
  virtual ~InternalNode(){
  }
}

template <typename Type, int Ord>
class BTree {
  private:
    Node<Type, Ord>* root;

    int getMinKeys() const{
      int MinKeys = (Ord + 1) / 2 - 1;
      return MinKeys;
    }

    bool searchHelper(Node<Type,Ord>* node, const Type& key) const{
      if (node == nullptr) {
        return false;
      }
      if (node->is_leaf) {
        return false;
      }
      int i = 0;
      while (i < node->num_keys && k > node->key[i]) {
        i++;
      }
      if (i == node->num_keys && k == node->key[i]) {
        return true;
      }

      auto internal = static_cast<InternalNode<Type,Ord>*>(node);
      return searchHelper(internal->child[i], key);
    }

    void displayHelper(Node<Type,Ord>* node) const{
      if (node == nullptr) {
        return;
      }
      if (node->is_leaf) {
          for (int i = 0; i < node->num_keys; i++) {
            cout << node->key[i] << " ";
          }
      } else{
        auto internal = static_cast<InternalNode<Type,Ord>*>(node);
        for (int i = 0; i < node->num_keys; i++) {
            displayHelper(internal->child[i]);
            cout << node->key[i] << " ";
        }
        displayHelper(internal->child[node->num_keys]);
      }
     void splitNode(Node<Type,Ord>* node, Type& promotedKey, Node<Type, Ord>*& rightChild){
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
       }else{
         InternalNode<Type, Ord>* newInternal = new InternalNode<Type, Ord>();
         auto internal = static_cast<InternalNode<Type, Ord>*>(node);
         int newKeysCount = 0;

         for (int i = midIndex + 1; i < Ord; i++) {
           newInternal->key[newKeysCount] = node->key[i];
           newInternal->child[newKeysCount] = internal->child[i];
           newKeysCount++;
         }
         newInternal->child[newKeysCount] = inernal->child[Ord];
         newInternal->num_keys = newKeysCount;
         internal->num_keys = midIndex;
         rightChild = newInternal;
       }
      }
      bool insertHelper(Node<Type,Ord>* node, const Type& key, Type& promotedKey, Node<Type, Ord>*& rightChild){
        if (node->is_leaf) {
          int i = node->num_keys - 1;
          while(i >= 0 && node->key[i] > key){
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
        }else{
          auto internal = static_cast<InternalNode<Type, Ord>*>(node);
          int i = 0;
          while (i < node->num_keys && k > internal->key[i]) {
            i++;
          }

          Type pKey;
          Node<Type, Ord>* rChile = nullptr;
          bool wasSplit = insertHelper(internal->child[i], key, pKey, rChild);

          if (wasSplit) {
            int j = internal->num_keys - 1;
            while(j >= i){
              internal->key[j + 1] = internal->key[j];
              internal->child[j + 2] = internal->child[j + 1];
              j--;
            }

            internal->key[i] = pKey;
            internal->child[i+1] = rChild;
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
         std::cout << std::endl;
       }

      void remove(const Type& key) {
         if (root == nullptr) {
           return;
         }

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
};

  int main() {
    BTree<int, 3> tree;

    tree.insert(10);
    tree.insert(20);
    tree.insert(5);
    tree.insert(6);
    tree.insert(12);
    tree.insert(30);
    tree.insert(7);
    tree.insert(17);

    std::cout << "Traversal after insertions: ";
    tree.displayDFS();

    std::cout << "Searching for 6: " << (tree.search(6) ? "Found" : "Not Found") << std::endl;
    std::cout << "Searching for 15: " << (tree.search(15) ? "Found" : "Not Found") << std::endl;

    tree.remove(6);
    std::cout << "Traversal after removing 6: ";
    tree.displayDFS();

    tree.remove(13);
    std::cout << "Traversal after attempting to remove 13: ";
    tree.displayDFS();

    return 0;
  }