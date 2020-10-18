#include <cstdio>
#include <boost/functional/hash.hpp>
#include <algorithm>
#include <cassert>
#include <functional>
#include <vector>
#include <cstring>
#include <iostream>
#include <unordered_set>
#include <set>
#include <unordered_map>
#include <map>
using namespace std;
#ifdef DEBUG
    #define debug(...) printf(__VA_ARGS__)
#else
    #define debug(...) (void)0
#endif
#define mp make_pair
#define pb push_back
#define LL long long
#define pii pair<int,int>
#define PII pair<long long, long long>
#define fi first
#define se second
#define all(x) (x).begin(),(x).end()
#define SZ(x) ((int)(x).size())

#define max_sz 41929539
struct data{
    int ind;
    unsigned int freq;
    size_t _hash;
};

string t[20] = { "about", "after", "at", "before",
                 "between", "by", "down", "for",
                 "from", "in", "near", "of",
                 "on", "since", "than", "to",
                 "under", "up", "with", "without" };

typedef pair<char*, unsigned int> mypair;
unordered_map<string, int> prep;

inline bool isprep(string str) { return prep.find( str ) != prep.end(); }

int LevenshteinDistance(char *s, int len_s, char *t, int len_t) {
    /* base case: empty strings */
    if (len_s == 0) return len_t;
    if (len_t == 0) return len_s;
    /* test if last characters of the strings match */
    int cost;
    if (s[len_s-1] == t[len_t-1]) cost = 0;
    else  cost = 1;
    /* return minimum of delete char from s, delete char from t, and delete char from both */
    return min(LevenshteinDistance(s, len_s - 1, t, len_t    ) + 1,
               min( LevenshteinDistance(s, len_s    , t, len_t - 1) + 1,
                   LevenshteinDistance(s, len_s - 1, t, len_t - 1) + cost));
}

inline void proceed(char *& s) {
    while( *s ) s++;
    s++;
}
void print(char* s) {
    char *ptr = s;
    printf("%s", ptr);
    while(1) {
        proceed(ptr);
        if( *ptr >= '0' && *ptr <= '9' ) break;
        printf(" %s", ptr);
    }
    printf("\t%s\n", ptr);
}
bool cmp(  mypair a, mypair b) {
    if( a.se != b.se ) return a.se > b.se;
    int tmp;
    while( ((tmp=strcmp(a.fi, b.fi)) == 0)) {
        proceed(a.fi); proceed(b.fi);
        if( isdigit(*a.fi) || isdigit(*b.fi)) return isdigit(*a.fi);
    }

    return tmp<0;
}

const long long bufsz = 4*373395730;
char Buffer[bufsz];
#define Bucket 10000019
struct two{
    int start;
    unsigned int freq;
};
two total[max_sz];
int start_of_total[Bucket];

int main(int argc, char *argv[]) {
    FILE *in = fopen(argv[1], "rb");
    for(int i=0;i<20;i++)  prep[ t[i] ] = i;
    fread(Buffer, 1, bufsz, in);
    fread(start_of_total, 4, Bucket, in);
    fread(total, sizeof(two), max_sz, in);
    fclose(in);

    char line[105];
    while( fgets(line, 105, stdin) != NULL ) {
        printf("query: %s", line);
        int I= strlen(line)-1;
        if( I > 0 && (line[I] == '\n')) line[I] = '\0';

        vector< pair<char*, unsigned int> > ans;
        vector<string> tot;
        vector<string> v;

        //strtok
        char *pch;
        pch = strtok( line, " ");
        while( pch != NULL) {
            tot.pb( pch );
            if( !isprep( pch ) ) v.pb( pch );
            pch = strtok(NULL, " ");
        }

        size_t ind = boost::hash<vector<string>>()(v)%Bucket;
        int start = start_of_total[ind];
        int end ;
        if( ind+1 != Bucket )
            end = start_of_total[ind+1];
        else
            end = max_sz;

        if(  v.size() != tot.size() ) {
            //there is prep in current query
            for(int qq = start; qq < end; qq++) {
                char *ptr = &Buffer[ total[qq].start ];
                //start checking this candidate
                vector<string> ctot;
                int cur = 0;
                bool invalid = false;
                while( !( *ptr >= '0' && *ptr <= '9' ) ) {
                    if( !isprep(ptr) ) {
                        if( cur == SZ(v) || ptr != v[ cur ]) { invalid = true;  break; }
                        cur++;
                    }
                    ctot.pb( ptr );
                    proceed(ptr);
                }
                if(cur != SZ(v) || invalid) continue;

                bool flag = true;

                int x = 0, y = 0, prev_x = 0, prev_y = 0;
                for(int i = 0; i <= SZ(v); i++, x++, y++) {
                    if( i != SZ(v)) {
                        while( ctot[x] != v[i] ) x++;
                        while( tot[y] != v[i] ) y++;
                    }else { x = SZ(ctot), y = SZ(tot); }
                    // [prev_x .. x)  and [prev_y..y)
                    if(  prev_x != x && prev_y == y) { flag = false; break; }
                    char a[10], b[10];
                    for(int k = prev_x; k < x; k++) a[k-prev_x] = prep[ ctot[k] ];
                    for(int k = prev_y; k < y; k++) b[k-prev_y] = prep[ tot[k] ];
                    int dis = LevenshteinDistance(a, x-prev_x, b, y-prev_y);

                    if( dis > 1) flag = false;
                    if( !flag ) break;
                    prev_x = x+1, prev_y = y+1;
                }
                //flag == true stands for it's a valid candidate
                if( flag ) ans.pb(mp(&Buffer[ total[qq].start ], total[qq].freq));
            }
        } else {
            // there is no prep in current query
            for(int qq = start; qq != end; qq++) {
                char *ptr = &Buffer[ total[qq].start ];
                //start checking this candidate
                int cur = 0, token = 0;
                bool invalid = false;
                while( !( *ptr >= '0' && *ptr <= '9' ) ) {
                    if( !isprep( ptr )) {
                        if( cur == SZ(v) || ptr != v[cur]) { invalid = true; break; }
                        cur++;
                    }
                    token++;
                    proceed(ptr);
                }
                if(cur != SZ(v) || invalid) continue;
                if( token-SZ(tot) <= 2)
                    ans.pb( mp(&Buffer[total[qq].start], total[qq].freq) );
            }
        }
        sort( all(ans), cmp);
        auto it = unique( all(ans) );
        ans.resize( distance(ans.begin(), it) );
        int up = min( SZ(ans), 10);
        printf("output: %d\n", up);
        for(int i = 0; i < up; i++) print( ans[i].fi );
    }
}
