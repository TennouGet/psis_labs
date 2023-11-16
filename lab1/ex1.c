#include "string.h"
#include <stdio.h>

int main(int argc, char **argv){

    char strn [100];

    for(int i=1; i < argc; i++){
        strncat(strn, argv[i], sizeof(argv[i]));
    }

    printf("%s\n", strn);

}