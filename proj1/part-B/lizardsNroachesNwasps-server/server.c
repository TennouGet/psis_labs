#include <ncurses.h>
#include "header.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include <zmq.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <pthread.h>
#include "messages.pb-c.h"

#include <stdint.h>

pthread_mutex_t mux_next_random; // UNDERSTAND THIS

// GLOBAL VARIABLES

void *context;
void *bugs_context;
void *publisher;
void *responder;
char buffer[20] = "screen";

char FRONT_PORT[20];
char PUB_PORT[20];

//added this
int id_roach = 0;
int const max_roaches = WINDOW_SIZE*WINDOW_SIZE/3;
int r_space = max_roaches;


time_t now;
int c_a_index = 0;
int c_r_index = 0;


//contains a list with timestamps for roaches respawn
time_t* r_respawn_list;
//contains a list with the ids of the roaches that respawn
int* time_to_r_id;
// client code | client barata code (0-9) | server barata code (0-(maxroaches-1))
int* code_to_barataid;
// INDEXED BY: server barata code (0-(maxroaches-1)) : pos x | pos y | barata value
int* barataid_to_pos;
// pos x || pos y || server barata code (0-(maxroaches-1))
int* position_to_barata;

lizards_struct lizards[25];
int* lizard_matrix;

int xyz_to_p( int x, int y, int z ) {
    return (z * WINDOW_SIZE * WINDOW_SIZE) + (y * WINDOW_SIZE) + x;
}

int xy_to_p(int x, int y, int maxX, int n_col){
    return (x*n_col+y);
}


matrix_translation p_to_xyz( int idx ) {
    struct matrix_translation result;

    result.z = idx / (WINDOW_SIZE * WINDOW_SIZE);
    idx -= (result.z * WINDOW_SIZE * WINDOW_SIZE);
    result.y = idx / WINDOW_SIZE;
    result.x = idx % WINDOW_SIZE;


    return result;
}

matrix_translation p_to_xy(int p, int n_col){
    struct matrix_translation result;

    result.x = (int) (p/n_col);
    result.y = p - (result.x*n_col);
    result.z = 0;

    return result;

}

// lizard thread and functions

void new_position(int* x, int *y, int direction){
    switch (direction)
    {
    case 0:
        (*x) --;
        if(*x ==0)
            *x = 2;
        break;
    case 1:
        (*x) ++;
        if(*x ==WINDOW_SIZE-1)
            *x = WINDOW_SIZE-3;
        break;
    case 2:
        (*y) --;
        if(*y ==0)
            *y = 2;
        break;
    case 3:
        (*y) ++;
        if(*y ==WINDOW_SIZE-1)
            *y = WINDOW_SIZE-3;
        break;
    default:
        break;
    }
}

void calc_pos(int i, int *lizard_matrix, int direction){

    int n_of_client = lizards[i].ch - 97;

    bool collision = false;

    int x = lizards[i].x;
    int y = lizards[i].y;
    //printf("x: %d, y: %d.\n", lizards[i].x, lizards[i].y);
    //int pos = x*WINDOW_SIZE + y;

    switch (direction)
    {
    case 0:
        (x) --;
        if(x ==0)
            x = 2;
        break;

    case 1:
        (x) ++;
        if(x ==WINDOW_SIZE-1)
            x = WINDOW_SIZE-3;
        break;

    case 2:
        (y) --;
        if(y ==0)
            y = 2;
        break;

    case 3:
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

void lizard_connect(int* lizard_matrix, lizards_struct lizards[25], ResponseToClient * client_response){

    bool found_empty_spot = false;
    bool server_full = false;
    int n_of_char_to_give = 97;
    char char_to_give = 0;
    int x_rand=0;
    int y_rand=0;
    int i = 0;
    int j = 0;

    
    
    //srand((int)time); // uncomment THIS!

    pthread_mutex_lock(&mux_next_random); // lock to make sure two threads don't get the same i
    
    for(i=0; i < 25; i++){
        if(lizards[i].code == 0){
            char_to_give = n_of_char_to_give + i;

            while(j!=1000){        // also needs to be locked to make sure two lizards don't try to get placed on the same position
                
                x_rand = rand() % (WINDOW_SIZE-1);
                y_rand = rand() % (WINDOW_SIZE-1); 
                
                if(lizard_matrix[x_rand*WINDOW_SIZE + y_rand] == -1){ // ADICIONAR VERIFICAÇAO PARA ROACHES!

                    lizard_matrix[x_rand*WINDOW_SIZE + y_rand] = lizards[i].ch - 97; //number of lizard occupying space

                    // from here on could be unlocked, but it's inside an if (?)

                    found_empty_spot = true;
                    lizards[i].ch = char_to_give;
                    lizards[i].code = rand();
                    lizards[i].x = x_rand;
                    lizards[i].y = y_rand;
                    lizards[i].score = 0;

                    client_response->code = lizards[i].code;
                    //char buffer [2];
                    //strcat(buffer, &lizards[i].ch);
                    client_response->assigned_char = strdup(&lizards[i].ch);
                    client_response->status = 1;
                    client_response->score = 0; 

                    break;
                }
                j++;
            } 
            break;
        }
    }
    
    pthread_mutex_unlock(&mux_next_random);

    if(char_to_give == '0'){    //server full
        server_full = true;
        client_response->status = -1; // error -1: server full
    }

    if(!server_full && !found_empty_spot){
        client_response->status = -2; // error -2: no empty space
    }

    char_to_give = '0';

    return;

}

void *thread_lizards( void *ptr ){

    long int thread_number = (long int)ptr;

    void *responder = zmq_socket(context, ZMQ_REP);
    int rc = zmq_connect(responder, "inproc://back-end");
    assert(rc == 0);

    zmq_msg_t zmq_msg;
    zmq_msg_init(&zmq_msg);

    int i=0;

    int msg_len;
    void * msg_data;
    char * msg_buf;
    ClientLizardMessage * client;
    ResponseToClient * client_response;
    ResponseToClient client_response_ori = RESPONSE_TO_CLIENT__INIT; 
    RemoteScreen screen = REMOTE_SCREEN__INIT;

    client_response = &client_response_ori;

    client_response->has_status = 1;
    client_response->has_code = 1;
    client_response->has_score = 1;

    screen.has_msg_type = 1;
    screen.has_old_x = 1;
    screen.has_old_y = 1;
    screen.has_new_x = 1;
    screen.has_new_y = 1;
    screen.has_score = 1;
    screen.has_old_direction = 1;
    screen.has_new_direction = 1;

    while (1){

        //zmq_recv (responder, &client, sizeof(client_message), 0);
        msg_len = zmq_recvmsg(responder, &zmq_msg, 0); 
        msg_data = zmq_msg_data(&zmq_msg);
        client = client_lizard_message__unpack(NULL, msg_len, msg_data); 


        // process lizard connection messages
        if(client->msg_type == 0){

            client_response = &client_response_ori;

            lizard_connect(lizard_matrix, lizards, client_response);

            client_response = &client_response_ori;


            msg_len = response_to_client__get_packed_size(client_response);
            char * msg_buf = malloc(msg_len);
            response_to_client__pack(client_response, msg_buf);
            zmq_send (responder, msg_buf, msg_len, 0);
            //free(msg_buf);
            free(client_response_ori.assigned_char);
        }
        
        // process lizard movement message
        if(client->msg_type == 1){

            for(i=0; i < 25; i++){
                if(lizards[i].ch == *client->ch && lizards[i].code == client->code){
                    break;
                }
            }

            screen.old_x = lizards[i].x;
            screen.old_y = lizards[i].y;
            screen.old_direction = lizards[i].direction;

            calc_pos(i, lizard_matrix, client->direction); // calculate new position

            //----------------------------------------------------------


            int q = 0;
            int eaten_barata_id = 0;
            while (q!=max_roaches){
                if (position_to_barata[xyz_to_p(lizards[i].x,lizards[i].y,q)]!=-1){
                    
                    eaten_barata_id = position_to_barata[xyz_to_p(lizards[i].x,lizards[i].y,q)];

                    lizards[i].score = lizards[i].score + barataid_to_pos[eaten_barata_id*4+2];

                    barataid_to_pos[eaten_barata_id*4+3] = 1;
                    position_to_barata[xyz_to_p(lizards[i].x,lizards[i].y,q)]=-1;

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

            if(lizards[i].score > 49)
                lizards[i].winner = true; // classify as a new winner lizard

            client_response->code = lizards[i].code;
            client_response->assigned_char = strdup(&lizards[i].ch);
            client_response->status = 1;
            client_response->score = lizards[i].score;

            msg_len = response_to_client__get_packed_size(client_response);
            msg_buf = malloc(msg_len);
            response_to_client__pack(client_response, msg_buf);
            zmq_send (responder, msg_buf, msg_len, 0);
            free(msg_buf);

            screen.ch = strdup(&lizards[i].ch);
            screen.score = lizards[i].score;
            screen.new_x = lizards[i].x;
            screen.new_y = lizards[i].y;
            screen.new_direction = lizards[i].direction;
            screen.msg_type = 1;


            
            zmq_send(publisher, buffer, strlen(buffer), ZMQ_SNDMORE);
            msg_len = remote_screen__get_packed_size(&screen);
            msg_buf = malloc(msg_len);
            remote_screen__pack(&screen, msg_buf);
            zmq_send (publisher, msg_buf, msg_len, 0);
            //free(msg_buf);
            //free(screen.ch);
            //printf("hi");
        }

        // process lizard leave message
        if(client->msg_type == 2){

                for(i=0; i < 25; i++){
                    if(lizards[i].ch == *client->ch && lizards[i].code == client->code){
                        break;
                    }
                }

                lizard_matrix[lizards[i].x*WINDOW_SIZE + lizards[i].y] = -1; //number of lizard occupying space

                client_response->code = lizards[i].code;
                *client_response->assigned_char = lizards[i].ch;
                client_response->status = 2;

                *screen.ch = lizards[i].ch;
                screen.score = lizards[i].score;
                screen.new_x = lizards[i].x;
                screen.new_y = lizards[i].y;
                screen.new_direction = lizards[i].direction;
                screen.old_direction = lizards[i].direction;
                screen.msg_type = 2;

                zmq_send(publisher, buffer, strlen(buffer), ZMQ_SNDMORE);
                msg_len = remote_screen__get_packed_size(&screen);
                msg_buf = malloc(msg_len);
                remote_screen__pack(&screen, msg_buf);
                zmq_send (responder, msg_buf, msg_len, 0);
                free(msg_buf);


                msg_len = response_to_client__get_packed_size(client_response);
                msg_buf = malloc(msg_len);
                response_to_client__pack(client_response, msg_buf);
                zmq_send (responder, msg_buf, msg_len, 0);
                free(msg_buf);

                client_response->code = 0;
                client_response->assigned_char = 0;
                client_response->status = 0;

                lizards[i].ch = 0;
                lizards[i].score = 0;
                lizards[i].code = 0;
                lizards[i].x = 0;
                lizards[i].y = 0;
            }
    
    }

    zmq_msg_close (&zmq_msg);

}

void *thread_bugs( void *ptr ){

    long int thread_number = (long int)ptr;

    zmq_msg_t zmq_msg;
    zmq_msg_init(&zmq_msg);

    int i=0;

    int msg_len;
    void * msg_data;
    char * msg_buf;
    ClientRoachesMessage * client;
    ResponseToClient roach_response = RESPONSE_TO_CLIENT__INIT; ;

    RemoteScreen screen = REMOTE_SCREEN__INIT;


    roach_response.has_status = 1;
    roach_response.has_code = 1;
    roach_response.has_score = 0;

    screen.has_msg_type = 1;
    screen.has_old_x = 0;
    screen.has_old_y = 0;
    screen.has_new_x = 0;
    screen.has_new_y = 0;
    screen.has_score = 0;
    screen.has_old_direction = 0;
    screen.has_new_direction = 0;

    screen.screen_roaches = malloc(sizeof(int)*40);;
    screen.n_screen_roaches = 40;

    while(1){

        msg_len = zmq_recvmsg(responder, &zmq_msg, 0); 
        msg_data = zmq_msg_data(&zmq_msg);
        client = client_roaches_message__unpack(NULL, msg_len, msg_data); 

        // process roach connection message
        if(client->msg_type == 3){


            int n_roaches = client->n_roaches;

            if(r_space-n_roaches>=0){
                //accept connection
                r_space = r_space-n_roaches;

                int code = rand();

                int i = 0;
                while (i!=n_roaches){
                    code_to_barataid[id_roach*2+0] = code;
                    code_to_barataid[id_roach*2+1] = i;
                    
                    //roach x pos
                    int x = 1+rand()%(WINDOW_SIZE-3);
                    barataid_to_pos[id_roach*4+0] = x;
                    //roach y pos
                    int y = 1+rand()%(WINDOW_SIZE-3);
                    barataid_to_pos[id_roach*4+1] = y;
                    //roach value
                    barataid_to_pos[id_roach*4+2] = client->r_scores[i];


                    //storing the roach in a 3 dimensional matrix that represents the map and the high represents the amount of roaches on the spot
                    int n = 0;
                    while(position_to_barata[xyz_to_p(x,y,n)]!=-1){
                        n++;
                    }
                    position_to_barata[xyz_to_p(x,y,n)] = id_roach;


                    id_roach++;
                    i++;
                }



                roach_response.status = 1;
                roach_response.code = code;
            }
            else{
                //reject
                roach_response.status = 0;
            }
            

            msg_len = response_to_client__get_packed_size(&roach_response);
            msg_buf = malloc(msg_len);
            response_to_client__pack(&roach_response, msg_buf);
            zmq_send (responder, msg_buf, msg_len, 0);
            free(msg_buf);
        }

        // process roach movement message
        if(client->msg_type == 4){

            int i = 0;
            while(code_to_barataid[i*2+0]!= client->code){
                i++;
            }


            int b = 0;
            while (b!=10){
                if (client->r_bool[b]==1){
                    
                    while (code_to_barataid[i*2+1]!= b){
                        i++;
                    }

                    int roach_code = i;

                    if (barataid_to_pos[roach_code*4+3]!=1){
                        
                        
                        int x = barataid_to_pos[roach_code*4+0];
                        int y = barataid_to_pos[roach_code*4+1];
                        int v = barataid_to_pos[roach_code*4+2];

                        //store old information
                        int old_x = x;
                        int old_y = y;
                        
                        //calculate new position
                        new_position(&x, &y, client->r_direction[b]);

                        if(lizard_matrix[x*WINDOW_SIZE+y]<0){
                            //no lizards there, roach can move

                            //delete old information about barata position on the 3d matrix
                            int n = 0;
                            while(position_to_barata[xyz_to_p(old_x,old_y,n)]!=roach_code){
                                n++;
                            }
                            position_to_barata[xyz_to_p(old_x,old_y,n)] = -1;

                            

                            //update matrices barataid_to_pos and position_to_barata

                            barataid_to_pos[roach_code*4+0] = x;
                            barataid_to_pos[roach_code*4+1] = y;
                            
                            n = 0;
                            while(position_to_barata[xyz_to_p(x,y,n)]!=-1){
                                n++;
                            }
                            position_to_barata[xyz_to_p(x,y,n)] = roach_code;

                            screen.screen_roaches[b*4+0] = x;
                            screen.screen_roaches[b*4+1] = y;
                            screen.screen_roaches[b*4+2] = v;
                            screen.screen_roaches[b*4+3] = roach_code;


                        }
                        else
                            screen.screen_roaches[b*4+3] = -1;
                    }
                }
                else
                    screen.screen_roaches[b*4+3] = -1;
                b++;
            }


            roach_response.code = client->code;
            roach_response.status = 1;
        

            msg_len = response_to_client__get_packed_size(&roach_response);
            msg_buf = malloc(msg_len);
            response_to_client__pack(&roach_response, msg_buf);
            zmq_send (responder, msg_buf, msg_len, 0);
            free(msg_buf);


            
            screen.msg_type = 3;
            zmq_send(publisher, buffer, strlen(buffer), ZMQ_SNDMORE);
            msg_len = remote_screen__get_packed_size(&screen);
            msg_buf = malloc(msg_len);
            remote_screen__pack(&screen, msg_buf);
            zmq_send (publisher, msg_buf, msg_len, 0);

        }

    }

}


// display thread and functions

void update_window(WINDOW * my_win, RemoteScreen * screen, int mode){

    int i = 0;

    if(mode==0 || mode==1 || mode==2){ //delete lizard at previous position

        mvwaddch(my_win, screen->old_x, screen->old_y, ' ');

        switch (screen->old_direction)
        {
        case 0:                         // VERIFY IF THIS WORKS!!
            for(i=0; i < 6; i++){
                if(mode==0)
                    mvwaddch(my_win, screen->new_x+i, screen->new_y, ' ');
                else
                    mvwaddch(my_win, screen->old_x+i, screen->old_y, ' ');
            }
            break;
        case 1:
            for(i=0; i < 6; i++){
                if(mode==0)
                    mvwaddch(my_win, screen->new_x-i, screen->new_y, ' ');
                else
                    mvwaddch(my_win, screen->old_x-i, screen->old_y, ' ');
            }
            break;
        case 2:
            for(i=0; i < 6; i++){
                if(mode==0)
                    mvwaddch(my_win, screen->new_x, screen->new_y+i, ' ');
                else
                    mvwaddch(my_win, screen->old_x, screen->old_y+i, ' ');
            }
            break;
        case 3:
            for(i=0; i < 6; i++){
                if(mode==0)
                    mvwaddch(my_win, screen->new_x, screen->new_y-i, ' ');
                else
                    mvwaddch(my_win, screen->old_x, screen->old_y-i, ' ');
            }
            break;
        default:
            mvprintw(WINDOW_SIZE+3, 0, "\rERROR: no lizard direction.");
            break;
        }
    }

    if(mode==1 || mode==2){ //add lizard at new position
        
        switch (screen->new_direction)
        {
        case 0:
            for(i=0; i < 6; i++){
                if(mode == 2)
                    mvwaddch(my_win,screen->new_x+i, screen->new_y, '*');
                else
                mvwaddch(my_win, screen->new_x+i, screen->new_y, '.');
            }
            break;
        case 1:
            for(i=0; i < 6; i++){
                if(mode == 2)
                    mvwaddch(my_win, screen->new_x-i, screen->new_y, '*');
                else
                mvwaddch(my_win, screen->new_x-i, screen->new_y, '.');
            }
            break;
        case 2:
            for(i=0; i < 6; i++){
                if(mode == 2)
                    mvwaddch(my_win,screen->new_x, screen->new_y+i, '*');
                else
                mvwaddch(my_win, screen->new_x, screen->new_y+i, '.');
            }
            break;
        case 3:
            for(i=0; i < 6; i++){
                if(mode == 2)
                    mvwaddch(my_win, screen->new_x, screen->new_y-i, '*');
                else
                mvwaddch(my_win, screen->new_x, screen->new_y-i, '.');
            }
            break;
        default:
            mvprintw(WINDOW_SIZE+3, 0, "\rERROR: no lizard direction.");
            break;
        }

        mvwaddch(my_win, screen->new_x, screen->new_y, *screen->ch);
    }

    mvprintw(WINDOW_SIZE + 4 + (*screen->ch - 97), 4, "\rLizard %c score: %d.", *screen->ch, screen->score);

}

void *thread_display(void *PORT)
{	
    char * sub_PORT = (char *) PORT;

    int max_roaches = round((WINDOW_SIZE*WINDOW_SIZE)/3);

    // screen matrix 
    int roaches[max_roaches][3];

    int n = 0;
    while(n!=max_roaches){
        roaches[n][0]=0;
        roaches[n][1]=0;
        roaches[n][2]=0;
        n++;
    }

    char sub_name[20] = "screen";

    char IP_nPORT[40] = "";

    strcat(IP_nPORT, sub_PORT);

    // Socket to subscribe
    void *subscriber = zmq_socket(context, ZMQ_SUB);
    zmq_connect(subscriber, "tcp://localhost:5557");
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, sub_name, strlen(sub_name));

    zmq_msg_t zmq_msg;
    zmq_msg_init(&zmq_msg);
    int msg_len;
    void * msg_data;
    RemoteScreen * screen2;

    printf("Connected to server");

    printf("sub_name: %s", sub_name);

    char buffer[20];
    
    // ncurses initialization
	initscr();
	cbreak();
    keypad(stdscr, TRUE);
	noecho();


    // creates a window and draws a border
    WINDOW * my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);
    WINDOW * text_win = newwin(26, 40, WINDOW_SIZE, 0);
    box(my_win, 0 , 0);
	wrefresh(my_win);
    wrefresh(text_win);

    while (1)
    {

        // receive message from screen trough socket
        zmq_recv (subscriber, buffer, strlen(buffer), 0);
        msg_len = zmq_recvmsg(subscriber, &zmq_msg, 0);
        msg_data = zmq_msg_data(&zmq_msg);
        screen2 = remote_screen__unpack(NULL, msg_len, msg_data);
        
        // process the movement message

        int i = 0;
        
        if(screen2->msg_type == 1){ // process lizard movement

            if(screen2->score > 49)
                update_window(my_win, screen2, 2);
            else
                update_window(my_win, screen2, 1);

            box(my_win, 0 , 0);
            char str[40];
            snprintf(str, sizeof(str), "\rLizard %s score: %d.", screen2->ch, screen2->score);

            mvwaddstr(text_win, *screen2->ch - 97, 0, str);
            wrefresh(text_win);
            wrefresh(my_win);
        }

        if(screen2->msg_type == 2){ // process lizard leave

            wmove(my_win, screen2->new_x, screen2->new_y);
            waddch(my_win,' ');
            update_window(my_win, screen2, 0); // erase leaving lizard
            box(my_win, 0 , 0);
            wmove(text_win, *screen2->ch - 97, 0);
            wclrtoeol(text_win);
            wrefresh(text_win);
            wrefresh(my_win);

        }

        if(screen2->msg_type == 3){ // process roaches movement

            int old_x = 0;
            int old_y = 0;
            int new_x = 0;
            int new_y = 0;
            int v = 0;
            int ID = 0;

            for(i=0; i<10; i++){
                ID = screen2->screen_roaches[i*4 + 3];
                old_x = roaches[ID][0];
                old_y = roaches[ID][1];
                new_x = screen2->screen_roaches[i*4 + 0];
                new_y = screen2->screen_roaches[i*4 + 1];
                v = screen2->screen_roaches[i*4 + 2];

                if(ID > -1){
                    if(v != 0){
                        mvwaddch(my_win, old_x, old_y, ' ');
                    }
                    mvwaddch(my_win, new_x, new_y, v + 48);
                    roaches[ID][0] = new_x;
                    roaches[ID][1] = new_y;
                    roaches[ID][2] = v;
                }
            }

            box(my_win, 0 , 0);	
            wrefresh(my_win);
        }

        //zmq_msg_close(msg_data);
        //free(screen);
        //zmq_msg_close (&zmq_msg);
    }

  	endwin();			// End curses mode
}

// server and functions

int input_treatment(int argc, char *argv[]){

    if((argc == 2 || argc == 3)  && (atoi(argv[1]) < 0 || atoi(argv[1]) > 99999)){
        printf("Invalid REP port, try again.\n");
        return 0;
    }
    if(argc == 3 && (atoi(argv[2]) < 0 || atoi(argv[2]) > 99999)){
        printf("Invalid PUB port, try again.\n");
        return 0;
    }

    if(argc == 1){
        strcat(FRONT_PORT, "tcp://*:5555");
        strcat(PUB_PORT, "tcp://*:5557");
        printf("No input arguments, default ports used (REP:5555/PUB:5557) (arg 1 - REP, arg 2 - PUB).\n");
    }
    if(argc == 2){
        strcat(FRONT_PORT, "tcp://*:");
        strcat(FRONT_PORT, argv[1]);
        strcat(PUB_PORT, "tcp://*:5557");
        printf("No input PUB port, default PUB port used (PUB:5557)) (arg 1 - REP, arg 2 - PUB).\n");
    }
    if(argc == 3){
        strcat(FRONT_PORT, "tcp://*:");
        strcat(FRONT_PORT, argv[1]);
        strcat(PUB_PORT, "tcp://*:");
        strcat(PUB_PORT, argv[2]);
    }
    if(argc > 3){
        printf("Too many arguments, please insert REP port and PUB port only.\n");
        return 0;
    }

    return 1;
}


int main(int argc, char* argv[]){

    if(input_treatment(argc, argv) == 0){
        return 0;
    }
    
    pthread_mutex_init(&mux_next_random, NULL);

    context = zmq_ctx_new();
    
    //  Socket facing clients
    void *frontend = zmq_socket(context, ZMQ_ROUTER);
    int rc = zmq_bind (frontend, "tcp://*:5555"); // CHANGE TO TCP
    assert (rc == 0);

    //  Socket facing services
    void *backend = zmq_socket (context, ZMQ_DEALER);
    rc = zmq_bind (backend, "inproc://back-end");
    assert (rc == 0);

    // Socket to publish to displays
    publisher = zmq_socket(context, ZMQ_PUB);
    rc = zmq_bind(publisher, PUB_PORT);
    assert(rc == 0);

    responder = zmq_socket(context, ZMQ_REP);
    int rc3 = zmq_bind(responder, "tcp://*:5556");
    assert(rc3 == 0);


    // Variable initialization

    lizard_matrix = (int *) malloc(WINDOW_SIZE*WINDOW_SIZE*sizeof(int));

    r_respawn_list = (time_t *) malloc(max_roaches*sizeof(int));

    time_to_r_id = (int *) malloc(max_roaches*sizeof(int));

    code_to_barataid = (int *) malloc(max_roaches*2*sizeof(int));

    barataid_to_pos = (int *) malloc(max_roaches*4*sizeof(int));

    position_to_barata = (int *) malloc(WINDOW_SIZE*WINDOW_SIZE*max_roaches*sizeof(int));



    for(int i=0; i < WINDOW_SIZE*WINDOW_SIZE; i++){
        lizard_matrix[i] = -1;
        if(i<25){
            lizards[i].code = 0;
            lizards[i].winner = false;
        }
    }

    int n = 0;

    while(n!=WINDOW_SIZE*WINDOW_SIZE*max_roaches){

        position_to_barata[n] = -1;

        n++;
    }


    n = 0;
    while(n!=max_roaches){
        r_respawn_list[n] = 0;

        time_to_r_id[n] = 0;
        
        code_to_barataid[n*2+0] = 0;
        code_to_barataid[n*2+1] = 0;

        barataid_to_pos[n*4+0] = 0;
        barataid_to_pos[n*4+1] = 0;
        barataid_to_pos[n*4+2] = 0;
        barataid_to_pos[n*4+3] = 0;

        n++;
    }

    // Spawn lizard threads
    long int worker_nbr;
    for (worker_nbr = 0; worker_nbr < 4; worker_nbr++) {
        pthread_t worker;
        pthread_create( &worker, NULL, thread_lizards, (void *) worker_nbr);
    }

    // Spawn display thread
    pthread_t display;
    pthread_create( &display, NULL, thread_display, (void *) PUB_PORT); // ADD IP_nPORT

    // Spawn display thread
    pthread_t bugs;
    pthread_create( &bugs, NULL, thread_bugs, (void *) worker_nbr);

    //  Start the proxy
    zmq_proxy (frontend, backend, NULL);

    pthread_mutex_destroy(&mux_next_random);

}