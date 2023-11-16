#include <stdio.h>

int main(int argc, char **argv){

    char strnout [100];

    int j=0, k=0;

    char* c;

    for(int i=1; i < argc; i++){

        while(argv[i][j] != '\0'){
            strnout[k] = argv[i][j];
            j++;
            k++;
        }
        j=0;
    }

    printf("%s\n", strnout);

}