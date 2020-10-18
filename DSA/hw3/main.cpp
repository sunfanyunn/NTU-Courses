#include <cstdio>
#include <iostream>
#include <algorithm>
#include <vector>
#include <set>
#include <string>
#include <cstring>
#include <cassert>
#include <sstream>
#include <utility>
#include <functional>

#define pb push_back
#define all(x) x.begin(), x.end()
#define mp make_pair
#define fi first
#define se second
using namespace std;
typedef long long LL;
typedef pair<int,int> pii;
#define maxn 35

class B {
public:
    int left, right, length;
    bool determined;
    B(int len) {
        length = len;
        determined = false;
    };
    B() {determined = false;}
    bool test() {
        if( right - left == length - 1)
            determined = true;
        return determined;
    }
    void finished(int start, int end) {
        left = start, right = end, determined = true;
    }
};
class L {
public:
    std::vector<B> Block;
    int len;
    std::vector<int*> address;
    bool type;
    bool fill_black( int start, int end);
    bool fill_white( int start, int end);
    int cnt_cover(int ind, int Len=1);
    void print();
    void init_runrange( );
    bool intersect();
    bool find_empty();
    bool bblock();
    bool block();
    bool refine_runrange();
    bool fillin();
    bool logic();
    bool check(int N);
};
L r[maxn], c[maxn];
int m, n, table[maxn][maxn];

void L::print() {
    for(int i = 0;i<Block.size();i++) printf("%d %d %d\n" ,Block[i].length, Block[i].left, Block[i].right);
    for(int i=1;i<=len;i++) printf("%3d", i);
    putchar('\n');
    for(int i=1;i<=len;i++) printf("%3d", *address[i]);
    putchar('\n');
}
bool L::fill_black(int start, int end) {
    if( start < 1 || end > len ) return true;
    for(int i = start; i <= end; i++) {
        if( *address[i] == 0) return false;
        else if( *address[i] == -1) {
            (*address[i]) = 1;
            if( type ) { if( !c[i].logic() ) return false; }
            else  { if( !r[i].logic() ) return false; }
        }
    }
    return true;
}
bool L::fill_white(int start, int end) {
    if( start < 1 || end > len ) return true;
    for(int i = start; i <= end; i++) {
        if( *address[i] == 1) return false;
        else if( *address[i] == -1) {
            (*address[i]) = 0;
            if( type ) { if( !c[i].logic() ) return false; }
            else { if(!r[i].logic() ) return false; }
        }
    }
    return true;
}
int L::cnt_cover(int ind , int Len) {
    int cnt = 0;
    for(B& b : Block) {
        if( b.left <= ind && b.right >= ind+Len-1) cnt++;
    }
    return cnt;
}
void L::init_runrange() {
    int ptr = 1;
    for(int i = 0; i < Block.size(); i++) {
        Block[i].left = ptr;
        ptr += (Block[i].length + 1);
    }
    ptr = len ;
    for(int i = (int)Block.size() - 1; i >= 0; i--) {
        Block[i].right = ptr;
        ptr -= (Block[i].length + 1);
    }
}
bool L::intersect() {
    for( B& b : Block) {
        if (b.determined) continue;
        else {
            int u = b.right - b.left + 1 - b.length;
            if( !fill_black( b.left + u, b.right - u) ) return false;
        }
    }
    return true;
}
bool L::find_empty() {
    for(int K = 0; K <= Block.size(); K++) {
        if( K != Block.size() ) if( Block[K].determined ) continue;
        if( K == 0 ) {
            if( !fill_white(1, Block.front().left-1) ) return false;
        }
        else if( K == Block.size() ) {
            if( !fill_white( Block.back().right + 1,  len) ) return false;
        }
        else  {
           if( !fill_white( Block[K-1].right+1, Block[K].left-1)) return false;
        }
    }
    return true;
}
//check if possible
bool L::check(int N) {
    int con = 0, ptr = 0;
    for(int i = 1; i <= N; i++) {
        assert( *address[i] != -1);
        if( *address[i] == 1) con++;
        else if( con != 0) {
            if( con != Block[ptr].length ) return false;
            else if( !Block[ptr].determined ) Block[ptr].finished(i-con, i-1);
            ptr++, con = 0;
        }
    }
    if(len == N) {
        if( con != 0) {
            if( !Block.back().determined ) Block.back().finished(len-con+1, len);
            return con == Block.back().length && ptr == (int)Block.size()-1;
        }
        else
            return ptr == (int)Block.size();
    }
    if( con != 0 ) {
        int length = Block[ptr].length;
        if( length - con > len - N) return false;
        Block[ptr].finished(N-con+1, N-con+length);
        if( !fill_black(N+1, N+length-con) ) return false;
        int tmp = N+1+length-con;
        if( tmp <= len) {
            if( !fill_white( tmp, tmp) ) return false;
            if( ptr+1 < (int)Block.size() )
                Block[ptr+1].left = max(Block[ptr+1].left, tmp+1);
        }
    }
    return true;
}
bool L::refine_runrange() {
    int con = 0;
    for(int i = 1; i <= len; i++) {
        if( *address[i] == 1) con++;
        else if( con != 0 ) {
            int cnt = 0, rec;
            int start = i-con, end = i-1;
            for(int j = 0; j < Block.size(); j++) {
                if( Block[j].left <= start && Block[j].right >= end) cnt++, rec = j;
            }
            if( cnt == 1 && con > Block[rec].length ) return false;
            con = 0;
        }
    }
    for(int K = 0; K < (int)Block.size(); K++ ) {
        B& b = Block[K];
        if(b.determined) continue;
        int left = b.left, right = b.right;
        //first phase refine
        for(int i = min(len, b.left+b.length-1); i >= b.left; i--)
            if( *address[i] == 0 ) {
                b.left = i+1; break;
            }
        for(int i = max(1, b.right-b.length+1); i <= b.right; i++)
            if( *address[i] == 0) {
                b.right = i-1; break;
            }
        if( b.right - b.left + 1 < b.length) return false;
        else if ( b.test() ) {
            if( !fill_black(b.left, b.right) ) return false;
            if( !fill_white(b.left-1, b.left-1) || !fill_white(b.right+1, b.right+1) ) return false;
            continue;
        }
        //second phase refine
        int L, R;
        if( Block.size() == 1)           L = b.left, R = b.right;
        else if( K == 0)                 L = b.left,             R = min(b.right, Block[K+1].left - 1);
        else if(K==(int)Block.size()-1)  L = max(b.left, Block[K-1].right+1), R = b.right;
        else                             L = max(b.left, Block[K-1].right+1), R = min(b.right, Block[K+1].left - 1);

        while( (*address[L] ) != 1 && R > L) L++;
        while( (*address[R] ) != 1 && R > L) R--;
        // R can be equal to L
        if( R >= L ) {
            if( *address[L] == 1 && *address[R] == 1 ) {
                int u = b.length - (R - L + 1);
                if( !fill_black(L+1, R-1) || u < 0)   return false;
                int a = b.left, B = b.right;
                b.left  = max(b.left, L - u);
                b.right = min(b.right, R + u);
                if( b.right - b.left < b.length - 1) return false;
            }
        }
        if( b.test() ) if( !fill_black( b.left, b.right) ) return false;
    }
    return true;
}
bool L::block() {
    int con = 0;
    vector<int> arr;
    for(int i = 1; i <= len; i++) {
        if( (*address[i]) != 0 ) con++;
        else {
            if( con != 0) arr.push_back( con );
            con = 0;
        }
    }
    if( con != 0) arr.push_back( con );
    bool flag = true;
    if( arr.size() == Block.size() ) {
        for(int i = 0; i < arr.size(); i++)
            if( arr[i] != Block[i].length ) flag = false;
    }else flag = false;
    if( flag ) {
        for(int i = 1; i <= len; i++)
            if( *address[i] != 0) (*address[i]) = 1;
        return check(len) ;
    }
    arr.clear(); con = 0;
    for(int i = 1; i <= len; i++) {
        if( *address[i] == 1 ) con++;
        else {
            if( con != 0) arr.push_back( con );
            con = 0;
        }
    }
    if( con != 0) arr.push_back( con );
    flag = true;
    if( arr.size() == Block.size() ) {
        for(int i = 0; i < arr.size(); i++)
            if( arr[i] != Block[i].length ) flag = false;
    }else flag = false;
    if( flag ) {
        for(int i = 1; i <= len; i++)
            if( *address[i] != 1) (*address[i]) = 0;
        return check(len) ;
    }
    return true;
}
bool L::fillin() {
    for(int i = 0; i < Block.size(); i++) {
        B& b = Block[i];
        if( b.determined ) continue;
        if( *address[b.left] == 1 && cnt_cover(b.left) == 1) {
            b.finished(b.left, b.left + b.length-1);
            if( !fill_black(b.left,  b.right) ) return false;
            if( !fill_white(b.left-1, b.left-1) || !fill_white(b.right+1, b.right+1) ) return false;
        }
        else if( *address[b.right] == 1 && cnt_cover(b.right) == 1) {
            b.finished(b.right-b.length+1, b.right);
            if( !fill_black(b.left, b.right) ) return false;
            if( !fill_white(b.left-1, b.left-1) || !fill_white(b.right+1, b.right+1) ) return false;
        }
    }
    return true;
}
bool L::logic( ) {
    if( !refine_runrange() ) return false;
    if( !fillin() ) return false;
    if( !intersect() ) return false;
    if( !find_empty()) return false;
    assert( block() );
    return true;
}
void tcopy( int A[][maxn], int B[][maxn]) {
    for(int i = 1; i <= n; i++)
        for(int j = 1; j <= m; j++)
            A[i][j] = B[i][j];
}
void lcopy( L A[], L B[], int N) {
    for(int i=1;i<=N;i++) {
        A[i].Block = B[i].Block;
    }
}
void init() {
#ifndef DEBUG
    int next;
    for(int i = 1; i <= n; i++) {
        string line; getline(cin, line);
        istringstream iss(line);
        while(iss >> next)   r[i].Block.pb( B(next) );
        r[i].len = m; r[i].type = true;
        for(int j = 0; j <= m+1; j++) r[i].address.pb( &table[i][j] );
        r[i].init_runrange();
    }
    for(int i = 1; i <= m; i++) {
        string line; getline(cin, line);
        istringstream iss(line);
        while(iss >> next)   c[i].Block.pb( B(next) );
        c[i].len = n; c[i].type = false;
        for(int j = 0; j <= n+1; j++) c[i].address.pb( &table[j][i] );
        c[i].init_runrange();
    }
    memset(table, -1, sizeof(table));
#else
    int next;
    for(int i = 1; i <= m; i++) {
        string line; getline(cin, line);
        istringstream iss(line);
        while(iss >> next)   c[i].Block.pb( B(next) );
        c[i].len = n; c[i].type = false;
        for(int j = 0; j <= n+1; j++) c[i].address.pb( &table[j][i] );
        c[i].init_runrange();
    }
    for(int i = 1; i <= n; i++) {
        string line; getline(cin, line);
        istringstream iss(line);
        while(iss >> next)   r[i].Block.pb( B(next) );
        r[i].len = m; r[i].type = true;
        for(int j = 0; j <= m+1; j++) r[i].address.pb( &table[i][j] );
        r[i].init_runrange();
    }
    memset(table, -1, sizeof(table));
#endif
}
void output() {
    for(int i = 1; i <= n; i++) {
        for(int j = 1; j <= m; j++) {
            printf("%c", (table[i][j]) ? '#' : '.');
        }
        putchar('\n');
    }
}
bool Check(int C, int R) {
    if( !r[R].check(C) ) return false;
    if( !c[C].check(R) ) return false;
    return true;
}
bool dfs( int row=1 , int column=1) {
    if( column == m+1 ) row++, column = 1;
    if( row == n+1) return true;

    while( table[row][column] != -1) {
        if( !Check(column, row) ) return false;
        if( !r[row].logic() || !c[column].logic() ) return false;
        column++;
        if( column == m+1 ) row++, column = 1;
        if(row == n+1) return true;
    }
    // temp copy of everything
    int ttable[maxn][maxn];
    L tr[maxn], tc[maxn];
    tcopy(ttable, table); lcopy(tr, r, n); lcopy(tc, c, m);
    table[row][column] = 1;
    if( Check(column, row)  ) {
        if( r[row].logic() && c[column].logic() ) {
            if( dfs(row, column+1)) return true;
        }
    }
    tcopy(table, ttable); lcopy(r, tr, n); lcopy(c, tc, m);
    table[row][column] = 0;
    if( Check(column, row)  ) {
        if( r[row].logic() && c[column].logic() ) {
            if( dfs(row, column+1)) return true;
        }
    }
    return false;
}
int main() {
    cin >> n >> m;
    cin.ignore(256, '\n');
    //n rows, m columns
    init();
    //logic
    for(int qq = 0; qq < 10; qq++) {
        for(int i=1;i<=n;i++) r[i].logic();
        for(int i=1;i<=m;i++) c[i].logic();
    }
#ifdef DEBUG
#endif
    dfs();
    output();
    return 0;
}
