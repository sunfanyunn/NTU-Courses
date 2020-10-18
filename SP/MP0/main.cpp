#include <cstdio>
#include <iostream>

int main(int argc, char *argv[] ) {
    FILE *file;
    file = fopen( argv[1], "r");
    char c;
    int ans = 0;
    while( (c=fgetc(file)) != EOF ) {
        ans += (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' ||
                c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U' );
    }
    fclose(file);
    file = fopen(argv[2], "w");
    fprintf(file, "%d\n", ans);
}

