
#include <ncurses.h>
#include "header.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#define WINDOW_SIZE 15

#include <zmq.h>
#include <assert.h>
#include <string.h>
#include <time.h>


typedef struct char_n_pos {

   char ch;

   int code;

   int x;

   int y;

   int score;

} char_n_pos;

int collision_calculate_points(int score_a, int score_b){
    return (score_a + score_b)/2;
}

direction_t random_direction(){
    return  random()%4;

}

void new_position(int* x, int *y, direction_t direction){
    switch (direction)
    {
    case UP:
        (*x) --;
        if(*x ==0)
            *x = 2;
        break;
    case DOWN:
        (*x) ++;
        if(*x ==WINDOW_SIZE-1)
            *x = WINDOW_SIZE-3;
        break;
    case LEFT:
        (*y) --;
        if(*y ==0)
            *y = 2;
        break;
    case RIGHT:
        (*y) ++;
        if(*y ==WINDOW_SIZE-1)
            *y = WINDOW_SIZE-3;
        break;
    default:
        break;
    }
}

void calc_pos(char_n_pos characters[25], int i, int *screen_matrix, direction_t direction){

    int n_of_client = characters[i].ch - 65;

    bool collision = false;

    printf("calc_pos");

    int x = characters[i].x;
    int y = characters[i].y;

    //int aux = *(screen_matrix + x*WINDOW_SIZE + y*WINDOW_SIZE + n_of_client);

    switch (direction)
    {
    case UP:
        (x) --;
        if(x ==0)
            x = 2;
        for(int j=0; j < 25; j++){
            if(screen_matrix[x*WINDOW_SIZE*26 + y*26 + j]){
                collision = true;
                characters[i].score = characters[j].score = collision_calculate_points(characters[i].score, characters[j].score);
                break;
            }
        }
        if(!collision){
            screen_matrix[x*WINDOW_SIZE*26 + y*26 + n_of_client] = 1;
            screen_matrix[characters[i].x*WINDOW_SIZE*26 + characters[i].y*26 + n_of_client] = 0;
            characters[i].x = x;
            characters[i].y = y;
        }
        break;
    case DOWN:
        (x) ++;
        if(x ==WINDOW_SIZE-1)
            x = WINDOW_SIZE-3;
        for(int j=0; j < 25; j++){
            if(screen_matrix[x*WINDOW_SIZE + y*WINDOW_SIZE + j]){
                collision = true;
                characters[i].score = characters[j].score = collision_calculate_points(characters[i].score, characters[i].score);
                break;
            }
        }
        if(!collision){
            screen_matrix[x*y*n_of_client] = 1;
            screen_matrix[characters[i].x*characters[i].y*n_of_client] = 0;
            characters[i].x = x;
            characters[i].y = y;
        }
        break;
    case LEFT:
        (y) --;
        if(y ==0)
            y = 2;
        for(int j=0; j < 25; j++){
            if(screen_matrix[x*WINDOW_SIZE + y*WINDOW_SIZE + j]){
                collision = true;
                characters[i].score = characters[j].score = collision_calculate_points(characters[i].score, characters[i].score);
                break;
            }
        }
        if(!collision){
            screen_matrix[x*y*n_of_client] = 1;
            screen_matrix[characters[i].x*characters[i].y*n_of_client] = 0;
            characters[i].x = x;
            characters[i].y = y;
        }
        break;
    case RIGHT:
        (y) ++;
        if(y ==WINDOW_SIZE-1)
            y = WINDOW_SIZE-3;
        for(int j=0; j < 25; j++){
            if(screen_matrix[x*WINDOW_SIZE + y*WINDOW_SIZE + j]){
                collision = true;
                characters[i].score = characters[j].score = collision_calculate_points(characters[i].score, characters[i].score);
                break;
            }
        }
        if(!collision){
            screen_matrix[x*y*n_of_client] = 1;
            screen_matrix[characters[i].x*characters[i].y*n_of_client] = 0;
            characters[i].x = x;
            characters[i].y = y;
        }
        break;
    default:
        break;
    }
}

int main()
{	
    struct char_n_pos characters[25];
    int characters_n = 0;
    int i=0;

    //int screen_matrix[WINDOW_SIZE*WINDOW_SIZE*25];
    int* screen_matrix = (int *) malloc(WINDOW_SIZE*WINDOW_SIZE*26*sizeof(int));

    for(i=0; i < WINDOW_SIZE*WINDOW_SIZE*26; i++){
        screen_matrix[i] = 0;
    }

    FILE *fptr;
    FILE *fptr2;
    fptr = fopen("output.txt","w");
    fptr2 = fopen("output2.txt","w");

    int read_fd = 0;

    // Socket to talk to clients
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, "tcp://*:5555");
    assert (rc == 0);

    // Socket to publish to displays
    void *pub_context = zmq_ctx_new ();
    void *publisher = zmq_socket (pub_context, ZMQ_PUB);
    int rc2 = zmq_bind (publisher, "tcp://*:5557");
    assert(rc2 == 0);

    struct remote_char_t client;
    struct remote_screen screen;
    struct response_to_client client_response;

    char buffer[20] = "screen";

    screen.ch = 'a';
    screen.msg_type=0;
    screen.pos_x=0;

    zmq_send (publisher, buffer, strlen(buffer), ZMQ_SNDMORE);
    zmq_send (publisher, &screen, sizeof(screen), 0);


    // ncurses initialization
	initscr();		    	
	cbreak();				
    keypad(stdscr, TRUE);   
	noecho();	    


    /* creates a window and draws a border */
    WINDOW * my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);
    box(my_win, 0 , 0);	
	wrefresh(my_win);

    /* information about the character */
    int ch;
    int pos_x = WINDOW_SIZE/2;
    int pos_y = WINDOW_SIZE/2;

    int n_of_char_to_give = 65;
    char char_to_give = 0;


    direction_t  direction;
    i = 0;

    while (1)
    {

        // receive message from client trough socket
        zmq_recv (responder, &client, sizeof(remote_char_t), 0);
        //zmq_send (responder, "World", 5, 0);

        // process connection messages

        if(client.msg_type == 0){

            //srand((int)time); // uncomment THIS!

            char_to_give = n_of_char_to_give++;
            characters[characters_n].ch = char_to_give;
            characters[characters_n].code = rand();
            characters[characters_n].x = WINDOW_SIZE/2;
            characters[characters_n].y = WINDOW_SIZE/2;
            client_response.code = characters[characters_n].code;

            screen_matrix[characters[characters_n].x*WINDOW_SIZE*26 + characters[characters_n].y*26 + characters[i].ch - 65] = 1; //window-size - chars
            
            for(i=0; i < WINDOW_SIZE*WINDOW_SIZE*26; i++){
                fprintf(fptr,"%d\n",screen_matrix[i]);
            }
            i=0;

            characters_n++;

            client_response.assigned_char = char_to_give;
            client_response.status = 1;

            zmq_send (responder, &client_response, sizeof(client_response), 0);
            //zmq_recv (responder, &client, sizeof(remote_char_t), 0);
        }
        
        // process the movement message
        
        if(client.msg_type == 1){

            for(i=0; i < characters_n; i++){
                if(characters[i].ch == client.ch && characters[i].code == client.code){
                    break;
                }
            }

            wmove(my_win, characters[i].x, characters[i].y);
            waddch(my_win,' ');

            //new_position(&characters[i].x, &characters[i].y, client.direction);

            calc_pos(characters, i, screen_matrix, client.direction);

            wmove(my_win, characters[i].x, characters[i].y);
            waddch(my_win, characters[i].ch);
            //characters[i].ch outputs gibberish..
            //waddch(my_win, 'A'); 

            wrefresh(my_win);

            client_response.code = characters[i].code;
            client_response.assigned_char = characters[i].ch;
            client_response.status = 1;

            zmq_send (responder, &client_response, sizeof(client_response), 0);
            //zmq_recv (responder, &client, sizeof(remote_char_t), 0);
        }

        screen.ch = characters[i].ch;
        screen.pos_x = characters[i].x;
        screen.pos_y = characters[i].y;
        screen.msg_type = 1;
        zmq_send(publisher, buffer, strlen(buffer), ZMQ_SNDMORE);
        zmq_send(publisher, &screen, sizeof(screen), 0);

    }
  	endwin();			// End curses mode
    fclose(fptr);

	return 0;
}