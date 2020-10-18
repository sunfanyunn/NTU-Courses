#include <cstring>
#include <string>
#include <sys/stat.h>
#include <cstdio>
#include <algorithm>
#include <stdlib.h>
#include <stdint.h>
using namespace std;

char op[150];
char cli[101], ser[101];
int n[1005], o[1005];
int dp[1005][1005];
int N, O;

int exist(char *name) {
  struct stat buffer;
  return (stat (name, &buffer) == 0);
}
/*
uint64_t hash(const char *s) {
  uint64_t ret = 0;
  while (*s)
    ret = ret * 131 + *s++;
  return ret;
}
*/
void calc() {
    N = O = 0;
    // find lcs
    FILE *nn = fopen(cli, "r");
    FILE *oo = fopen(ser, "r");
    char c;
    uint64_t ret = 0;
    while( (c=fgetc(nn)) != EOF ) {
        if( c == '\n' ) n[ N++ ] = ret, ret = 0;
        else
            ret = ret * 131 + c;

    }
    fclose(nn);
    ret = 0;
    while( (c=fgetc(oo)) != EOF ) {
        if( c == '\n' ) o[ O++ ] = ret, ret = 0;
        else
            ret = ret * 131 + c;
    }
    fclose(oo);
    for(int i = 0; i <= O; i++) dp[0][i] = 0;
    for(int i = 0; i <= N; i++) dp[i][0] = 0;
    for(int i = 1; i <= N; i++)
        for(int j = 1; j <= O; j++)
            dp[i][j] = ( n[i-1] == o[j-1] ) ? dp[i-1][j-1]+1 : max( dp[i-1][j], dp[i][j-1] ) ;
    printf("%d %d\n", N-dp[N][O], O-dp[N][O]);
    fflush(stdout);
}
int countLine(char *s ) {
    FILE *f = fopen(s, "r");
    char c;int cnt = 0;
    while( (c=fgetc(f)) != EOF ) {
        cnt += (c=='\n');
    }
    fclose(f);
    return cnt;
}
int main() {
    cli[0] = 'c', cli[1] = 'l', cli[2] = 'i', cli[3] = 'e', cli[4] = 'n', cli[5] = 't', cli[6] = '/';
    ser[0] = 's', ser[1] = 'e', ser[2] = 'r', ser[3] = 'v', ser[4] = 'e', ser[5] = 'r', ser[6] = '/';
    while( scanf("%s", op) != EOF && op[0] != 'e' ) {
        /*
        if( op[0] == '$' ) {
            fgets(op, 150, stdin);
            system(op);
            continue;
        }
        */
        //update
        scanf("%s", cli+7); strcpy(ser+7, cli+7);
        if( exist(cli) && exist(ser) ) {
            //calc
            calc();
            string s = "yes | cp " + string(cli) + " " + string(ser);
            //cp cli/P ser/P
            system( s.c_str() );
        }
        else if( exist(cli) ) {
            N = countLine(cli);
            //count line
            printf("%d 0\n", N);
            fflush(stdout);
            string s = "yes | cp " + string(cli) + " " + string(ser);
            //cp cli/P ser/P
            system( s.c_str() );
        }else {
            O = countLine(ser);
            printf("0 %d\n", O);
            fflush(stdout);
            strcpy(op, "rm -f "); strcpy(op + 6, ser);
            //rm -f server/filename
            system(op);
        }
    }
    return 0;
}

