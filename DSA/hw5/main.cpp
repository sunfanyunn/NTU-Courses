#include <bits/stdc++.h>
#define fi first
#define se second
#define pb push_back
#ifdef DEBUG
    #define debug(...) printf(__VA_ARGS__)
#else
    #define debug(...) (void)0
#endif
using namespace std;

FILE *dic;
unordered_set<string> dict;
unordered_map<string, vector<string> > dict_ed;

void method_1(string word) {
    vector<string> ed2;
    vector<string> ed1;
    //insert
    word.insert(0, 1, ' ');
    for(int i = 0; i < word.size(); i++) {
        for(int j = 0; j < 26; j++) {
            word[i] = 'a'+j;
            ed1.push_back(word);
        }
        if( i != (int)word.size()-1 ) word[i] = word[i+1];
    }
    word.pop_back();
    //delete
    string del = word; del.pop_back();
    for(int i = word.size()-1; i >= 0; i--) {
        ed1.pb( del );
        if( i ) del[i-1] = word[i];
    }

    //substitute
    for(int i = 0; i < word.size(); i++) {
        char tmp = word[i];
        for(int j = 0; j < 26; j++) {
            word[i] = 'a'+j;
            ed1.push_back(word);
        }
        word[i] = tmp;
    }
    //transpose
    for(int i = 0; i <(int)word.size()-1; i++) {
        swap(word[i], word[i+1]);
        ed1.push_back(word);
        swap(word[i], word[i+1]);
    }
    for(string& str : ed1) {
        //insert
        str.insert(0, 1, ' ');
        for(int i = 0; i < str.size(); i++) {
            for(char ch = 'a'; ch <= 'z'; ch++) {
                str[i] = ch;
                if( dict.find(str) != dict.end() ) ed2.pb(str );
            }
            if( i != (int)str.size()-1 ) str[i] = str[i+1];
        }
        str.pop_back();
        //delete
        del = str;del.pop_back();
        for(int i = str.size()-1; i >= 0; i--) {
            if( dict.find(del) != dict.end() )
                ed2.push_back(del);
            if( i ) del[i-1] = str[i];
        }
        //substitute
        for(int i = 0; i < str.size(); i++) {
            char tmp = str[i];
            for(int j = 0; j < 26; j++) {
                str[i] = 'a'+j;
                if( dict.find(str) != dict.end() )
                    ed2.push_back(str);
            }
            str[i] = tmp;
        }
        //transpose
        for(int i = 0; i <(int)str.size()-1; i++) {
            swap(str[i], str[i+1]);
            if( dict.find(str) != dict.end() )
                ed2.push_back(str);
            swap(str[i], str[i+1]);
        }
    }
    for( string& str : ed1) if( dict.find(str) != dict.end() )  ed2.push_back(str);
    sort(ed2.begin(), ed2.end());
    vector<string>::iterator it = unique(ed2.begin(), ed2.end());
    ed2.resize(distance(ed2.begin(), it));
    for(string& s : ed2) printf(" %s", s.c_str());
    if(ed2.empty()) printf(" NONE");
    putchar('\n');
}
int dp[36][36];
bool edit_distance(string a, string b) {
    int n = a.size(), m = b.size();
    int tmp = n-m; if(tmp<0) tmp=-tmp;
    if( tmp > 2) return false;
    int charDic[26]; memset(charDic, 0, sizeof(charDic));
    for(int i = 0; i < n; i++)
        if( a[i] < 'a' || a[i] > 'z' ) return false;
    //start dp
    for(int i = 1; i <= n; i++) {
        int db = 0;
        for(int j = 1; j <= m; j++) {
            int i1 = charDic[ b[j-1]-'a' ];
            int j1 = db;
            int cost = 0;
            if( a[i-1] == b[j-1] ) db = j;
            else cost = 1;
            dp[i][j] = min(dp[i-1][j-1]+cost, min(dp[i-1][j]+1, dp[i][j-1]+1));
            if( i1 > 0 && j1 > 0 )
                dp[i][j] = min(dp[i][j], dp[i1-1][j1-1] + (i-i1-1) + (j-j1-1) + 1);
        }
        charDic[ a[i-1] - 'a' ] = i;
    }
    return dp[n][m]<=2;
}
void method_2(string word) {
    vector<string> ed2;
    vector<string> ed1;
    //insert
    string ins(word);
    ins.pb(' ');
    //word.size() == ins.size()-1
    for(int i = word.size(); i >= 0; i--) {
        for(int j = 0; j < 26; j++) {
            ins[i] = 'a'+j;
            ed1.push_back(ins);
        }
        if( i ) ins[i] = ins[i-1];
    }
    //delete
    string del = word;del.pop_back();
    for(int i = word.size()-1; i >= 0; i--) {
        ed1.push_back(del);
        if( i ) del[i-1] = word[i];
    }
    //substitute
    string sub = word;
    for(int i = 0; i < word.size(); i++) {
        char tmp = sub[i];
        for(int j = 0; j < 26; j++) {
            sub[i] = 'a'+j;
            ed1.push_back(sub);
        }
        sub[i] = tmp;
    }
    //transpose
    string trans = word;
    for(int i = 0; i <(int)word.size()-1; i++) {
        swap(trans[i], trans[i+1]);
        ed1.push_back(trans);
        swap(trans[i], trans[i+1]);
    }
    for(string& str : ed1) {
        unordered_map<string, vector<string> >::iterator it;
        //delete and substitute
        del = str;del.pop_back();
        for(int i = str.size()-1; i >= 0; i--) {
            if( dict.find(del) != dict.end() )
                ed2.push_back(del);
            //substitute
            it = dict_ed.find(del);
            if( it != dict_ed.end() ) {
                for(string& s : it->se) if( edit_distance(s, word) )
                    ed2.pb( s );
            }
            //end substitute
            if( i ) del[i-1] = str[i];
        }
        //insert
        it = dict_ed.find(str);
        if( it != dict_ed.end() ) {
            for(string& s : it->se)
                ed2.pb( s );
        }
        //transpose
        trans = str;
        for(int i = 0; i <(int)word.size()-1; i++) {
            swap(trans[i], trans[i+1]);
            if( dict.find(trans) != dict.end() )
                ed2.push_back(trans);
            swap(trans[i], trans[i+1]);
        }
    }

    for( string& str : ed1) if( dict.find(str) != dict.end() )  ed2.push_back(str);
    sort(ed2.begin(), ed2.end());
    vector<string>::iterator it = unique(ed2.begin(), ed2.end());
    ed2.resize(distance(ed2.begin(), it));
    for(string& s : ed2) printf(" %s", s.c_str());
    if(ed2.empty()) printf(" NONE");
    putchar('\n');
}
int main() {
    dic = fopen("/tmp2/dsa2016_hw5/cmudict-0.7b",  "r");
    char w[50];
    char line[1000];
    while( fgets(line, 1000, dic) ) {
        sscanf(line, "%s", w);
        string tmp(w);
        bool flag = false;;
        for(int i = 0; i < tmp.size(); i++)
            if( isalpha(tmp[i]) )  tmp[i] = tolower(tmp[i]);
            else { flag = true;}
        dict.insert(tmp);
        //dictionary only consists of lowercase alphabets
        if( !flag ) {
            string cur = tmp; cur.pop_back();
            for(int i = tmp.size()-1; i >= 0; i--) {
                dict_ed[cur].pb( tmp );
                if( i ) cur[i-1] = tmp[i];
            }
        }
    }
    fclose(dic);
    for(int i = 0; i < 36; i++) dp[i][0] = i;
    for(int i = 1; i < 36; i++) dp[0][i] = i;

    while(fgets(line, 1000, stdin)) {
        if( sscanf(line, "%s", w) != 1) break;
        string word(w);
        printf("%s ==>", w);
        if( dict.find(word) != dict.end() )  { printf(" OK\n"); continue; }
        // experient with random testcases
        int i=0;
        for( ; i < word.size() && word[i] >= 'a' && word[i] <= 'z' ; i++){}

        if( i != word.size() ) method_1( word ),debug("1");
        else method_2( word ),debug("2");
    }
    return 0;
}
