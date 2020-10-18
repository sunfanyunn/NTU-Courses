#include <bits/stdc++.h>

using namespace std;
#define fi first
#define se second
#define pb push_back
#define mp make_pair

#define maxn
int arr[1000];
int sz = 0;

void preorder(int ind) {
    if( ind >= sz || arr[ind] < 0) return;
    printf("%d ", arr[ind]);
    preorder(ind*2);preorder(ind*2+1);
}
void inorder(int ind) {
   if( ind >= sz || arr[ind] < 0) return;
   inorder(ind*2);
   printf("%d ", arr[ind]);
   inorder(ind*2+1);
}
void postorder(int ind) {
    if( ind >= sz || arr[ind] < 0) return;
    postorder(ind*2);postorder(ind*2+1);
    printf("%d ", arr[ind]);
}
int main() {
    while(scanf("%d", &arr[sz]) == 1) {
       sz++;
    }
    preorder(1);
    puts("");
    inorder(1);
    puts("");
    postorder(1);
    puts("");
    return 0;
}
