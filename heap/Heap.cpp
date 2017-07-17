#include <queue>
#include <vector>
#include <iostream>

using namespace std;
#if 0
namespace leviTest{
    template<class T>
    struct TreeNode{
        T val; //index
        TreeNode<T> *left;
        TreeNode<T> *right;
    };

    template<class T, class _comp>
    class HuffmanTree{
        priority_queue<T, vector<T>, _comp> pq;

        TreeNode<T>* mergeNode(TreeNode<T> *left, TreeNode<T> *right){
            TreeNode<T> *ret = new TreeNode<T>();

            ret->left = left;
            ret->right = right;

            return ret;
        }

        TreeNode<T>* newLeaf(T val){
            TreeNode<T> *ret = new TreeNode<T>();

            ret->val = val;
            ret->left = NULL;
            ret->right = NULL;

            return ret;
        }

        TreeNode<T>* makeTree(){
        }
    };
};
#endif

template<class T>
struct TreeNode{
    T val; //index
    TreeNode<T> *left;
    TreeNode<T> *right;
};

template<class T>
TreeNode<T>* mergeNode(TreeNode<T> *left, TreeNode<T> *right){
    TreeNode<T> *ret = new TreeNode<T>();

    ret->val = 0;
    ret->left = left;
    ret->right = right;

    return ret;
}

template<class T>
TreeNode<T>* newLeaf(T val){
    TreeNode<T> *ret = new TreeNode<T>();

    ret->val = val;
    ret->left = NULL;
    ret->right = NULL;

    return ret;
}

void travalTree(TreeNode<unsigned int> *root, bool right, int height){
    if(!root) return;
    
    travalTree(root->right, true, height + 1);
    if(height){
        if(right){
            printf("%*d\n", height * 2 + 1, root->val);
            printf("%*c\n", height * 2, '/');
        }else{
            printf("%*c\n", height * 2, '\\');
            printf("%*d\n", height * 2 + 1, root->val);
        }
    }else{
        printf("*\n");
    }

    travalTree(root->left, false, height + 1);
}

int main(){
    char freq[256];
    struct Node{
        Node(){};
        Node(char index, unsigned int freq):index(index), freq(freq)
        {
            leafNode = newLeaf<unsigned int>(index);
        }
        Node(unsigned int freq, TreeNode<unsigned int> *leafNode):freq(freq), leafNode(leafNode){
        }
        Node(const Node& other){
            index = other.index;
            freq = other.freq;
            leafNode = newLeaf<unsigned int>(other.leafNode->val);
        }
        void operator=(const Node& other){
            index = other.index;
            freq = other.freq;
            leafNode = newLeaf<unsigned int>(other.leafNode->val);
        }
        ~Node(){delete leafNode;}
        char index;
        unsigned int freq;
        TreeNode<unsigned int> *leafNode;
    };
    struct Cmp{
        bool operator()(Node* &a, Node* &b){
            return a->freq > b->freq;
        }
    };
    std::priority_queue<Node*, vector<Node*>, Cmp> pq;
    pq.push(new Node(1, 3));
    pq.push(new Node(2, 7));
    pq.push(new Node(3, 1));
    pq.push(new Node(4, 8));
    pq.push(new Node(5, 6));
    pq.push(new Node(6, 4));

    TreeNode<unsigned int> *node = NULL;

    while(!pq.empty()){
        //cout<< (int)pq.top().index << endl;
        Node *left, *right;

        left = pq.top();
        pq.pop();
        if(!pq.empty()){
            right = pq.top();
            pq.pop();
    
            //printf("%d, %d\n", left->freq, right->freq);
            node = mergeNode(left->leafNode, right->leafNode);
            //printf("node: %p, node->val: %d\n", node, node->val);

            pq.push(new Node(left->freq + right->freq, node));
            //printf("node: %p, node->val: %d\n", node, node->val);
        }
    }
    
    travalTree(node, 0, 0);
}
