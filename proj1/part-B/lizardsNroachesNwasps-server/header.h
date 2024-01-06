#define WINDOW_SIZE 15
#include <stdbool.h>

// declaration the struct corresponding to the exchanged messages

typedef struct lizards_struct {

   char ch;

   int code;

   int x;

   int y;

   int score;

   bool winner;

   int direction;

} lizards_struct;

typedef struct matrix_translation {

   int x;

   int y;

   int z;

} matrix_translation;