#include <bits/stdc++.h>

using namespace std;

int sz;
int pre[50], post[50];
int arr[10000];

// A recursive function to construct Full from pre[] and post[].
// preIndex is used to keep track of index in pre[].
// l is low index and h is high index for the current subarray in post[]
int constructTreeUtil (int ind,  int* preIndex, int l, int h) {
    // Base case
    if (*preIndex >= sz || l > h)
        return -1;
    // The first node in preorder traversal is root. So take the node at
    // preIndex from preorder and make it root, and increment preIndex
    arr[ind] =  pre[*preIndex];
    ++*preIndex;
    // If the current subarry has only one element, no need to recur
    if (l == h)
        return arr[ind];
    // Search the next element of pre[] in post[]
    int i;
    for (i = l; i <= h; ++i)
        if (pre[*preIndex] == post[i])
            break;
    // Use the index of element found in postorder to divide postorder array in
    // two parts. Left subtree and right subtree
    if (i <= h)
    {
        arr[2*ind] = constructTreeUtil (2*ind,  preIndex, l, i);
        arr[2*ind+1] = constructTreeUtil (2*ind+1, preIndex, i + 1, h-1);
    }
    return arr[ind];
}

// The main function to construct Full Binary Tree from given preorder and
// postorder traversals. This function mainly uses constructTreeUtil()
int constructTree ()
{
    int preIndex = 1;
    arr[1] = constructTreeUtil (1, &preIndex, 1, sz-1);
}

void printInorder (int n)
{
    if (n >= sz || arr[n] <= 0)
        return;
    printInorder(2*n);
    printf("%d ", arr[n]);
    printInorder(2*n+1);
}

int main () {
    memset(pre, -1, sizeof(pre));
    memset(post, -1, sizeof(post));
{
    string line;
    getline(cin, line);
    istringstream iss(line);
    int next;sz = 1;
    while(iss >> next){
        pre[sz++] = next;
    }
}
{
    string line;
    getline(cin, line);
    istringstream iss(line);
    int next;sz = 1;
    while(iss >> next){
        post[sz++] = next;
    }
}
    cout << constructTree() << endl;
    for(int i = 0; i <= sz; i++) printf("%d ", arr[i]);
    puts("");
    printInorder(1);

    return 0;
}
