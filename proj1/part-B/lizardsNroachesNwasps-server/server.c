#include <ncurses.h>
#include "../header.h"
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
#include "../messages.pb-c.h"

pthread_mutex_t mux_next_random; // UNDERSTAND THIS

// GLOBAL VARIABLES

void *context;
void *publisher;
char buffer[20] = "screen";

char FRONT_PORT[20];
char PUB_PORT[20];

//added this
int id_roach = 0;
int const max_roaches = WINDOW_SIZE*WINDOW_SIZE/3;
int r_space = max_roaches;

time_t r_respawn_list[max_roaches];
time_t now;
int c_a_index = 0;
int c_r_index = 0;
int time_to_r_id[max_roaches];

// client code | client barata code (0-9) | server barata code (0-(maxroaches-1))
int code_to_barataid[max_roaches][2];
// INDEXED BY: server barata code (0-(maxroaches-1)) : pos x | pos y | barata value
int barataid_to_pos[max_roaches][4];
// pos x || pos y || server barata code (0-(maxroaches-1))
int position_to_barata[WINDOW_SIZE][WINDOW_SIZE][max_roaches];


lizards_struct lizards[25];
int* lizard_matrix;

// lizard thread and functions

ResponseToClient * lizard_connect(int* lizard_matrix, lizards_struct lizards[25]){

    bool found_empty_spot = false;
    bool server_full = false;
    int n_of_char_to_give = 97;
    char char_to_give = 0;
    int x_rand=0;
    int y_rand=0;
    int i = 0;
    int j = 0;

    ResponseToClient * client_response;
    
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
                    client_response->assigned_char = lizards[i].ch;
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

    return client_response;

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
    ClientLizardMessage * client;
    ResponseToClient * client_response;
    RemoteScreen * screen;

    while (1){

        //zmq_recv (responder, &client, sizeof(client_message), 0);
        msg_len = zmq_recvmsg(responder, &zmq_msg, 0); 
        msg_data = zmq_msg_data(&zmq_msg);
        client = client_lizard_message__unpack(NULL, msg_len, msg_data);

        client->msg_type = 1;

        // process lizard connection messages
        if(client->msg_type == 0){

            client_response = lizard_connect(lizard_matrix, lizards);
            zmq_send (responder, &client_response, sizeof(client_response), 0);
        }
        
        // process lizard movement message
        if(client->msg_type == 1){

            for(i=0; i < 25; i++){
                if(lizards[i].ch == *client->ch && lizards[i].code == client->code){
                    break;
                }
            }

            screen->old_x = lizards[i].x;
            screen->old_y = lizards[i].y;
            screen->old_direction = lizards[i].direction;

            update_window(my_win, lizards[i], 0); // clean previous lizard

            calc_pos(lizards, i, lizard_matrix, client->direction); // calculate new position

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

            if(lizards[i].score > 49)
                lizards[i].winner = true; // classify as a new winner lizard

            if(lizards[i].winner == true)
                update_window(my_win, lizards[i], 2); // draw new winner lizard
            else
                update_window(my_win, lizards[i], 1); // draw new lizard

            box(my_win, 0 , 0);

            char str[40];
            snprintf(str, sizeof(str), "\rLizard %c score: %d.", lizards[i].ch, lizards[i].score);

            mvwaddstr(text_win, i, 0, str);
            wrefresh(my_win);
            wrefresh(text_win);

            client_response->code = lizards[i].code;
            client_response->assigned_char = lizards[i].ch;
            client_response->status = 1;
            client_response->score = lizards[i].score;

            zmq_send (responder, &client_response, sizeof(client_response), 0);

            *screen->ch = lizards[i].ch;
            screen->score = lizards[i].score;
            screen->new_x = lizards[i].x;
            screen->new_y = lizards[i].y;
            screen->new_direction = lizards[i].direction;
            screen->msg_type = 1;
            zmq_send(publisher, buffer, strlen(buffer), ZMQ_SNDMORE);
            zmq_send(publisher, &screen, sizeof(screen), 0);
        }

        // process lizard leave message
        if(client->msg_type == 2){

                for(i=0; i < 25; i++){
                    if(lizards[i].ch == *client->ch && lizards[i].code == client->code){
                        break;
                    }
                }

                lizard_matrix[lizards[i].x*WINDOW_SIZE + lizards[i].y] = -1; //number of lizard occupying space
                wmove(my_win, lizards[i].x, lizards[i].y);
                waddch(my_win,' ');
                update_window(my_win, lizards[i], 0); // erase leaving lizard
                box(my_win, 0 , 0);

                wmove(text_win, lizards[i].ch - 97, 0);
                wclrtoeol(text_win);
                wrefresh(text_win);
                wrefresh(my_win);

                client_response->code = lizards[i].code;
                client_response->assigned_char = lizards[i].ch;
                client_response->status = 2;

                *screen->ch = lizards[i].ch;
                screen->score = lizards[i].score;
                screen->new_x = lizards[i].x;
                screen->new_y = lizards[i].y;
                screen->new_direction = lizards[i].direction;
                screen->old_direction = lizards[i].direction;
                screen->msg_type = 2;
                zmq_send(publisher, buffer, strlen(buffer), ZMQ_SNDMORE);
                zmq_send(publisher, &screen, sizeof(screen), 0);

                zmq_send (responder, &client_response, sizeof(client_response), 0);

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
    void *pub_context = zmq_ctx_new();
    publisher = zmq_socket(pub_context, ZMQ_PUB);
    int rc2 = zmq_bind(publisher, PUB_PORT);
    assert(rc2 == 0);


    // Variable initialization

    lizard_matrix = (int *) malloc(WINDOW_SIZE*WINDOW_SIZE*sizeof(int));

    for(int i=0; i < WINDOW_SIZE*WINDOW_SIZE; i++){
        lizard_matrix[i] = -1;
        if(i<25){
            lizards[i].code = 0;
            lizards[i].winner = false;
        }
    }

    // Spawn threads
    long int worker_nbr;
    for (worker_nbr = 0; worker_nbr < 4; worker_nbr++) {
        pthread_t worker;
        pthread_create( &worker, NULL, thread_lizards, (void *) worker_nbr);
    }

    //  Start the proxy
    zmq_proxy (frontend, backend, NULL);

    pthread_mutex_destroy(&mux_next_random);

}