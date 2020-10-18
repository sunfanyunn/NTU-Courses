#include <bits/stdc++.h>
#include <boost/functional/hash.hpp>
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

template<typename T>
ostream& operator <<(ostream &s, const vector<T> &c) {
    s << "[ ";
    for (auto it : c) s << it << " ";
    s << "]";
    return s;
}

#define max_sz 41929539
struct data{
    int ind;
    unsigned int freq;
    size_t _hash;
};


map<string, int> prep;
string t[20] = { "about", "after", "at", "before",
                 "between", "by", "down", "for",
                 "from", "in", "near", "of",
                 "on", "since", "than", "to",
                 "under", "up", "with", "without" };
inline bool isprep(string str) { return prep.find( str ) != prep.end(); }

size_t myhash (char *str) {
    vector<string> v;
    char *prev = str, *ptr = str;
    while( (*ptr )<'0' || *ptr > '9') {
        if( !(*ptr)) {
            string s(prev);
            if( !isprep(s) ) v.pb( s );
            prev = ptr+1;
        }
        ptr++;
    }
    return boost::hash<vector<string>>()(v);
}
#define Bucket 10000019

FILE *in, *out;
const long long bufsz = 4*373395730;
char Buffer[bufsz];
int Len[4];

struct two{
    int start;
    unsigned int freq;
};
vector< two > lookup_table[Bucket];

two total[max_sz];
int start_of_total[Bucket];

int main(int argc, char *argv[]) {
    assert( argc == 2);
    out = fopen(argv[1], "w");

    for(int i=0;i<20;i++)  prep[ t[i] ] = i;
    char *buffer = Buffer;
    for(int i = 2; i <= 5; i++) {
        string ttmp =  "/tmp2/dsa2016_project/" + to_string(i) + "gm.small.txt";
        in = fopen(ttmp.c_str(), "r");
        LL len = Len[i-2] = fread(buffer, 1, bufsz, in);
        buffer[len] = 0;
        // use to add 0 to appropriate place
        char *ptr = buffer, *prev = buffer;
        while( *(prev) != '\0' ) {
            //process one line
            while( !( *ptr >= '0' && *ptr <= '9') ) {
                if( *ptr == ' ' || *ptr == '\t') *ptr = 0;
                ptr++;
            }
            *(ptr-1) = 0;
            while( *ptr >= '0' && *ptr <= '9' ) ptr++;
            *ptr = 0;
            prev = ++ptr;
        }

        buffer += len+1;
        fclose(in);
    }
    fwrite(Buffer, 1, bufsz, out);

    debug("end first phase\n");
    data tmp;

    LL cnt = 0;

    buffer = Buffer;
    for(int i = 2; i <= 5; i++) {
        char *ptr = buffer, *prev=buffer, *Line=buffer;
        while( *(prev) != '\0' ) {
            //process one line
            while( !( *ptr >= '0' && *ptr <= '9') )  ptr++;
            *(ptr-1) = 0;
            // [Line, ptr)
            unsigned int freq = 0;
            while( *ptr >= '0' && *ptr <= '9' ) freq = (freq<<1) + (freq<<3) + (*ptr)-48, ptr++;

            tmp.freq = freq;
            tmp.ind = (Line-Buffer);
            tmp._hash = myhash(Line)%Bucket;

            lookup_table[tmp._hash].pb( (two){tmp.ind, freq} );
            //assert( myhash(Line) == myhash(&Buffer[tmp.ind]));

            *ptr = 0;
            Line = prev = ++ptr;

        }
        buffer += Len[i-2]+1;
    }
    debug("enter building hash_table\n");

    cnt = 0;
    for(int i = 0; i < Bucket; i++) {
        start_of_total[i] = cnt;
        for(two pppp : lookup_table[i]) {
            total[cnt++] = pppp;
        }
    }
    fwrite(start_of_total, sizeof(int), Bucket, out);
    assert( cnt == max_sz);
    fwrite(total, sizeof(two), cnt, out);
    debug("finish preprocessing\n");
}
