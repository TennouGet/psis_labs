// declaration the struct corresponding to the exchanged messages

typedef enum direction_t {UP, DOWN, LEFT, RIGHT} direction_t;

typedef struct remote_char_t {

   int msg_type;

   int code;

   char ch;

   int n_roaches;

   int r_scores[10];

   direction_t direction;

   direction_t r_direction[10];

   int r_bool[10];
} remote_char_t;


typedef struct remote_screen {

   int msg_type;

   char ch;

   int old_x, old_y, new_x, new_y;

   direction_t old_direction, new_direction;

   int screen_roaches[10][4];

} remote_screen;

typedef struct response_to_client {

   int status;
   
   int code;

   int assigned_char;

   int score;

} response_to_client;
