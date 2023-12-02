// declaration the struct corresponding to the exchanged messages

typedef enum direction_t {UP, DOWN, LEFT, RIGHT} direction_t;

typedef struct remote_char_t {

   int msg_type;

   int code;

   char ch;

   direction_t direction;
} remote_char_t;

typedef struct remote_screen {

   int msg_type;

   char ch;

   int pos_x, pos_y;

} remote_screen;

typedef struct response_to_client {

   int status;
   
   int code;

   int assigned_char;

} response_to_client;