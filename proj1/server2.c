
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
#include <stdio.h>
#include <math.h>


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


    //added this
    int id_roach = 0;
    int id_roach_client = 0;
    int max_roaches = WINDOW_SIZE*WINDOW_SIZE/3;
    int r_space = max_roaches;
    //int* universal_rcode = (int *) malloc(max_roaches*10*sizeof(int));


    // client code | client barata code (0-9) | server barata code (0-(maxroaches-1))
    int code_to_barataid[max_roaches][3];
    // INDEXED BY: server barata code (0-(maxroaches-1)) : pos x | pos y | barata value
    int barataid_to_pos[max_roaches][3];
    // pos x || pos y || server barata code (0-(maxroaches-1))
    int position_to_barata[WINDOW_SIZE][WINDOW_SIZE][max_roaches];

    int n = 0;
    int x = 0;
    int y = 0;
    while(x!=WINDOW_SIZE){
        while (y!=WINDOW_SIZE){
            while (n!=max_roaches){
                position_to_barata[x][y][n] = -1;
                n++;
            }
            n=0;
            y++;
        }
        y=0;
        x++;
    }


    // ----------------------------

    int* lizard_matrix = (int *) malloc(WINDOW_SIZE*WINDOW_SIZE*sizeof(int));

    for(i=0; i < WINDOW_SIZE*WINDOW_SIZE; i++){
        lizard_matrix[i] = -1;
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
    
    // added this
    struct response_to_client roach_response;
    // --------------------------

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


    //direction_t  direction;

    i = 0;

    while (1)
    {

        // receive message from client trough socket
        zmq_recv (responder, &client, sizeof(remote_char_t), 0);

        // process connection messages

        if(client.msg_type == 0){

            //srand((int)time); // uncomment THIS!

            char_to_give = n_of_char_to_give++;
            characters[characters_n].ch = char_to_give;
            characters[characters_n].code = rand();
            characters[characters_n].x = WINDOW_SIZE/2;
            characters[characters_n].y = WINDOW_SIZE/2;
            client_response.code = characters[characters_n].code;

            lizard_matrix[characters[characters_n].x*WINDOW_SIZE + characters[characters_n].y] = characters[i].ch - 65; //number of lizard occupying space

            characters_n++;

            client_response.assigned_char = char_to_give;
            client_response.status = 1;

            zmq_send (responder, &client_response, sizeof(client_response), 0);
            //zmq_recv (responder, &client, sizeof(remote_char_t), 0);
        }
        

        if(client.msg_type == 3){


            int n_roaches = client.n_roaches;

            if(r_space-n_roaches>=0){
                //accept connection
                r_space = r_space-n_roaches;
            }
            else{
                //reject

            }
           
            
            int code = rand();

            int i = 0;
            while (i!=n_roaches){
                code_to_barataid[id_roach_client+i][0] = code;
                code_to_barataid[id_roach_client+i][1] = i;
                code_to_barataid[id_roach_client+i][2] = id_roach;
                
                //roach x pos
                int x = rand()%WINDOW_SIZE;
                barataid_to_pos[id_roach][0] = x;
                //roach y pos
                int y = rand()%WINDOW_SIZE;
                barataid_to_pos[id_roach][1] = y;
                //roach value
                barataid_to_pos[id_roach][2] = client.r_scores[i];


                //storing the roach in a 3 dimensional matrix that represents the map and the high represents the amount of roaches on the spot
                int n = 0;
                while(position_to_barata[x][y][n]!=-1){
                    n++;
                }
                position_to_barata[x][y][n] = id_roach;


                id_roach++;
                i++;
            }

            id_roach_client = id_roach_client + 1;

            roach_response.status = 1;
            roach_response.code = code;

            zmq_send (responder, &roach_response, sizeof(roach_response), 0);
            //zmq_recv (responder, &client, sizeof(remote_char_t), 0);
        }

        if(client.msg_type == 4){

            int i = 0;
            while(code_to_barataid[i][0]!= client.code){
                i++;
            }


            int b = 0;
            while (b!=10){
                if (client.r_bool[b]==1){
                    
                    while (code_to_barataid[i][1]!= b){
                        i++;
                    }

                    int roach_code = code_to_barataid[i][2];

                    int x = barataid_to_pos[roach_code][0];
                    int y = barataid_to_pos[roach_code][1];
                    int v = barataid_to_pos[roach_code][2];
                        
                    //delete old information about barata position on the 3d matrix
                    int n = 0;
                    while(position_to_barata[x][y][n]!=roach_code){
                        n++;
                    }
                    position_to_barata[x][y][n] = -1;

                    wmove(my_win, x, y);
                    waddch(my_win,' ');

                    new_position(&x, &y, client.r_direction[b]);

                    wmove(my_win, x, y);
                    waddch(my_win, v+48);

                    //update matrices barataid_to_pos and position_to_barata

                    barataid_to_pos[roach_code][0] = x;
                    barataid_to_pos[roach_code][1] = y;
                    
                    n = 0;
                    while(position_to_barata[x][y][n]!=-1){
                        n++;
                    }
                    position_to_barata[x][y][n] = roach_code;
                    
                    wrefresh(my_win);

                }

                b++;
            }

        

            client_response.code = client.code;
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

            calc_pos(characters, i, lizard_matrix, client.direction);

            wmove(my_win, characters[i].x, characters[i].y);
            waddch(my_win, characters[i].ch);

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

	return 0;
}