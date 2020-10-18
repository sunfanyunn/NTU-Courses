#include <bits/stdc++.h>

using namespace std;
#define fi first
#define se second
#define pb push_back
#define mp make_pair

#define maxn
int n;
int arr[50];
void recur(int sz, int cur, stack<int> stac) {
    if( cur == n+1 || sz == n ) {
        while( !stac.empty() ) { arr[sz++] = stac.top();stac.pop();}
        for(int i = 0; i < sz; i++) printf("%d%c", arr[i], (i==sz-1)?'\n':' ');
        return;
    }
    
    stac.push(cur);
    recur(sz, cur+1, stac);
    stac.pop();
    

    if( !stac.empty() ) {
        arr[sz++] = stac.top();
        stac.pop();
        recur(sz, cur, stac);
    }
}

int main() {
    cin>>n;
    stack<int> s;
    recur(0, 1, s);
    return 0;
}
