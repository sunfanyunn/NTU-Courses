#include <cstdio>
#include <vector>
#include <cassert>
#include <iostream>
#include <limits>
#include <algorithm>
#include <cmath>
#include <cstring>
#define pb push_back
using namespace std;
#define maxm 1005
#define maxn 105
long double dp[maxm][maxn];
int rec[maxm][maxn];
long double  pitch[maxm], note[maxn];
int n, m;

int main() {
    scanf("%d", &m);
    for(int i = 1; i <= m; i++) scanf("%Lf", &pitch[i]);
    scanf("%d", &n);
    for(int i = 1; i <= n; i++) scanf("%Lf", &note[i]);

    memset(rec, 0, sizeof(rec));
    for(int i = 0; i <= n; i++) dp[0][i] = 0.0;
    long double M = numeric_limits<long double>::max();
    for(int i = 1; i <= m; i++) dp[i][0] = M;

    for(int i = 1; i <= m; i++) {
        int up = min(i, n);
        for(int j = 1; j <= up; j++) {
            if( i == j) dp[i][j] = abs(pitch[i]-note[j]) + dp[i-1][j-1];
            else {
                rec[i][j] = dp[i-1][j] < dp[i-1][j-1] ;
                dp[i][j] = abs(pitch[i]-note[j]) + ((rec[i][j])? dp[i-1][j]: dp[i-1][j-1]) ;
            }
        }
    }

    int ind = 1;
    for(int j = 2; j <= n; j++)
        if( dp[m][j] < dp[m][ind] ) ind = j;
    // dp[m][ind]  is the answer
    cout << dp[m][ind] << endl;
    vector<int> res;
    int x = m, y = ind;
    while(x >= 1) {
        for(; rec[x][y]==1 && x >= 1; x--){}
        res.pb( x );
        x--, y--;
    }
    for(int i = (int)res.size()-1; i >= 0; i--)
        printf("%d%c", res[i]-1, (i)?' ':'\n' );
    return 0;
}


