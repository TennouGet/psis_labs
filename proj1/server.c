
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

   direction_t direction;

} char_n_pos;

int collision_calculate_points(int score_a, int score_b){
    return (score_a + score_b)/2;
}

direction_t random_direction(){
    return  random()%4;

}

void calc_pos(char_n_pos characters[25], int i, int *lizard_matrix, direction_t direction){

    int n_of_client = characters[i].ch - 65;

    bool collision = false;

    int x = characters[i].x;
    int y = characters[i].y;
    int pos = x*WINDOW_SIZE + y;

    switch (direction)
    {
    case UP:
        (x) --;
        if(x ==0)
            x = 2;
        if(lizard_matrix[x*WINDOW_SIZE + y] >= 0){
            collision = true;
            characters[i].score = characters[lizard_matrix[pos]].score = collision_calculate_points(characters[i].score, characters[lizard_matrix[pos]].score);
            break;
        }

        if(!collision){
            lizard_matrix[x*WINDOW_SIZE + y] = n_of_client;
            lizard_matrix[characters[i].x*WINDOW_SIZE + characters[i].y] = -1;
            characters[i].x = x;
            characters[i].y = y;
            characters[i].direction = direction;
        }
        break;
    case DOWN:
        (x) ++;
        if(x ==WINDOW_SIZE-1)
            x = WINDOW_SIZE-3;
        if(lizard_matrix[x*WINDOW_SIZE + y] >= 0){
            collision = true;
            characters[i].score = characters[lizard_matrix[pos]].score = collision_calculate_points(characters[i].score, characters[lizard_matrix[pos]].score);
            break;
        }
        if(!collision){
            lizard_matrix[x*WINDOW_SIZE + y] = n_of_client;
            lizard_matrix[characters[i].x*WINDOW_SIZE + characters[i].y] = -1;
            characters[i].x = x;
            characters[i].y = y;
            characters[i].direction = direction;
        }
        break;
    case LEFT:
        (y) --;
        if(y ==0)
            y = 2;
        if(lizard_matrix[x*WINDOW_SIZE + y] >= 0){
            collision = true;
            characters[i].score = characters[lizard_matrix[pos]].score = collision_calculate_points(characters[i].score, characters[lizard_matrix[pos]].score);
            break;
        }
        if(!collision){
            lizard_matrix[x*WINDOW_SIZE + y] = n_of_client;
            lizard_matrix[characters[i].x*WINDOW_SIZE + characters[i].y] = -1;
            characters[i].x = x;
            characters[i].y = y;
            characters[i].direction = direction;
        }
        break;
    case RIGHT:
        (y) ++;
        if(y ==WINDOW_SIZE-1)
            y = WINDOW_SIZE-3;
        if(lizard_matrix[x*WINDOW_SIZE + y] >= 0){
            collision = true;
            characters[i].score = characters[lizard_matrix[pos]].score = collision_calculate_points(characters[i].score, characters[lizard_matrix[pos]].score);
            break;
        }
        if(!collision){
            lizard_matrix[x*WINDOW_SIZE + y] = n_of_client;
            lizard_matrix[characters[i].x*WINDOW_SIZE + characters[i].y] = -1;
            characters[i].x = x;
            characters[i].y = y;
            characters[i].direction = direction;
        }
        break;
    default:
        break;
    }
}

void update_window(WINDOW * my_win, char_n_pos lizard, int mode){

    int i = 0;

    if(mode==0){ //delete lizard at previous position

        //wmove(my_win, characters[i].x, characters[i].y);
        //waddch(my_win,' ');
        mvwaddch(my_win, lizard.x, lizard.y, ' ');

        switch (lizard.direction)
        {
        case UP:
            for(i=0; i < 5; i++){
                mvwaddch(my_win, lizard.x+i, lizard.y, ' ');
            }
            break;
        case DOWN:
            for(i=0; i < 5; i++){
                mvwaddch(my_win, lizard.x-i, lizard.y, ' ');
            }
            break;
        case LEFT:
            for(i=0; i < 5; i++){
                mvwaddch(my_win, lizard.x, lizard.y+i, ' ');
            }
            break;
        case RIGHT:
            for(i=0; i < 5; i++){
                mvwaddch(my_win, lizard.x, lizard.y-i, ' ');
            }
            break;
        default:
            mvprintw(WINDOW_SIZE+3, 0, "\rERROR: no lizard direction.");
            break;
        }
    }

    if(mode==1){ //add lizard at new position
        
        switch (lizard.direction)
        {
        case UP:
            for(i=0; i < 5; i++){
                mvwaddch(my_win, lizard.x+i, lizard.y, '.');
            }
            break;
        case DOWN:
            for(i=0; i < 5; i++){
                //mvwaddch(my_win, lizard.x, lizard.y-i, '.');
                wmove(my_win, lizard.x-i, lizard.y);
                waddch(my_win, '.');
            }
            break;
        case LEFT:
            for(i=0; i < 5; i++){
                mvwaddch(my_win, lizard.x, lizard.y+i, '.');
            }
            break;
        case RIGHT:
            for(i=0; i < 5; i++){
                mvwaddch(my_win, lizard.x, lizard.y-i, '.');
            }
            break;
        default:
            mvprintw(WINDOW_SIZE+3, 0, "\rERROR: no lizard direction.");
            break;
        }

        mvwaddch(my_win, lizard.x, lizard.y, lizard.ch);
    }





}

int main()
{	
    struct char_n_pos characters[25];
    int i=0;

    int* lizard_matrix = (int *) malloc(WINDOW_SIZE*WINDOW_SIZE*sizeof(int));

    for(i=0; i < WINDOW_SIZE*WINDOW_SIZE; i++){
        lizard_matrix[i] = -1;
        if(i<25)
            characters[i].code = 0;
    }

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


    // creates a window and draws a border
    WINDOW * my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);
    box(my_win, 0 , 0);	
	wrefresh(my_win);

    // information about the character
    int n_of_char_to_give = 65;
    char char_to_give = 0;
    int x_rand=0;
    int y_rand=0;
    int j=0;
    bool found_empty_spot = false;

    //direction_t  direction;

    i = 0;

    while (1)
    {

        // receive message from client trough socket
        zmq_recv (responder, &client, sizeof(remote_char_t), 0);

        // process connection messages

        if(client.msg_type == 0){

            //srand((int)time); // uncomment THIS!
            for(i=0; i < 25; i++){
                if(characters[i].code == 0){
                    char_to_give = n_of_char_to_give + i;
                    break;
                }
            }
            

            //char_to_give = n_of_char_to_give++;
            characters[i].ch = char_to_give;
            characters[i].code = rand();

            while(j!=1000){
                x_rand = rand() % (WINDOW_SIZE-1);
                y_rand = rand() % (WINDOW_SIZE-1);
                if(lizard_matrix[x_rand*WINDOW_SIZE + y_rand] < 0){ // ADICIONAR VERIFICAÇAO PARA ROACHES!
                    found_empty_spot = true;
                    break;
                }
                j++;
            }

            if(found_empty_spot){

                characters[i].x = x_rand;
                characters[i].y = y_rand;
                client_response.code = characters[i].code;

                lizard_matrix[characters[i].x*WINDOW_SIZE + characters[i].y] = characters[i].ch - 65; //number of lizard occupying space

                client_response.assigned_char = char_to_give;
                client_response.status = 1;
                client_response.score = 0;
            }
            else{
                client_response.status = -2; // error 2: no empty space
            }

            zmq_send (responder, &client_response, sizeof(client_response), 0);
        }
        
        // process the movement message
        
        if(client.msg_type == 1){

            for(i=0; i < 25; i++){
                if(characters[i].ch == client.ch && characters[i].code == client.code){
                    break;
                }
            }

            //wmove(my_win, characters[i].x, characters[i].y);
            //waddch(my_win,' ');

            update_window(my_win, characters[i], 0);

            calc_pos(characters, i, lizard_matrix, client.direction);

            //characters[i].direction = client.direction;

            update_window(my_win, characters[i], 1);

            //wmove(my_win, characters[i].x, characters[i].y);
            //waddch(my_win, characters[i].ch);

            wrefresh(my_win);

            client_response.code = characters[i].code;
            client_response.assigned_char = characters[i].ch;
            client_response.status = 1;
            client_response.score = characters[i].score;

            zmq_send (responder, &client_response, sizeof(client_response), 0);
        }

        if(client.msg_type == 2){

            for(i=0; i < 25; i++){
                if(characters[i].ch == client.ch && characters[i].code == client.code){
                    break;
                }
            }

            lizard_matrix[characters[i].x*WINDOW_SIZE + characters[i].y] = -1; //number of lizard occupying space
            wmove(my_win, characters[i].x, characters[i].y);
            waddch(my_win,' ');
            wrefresh(my_win);

            client_response.code = characters[i].code;
            client_response.assigned_char = characters[i].ch;
            client_response.status = 2;
            zmq_send (responder, &client_response, sizeof(client_response), 0);

            client_response.code = 0;
            client_response.assigned_char = 0;
            client_response.status = 0;

            characters[i].ch = 0;
            characters[i].score = 0;
            characters[i].code = 0;
            characters[i].x = 0;
            characters[i].y = 0;
        }

        screen.ch = characters[i].ch;
        screen.pos_x = characters[i].x;
        screen.pos_y = characters[i].y;
        screen.msg_type = 1;
        zmq_send(publisher, buffer, strlen(buffer), ZMQ_SNDMORE);
        zmq_send(publisher, &screen, sizeof(screen), 0);

    }
  	endwin();			// End curses mode

    free(lizard_matrix);

	return 0;
}