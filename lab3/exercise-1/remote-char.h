// TODO_1 
// declaration the struct corresponding to the exchanged messages

typedef enum direction_t {UP, DOWN, LEFT, RIGHT} direction_t;

typedef struct remote_char_t {

   int msg_type;

   char ch;

   direction_t direction;
} remote_char_t;

// TODO_2
//declaration of the FIFO location

char SERVER_INPUT[19] = "/tmp/server_input";