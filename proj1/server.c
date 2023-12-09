
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
#include <math.h>

typedef struct char_n_pos {

   char ch;

   int code;

   int x;

   int y;

   int score;

   direction_t direction;

} char_n_pos;

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

void calc_pos(char_n_pos lizards[25], int i, int *lizard_matrix, direction_t direction){

    int n_of_client = lizards[i].ch - 65;

    bool collision = false;

    int x = lizards[i].x;
    int y = lizards[i].y;
    //int pos = x*WINDOW_SIZE + y;

    switch (direction)
    {
    case UP:
        (x) --;
        if(x ==0)
            x = 2;
        break;

    case DOWN:
        (x) ++;
        if(x ==WINDOW_SIZE-1)
            x = WINDOW_SIZE-3;
        break;

    case LEFT:
        (y) --;
        if(y ==0)
            y = 2;
        break;

    case RIGHT:
        (y) ++;
        if(y ==WINDOW_SIZE-1)
            y = WINDOW_SIZE-3;
        break;

    default:
        break;
    }

    if(lizard_matrix[x*WINDOW_SIZE + y] >= 0){
            collision = true;

            int lizard_score = lizards[i].score;
            int other_lizard_id = lizard_matrix[x*WINDOW_SIZE + y];
            int other_lizard_score = lizards[other_lizard_id].score; 

            lizards[i].score = round((lizard_score+other_lizard_score)/2);
            lizards[other_lizard_id].score = lizards[i].score;
    }

    if(!collision){
            lizard_matrix[x*WINDOW_SIZE + y] = n_of_client;
            lizard_matrix[lizards[i].x*WINDOW_SIZE + lizards[i].y] = -1;
            lizards[i].x = x;
            lizards[i].y = y;
            lizards[i].direction = direction;
    }

}

void update_window(WINDOW * my_win, char_n_pos lizard, int mode){

    int i = 0;

    if(mode==0){ //delete lizard at previous position

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
    //added this
    int id_roach = 0;
    int id_roach_client = 0;
    int max_roaches = WINDOW_SIZE*WINDOW_SIZE/3;
    int r_space = max_roaches;

    time_t r_respawn_list[max_roaches];
    time_t now;
    int c_a_index = 0;
    int c_r_index = 0;
    int time_to_r_id[max_roaches];

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

    n = 0;
    while(n!=max_roaches){
        barataid_to_pos[n][0] = 0;
        barataid_to_pos[n][1] = 0;
        barataid_to_pos[n][2] = 0;
        barataid_to_pos[n][3] = 0;

        r_respawn_list[n]=0;

        code_to_barataid[n][0] = 0;
        code_to_barataid[n][1] = 0;
        code_to_barataid[n][2] = 0;

        n++;
    }

    



    // ----------------------------

    struct char_n_pos lizards[25];
    int i=0;

    int* lizard_matrix = (int *) malloc(WINDOW_SIZE*WINDOW_SIZE*sizeof(int));

    for(i=0; i < WINDOW_SIZE*WINDOW_SIZE; i++){
        lizard_matrix[i] = -1;
        if(i<25)
            lizards[i].code = 0;
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
    screen.old_x=0;

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
    bool server_full = false;

    //direction_t  direction;

    i = 0;

    while (1)
    {

        // receive message from client trough socket
        zmq_recv (responder, &client, sizeof(remote_char_t), 0);

        //check time to see if a roach can respawn
        time(&now);
        if(now>=r_respawn_list[c_r_index] && r_respawn_list[c_r_index]!=0){
            int id = time_to_r_id[c_r_index];
            barataid_to_pos[id][3]=0;
            //roach x pos
            int x = 1+rand()%(WINDOW_SIZE-3);
            barataid_to_pos[id][0] = x;
            //roach y pos
            int y = 1+rand()%(WINDOW_SIZE-3);
            barataid_to_pos[id][1] = y;

            i=0;
            while(position_to_barata[x][y][i]!=-1){
                i++;
            }
            position_to_barata[x][y][i] = barataid_to_pos[id][2];

            c_r_index++;
        }
        if(c_r_index==max_roaches){
            c_r_index=0;
        }

        // process lizard connection messages
        if(client.msg_type == 0){

            //srand((int)time); // uncomment THIS!
            for(i=0; i < 25; i++){
                if(lizards[i].code == 0){
                    char_to_give = n_of_char_to_give + i;
                    break;
                }
            }

            if(char_to_give == '0')    //server full
                server_full = true;
            
            if(!server_full){
                while(j!=1000){
                    x_rand = rand() % (WINDOW_SIZE-1);
                    y_rand = rand() % (WINDOW_SIZE-1);
                    if(lizard_matrix[x_rand*WINDOW_SIZE + y_rand] == -1){ // ADICIONAR VERIFICAÇAO PARA ROACHES!
                        found_empty_spot = true;
                        break;
                    }
                    j++;
                }
            }

            if(found_empty_spot){
                
                lizards[i].ch = char_to_give;
                lizards[i].code = rand();
                lizards[i].x = x_rand;
                lizards[i].y = y_rand;
                lizards[i].score = 0;

                lizard_matrix[lizards[i].x*WINDOW_SIZE + lizards[i].y] = lizards[i].ch - 65; //number of lizard occupying space

                client_response.code = lizards[i].code;
                client_response.assigned_char = lizards[i].ch;
                client_response.status = 1;
                client_response.score = 0;
            }
            if(server_full){
                client_response.status = -1; // error -1: server full
            }
            if(!server_full && !found_empty_spot){
                client_response.status = -2; // error -2: no empty space
            }

            char_to_give = '0';

            zmq_send (responder, &client_response, sizeof(client_response), 0);
        }
        
        // process lizard movement message  
        if(client.msg_type == 1){

            for(i=0; i < 25; i++){
                if(lizards[i].ch == client.ch && lizards[i].code == client.code){
                    break;
                }
            }

            screen.old_x = lizards[i].x;
            screen.old_y = lizards[i].y;
            screen.old_direction = lizards[i].direction;

            update_window(my_win, lizards[i], 0); // clean previous lizard

            calc_pos(lizards, i, lizard_matrix, client.direction); // calculate new position

            //----------------------------------------------------------


            int q = 0;
            int eaten_barata_id = 0;
            while (q!=max_roaches){
                if (position_to_barata[lizards[i].x][lizards[i].y][q]!=-1){
                    
                    eaten_barata_id = position_to_barata[lizards[i].x][lizards[i].y][q];

                    lizards[i].score = lizards[i].score + barataid_to_pos[eaten_barata_id][2];

                    barataid_to_pos[eaten_barata_id][3] = 1;
                    position_to_barata[lizards[i].x][lizards[i].y][q]=-1;

                    r_respawn_list[c_a_index]= time(&now) + 5;
                    time_to_r_id[c_a_index] = eaten_barata_id;

                    c_a_index++;
                    if (c_a_index==max_roaches){
                        c_a_index=0;
                    }
                }
                q++;
            }

            //----------------------------------------------------------

            update_window(my_win, lizards[i], 1); // draw new lizard

            wrefresh(my_win);

            client_response.code = lizards[i].code;
            client_response.assigned_char = lizards[i].ch;
            client_response.status = 1;
            client_response.score = lizards[i].score;

            zmq_send (responder, &client_response, sizeof(client_response), 0);

            screen.ch = lizards[i].ch;
            screen.new_x = lizards[i].x;
            screen.new_y = lizards[i].y;
            screen.new_direction = lizards[i].direction;
            screen.msg_type = 1;
            zmq_send(publisher, buffer, strlen(buffer), ZMQ_SNDMORE);
            zmq_send(publisher, &screen, sizeof(screen), 0);
        }

        // process lizard leave message
        if(client.msg_type == 2){

            for(i=0; i < 25; i++){
                if(lizards[i].ch == client.ch && lizards[i].code == client.code){
                    break;
                }
            }

            lizard_matrix[lizards[i].x*WINDOW_SIZE + lizards[i].y] = -1; //number of lizard occupying space
            wmove(my_win, lizards[i].x, lizards[i].y);
            waddch(my_win,' ');
            wrefresh(my_win);

            client_response.code = lizards[i].code;
            client_response.assigned_char = lizards[i].ch;
            client_response.status = 2;
            zmq_send (responder, &client_response, sizeof(client_response), 0);

            client_response.code = 0;
            client_response.assigned_char = 0;
            client_response.status = 0;

            lizards[i].ch = 0;
            lizards[i].score = 0;
            lizards[i].code = 0;
            lizards[i].x = 0;
            lizards[i].y = 0;
        }

        // process roach connection message
        if(client.msg_type == 3){


            int n_roaches = client.n_roaches;

            if(r_space-n_roaches>=0){
                //accept connection
                r_space = r_space-n_roaches;

                int code = rand();

                int i = 0;
                while (i!=n_roaches){
                    code_to_barataid[id_roach_client+i][0] = code;
                    code_to_barataid[id_roach_client+i][1] = i;
                    code_to_barataid[id_roach_client+i][2] = id_roach;
                    
                    //roach x pos
                    int x = 1+rand()%(WINDOW_SIZE-3);
                    barataid_to_pos[id_roach][0] = x;
                    //roach y pos
                    int y = 1+rand()%(WINDOW_SIZE-3);
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
            }
            else{
                //reject
                roach_response.status = 0;
            }
           
            zmq_send (responder, &roach_response, sizeof(roach_response), 0);
        }

        // process roach movement message
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

                    if (barataid_to_pos[roach_code][3]!=1){
                        
                        
                        int x = barataid_to_pos[roach_code][0];
                        int y = barataid_to_pos[roach_code][1];
                        int v = barataid_to_pos[roach_code][2];

                        //store old information
                        int old_x = x;
                        int old_y = y;
                        
                        //calculate new position
                        new_position(&x, &y, client.r_direction[b]);

                        if(lizard_matrix[x*WINDOW_SIZE+y]<0){
                            //no lizards there, roach can move

                            //delete old information about barata position on the 3d matrix
                            int n = 0;
                            while(position_to_barata[old_x][old_y][n]!=roach_code){
                                n++;
                            }
                            position_to_barata[old_x][old_y][n] = -1;

                            wmove(my_win, old_x, old_y);
                            waddch(my_win,' ');

                            //move the roach

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

                            screen.screen_roaches[b][0] = x;
                            screen.screen_roaches[b][1] = y;
                            screen.screen_roaches[b][2] = v;
                            screen.screen_roaches[b][3] = roach_code;


                        }
                        else
                            screen.screen_roaches[b][3] = -1;
                    }
                    wrefresh(my_win);
                }
                else
                    screen.screen_roaches[b][3] = -1;
                b++;
            }


            client_response.code = client.code;
            client_response.status = 1;
        

            zmq_send (responder, &client_response, sizeof(client_response), 0);
            
            screen.msg_type = 3;
            zmq_send(publisher, buffer, strlen(buffer), ZMQ_SNDMORE);
            zmq_send(publisher, &screen, sizeof(screen), 0);
        }

    }
  	endwin();			// End curses mode

    free(lizard_matrix);

	return 0;
}