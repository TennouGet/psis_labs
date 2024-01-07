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

//keeps timestamp of when to kick client
time_t* times_kicker_array;
//keeps type of client and secret code
int* codes_kicker_matrix;

int time_to_kick = 60;


lizards_struct lizards[25];
int* lizard_matrix;

int xyz_to_p( int x, int y, int z ) {
    return (z * WINDOW_SIZE * WINDOW_SIZE) + (y * WINDOW_SIZE) + x;
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
    bool wasp_collision = false;

    int x = lizards[i].x;
    int y = lizards[i].y;

    switch (direction)
    {
    case 0:
        (x) --;
        if(x ==0)
            x = 2;
        if(x ==-1)
            x = 1;
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
    if(position_to_barata[xyz_to_p(x,y,0)] != -1){
        //get bug id
        int bug_id = position_to_barata[xyz_to_p(x,y,0)];
        //check if that bug is a wasp (negative score)
        if (barataid_to_pos[bug_id*4+2] < 0){
            wasp_collision = true;
            lizards[i].score -= 10;
        }

    }

    if(!collision && !wasp_collision){
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
    int old_score;

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
    screen.has_state = 1;
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

            //enter client into inactivity kick table
            time(&now);

            i = 0;
            while(times_kicker_array[i]!=0){
                i++;
            }
            times_kicker_array[i] = now + time_to_kick;
            codes_kicker_matrix[i*2+0] = 0;
            codes_kicker_matrix[i*2+1] = client_response->code;
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

            old_score = lizards[i].score;

            calc_pos(i, lizard_matrix, client->direction); // calculate new position

            //----------------------------------------------------------


            int q = 0;
            int eaten_barata_id = 0;
            while (position_to_barata[xyz_to_p(lizards[i].x,lizards[i].y,q)]!=-1){
                
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
                
                q++;
            }

            //----------------------------------------------------------

            if(lizards[i].score > 49 && old_score > 49)
                lizards[i].state = 2; // classify as winner lizard
            else if(lizards[i].score <= 49 && lizards[i].score >= 0 && old_score <= 49 && old_score <= 49){
                lizards[i].state = 1; // classify as normal lizard
            }
            else if(lizards[i].score >= 0 && old_score < 0)
                lizards[i].state = 5; // classify as turned normal lizard
            else if(lizards[i].score < 0 && old_score >= 0)
                lizards[i].state = 3; // classify as turned loser lizard
            else if(lizards[i].score < 0 && old_score < 0)
                lizards[i].state = 4; // classify as loser lizard

            old_score = 0;

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
            screen.state = lizards[i].state;
            screen.new_x = lizards[i].x;
            screen.new_y = lizards[i].y;
            screen.new_direction = lizards[i].direction;
            screen.msg_type = 1;



            //update time to kick if inactive
            int n = 0;
            while(codes_kicker_matrix[n*2+1]!=client->code){
                n++;
            }
            time(&now);
            times_kicker_array[n] = now + time_to_kick;




            
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
            client_response->assigned_char = strdup(&lizards[i].ch);;
            client_response->status = 2;

            screen.ch = strdup(&lizards[i].ch);
            screen.score = lizards[i].score;
            screen.new_x = lizards[i].x;
            screen.new_y = lizards[i].y;
            screen.new_direction = lizards[i].direction;
            screen.old_direction = lizards[i].direction;
            screen.msg_type = 2;


            msg_len = response_to_client__get_packed_size(client_response);
            msg_buf = malloc(msg_len);
            response_to_client__pack(client_response, msg_buf);
            zmq_send (responder, msg_buf, msg_len, 0);
            free(msg_buf);

            zmq_send(publisher, buffer, strlen(buffer), ZMQ_SNDMORE);
            msg_len = remote_screen__get_packed_size(&screen);
            msg_buf = malloc(msg_len);
            remote_screen__pack(&screen, msg_buf);
            zmq_send (publisher, msg_buf, msg_len, 0);
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

                //enter client into inactivity kick table
                time(&now);

                i = 0;
                while(times_kicker_array[i]!=0){
                    i++;
                }
                times_kicker_array[i] = now + time_to_kick;
                codes_kicker_matrix[i*2+0] = 1;
                codes_kicker_matrix[i*2+1] = code;

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
                    int ctrl_wasp = 0;
                    
                    //do not accept move message to a dead roach
                    if (barataid_to_pos[roach_code*4+3]!=1){
                        
                        
                        int x = barataid_to_pos[roach_code*4+0];
                        int y = barataid_to_pos[roach_code*4+1];
                        int v = barataid_to_pos[roach_code*4+2];

                        //if it is a wasp move, must not let move into other animals
                        if (v<10){
                            ctrl_wasp = 1;
                        }
                        
                        //store old information
                        int old_x = x;
                        int old_y = y;
                        
                        //calculate new position
                        new_position(&x, &y, client->r_direction[b]);

                        if(lizard_matrix[x*WINDOW_SIZE+y]<0){
                            //no lizards there, roach can move
                            
                            //check if there is a wasp in new spot
                            int ctrl_other_wasp = 0;
                            //check if spot is occupied
                            if(position_to_barata[xyz_to_p(x,y,0)] != -1){
                                //get bug id
                                int bug_id = position_to_barata[xyz_to_p(x,y,0)];
                                //check if that bug is a wasp (negative score)
                                if (barataid_to_pos[bug_id*4+2] < 0){
                                    ctrl_other_wasp = 1;
                                }
                            }

                            //if it is a roach, it can't move into a wasp
                            //if it is a wasp, it cannot move into an occupied space
                            if((ctrl_wasp == 0 && ctrl_other_wasp == 0) || (position_to_barata[xyz_to_p(x,y,0)] == -1)){
                                //delete old information about barata position on the 3d matrix
                                int n = 0;
                                while(position_to_barata[xyz_to_p(old_x,old_y,n)]!=roach_code){
                                    n++;
                                }
                                position_to_barata[xyz_to_p(old_x,old_y,n)] = -1;

                                //if there exists a roach on top of the current one on 3D matrix, push it down, recursively
                                while(position_to_barata[xyz_to_p(old_x,old_y,n+1)]!=-1){
                                    position_to_barata[xyz_to_p(old_x,old_y,n)] = position_to_barata[xyz_to_p(old_x,old_y,n+1)];
                                    position_to_barata[xyz_to_p(old_x,old_y,n+1)] = -1;
                                    n++;
                                }  
                                

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
                        }
                        else{
                            //there is a lizard in new pos
                            if (ctrl_wasp==0){
                                //roach can't move
                                screen.screen_roaches[b*4+3] = -1;
                            }
                            if (ctrl_wasp==1){
                                screen.screen_roaches[b*4+3] = -1;
                                //wasp stungs lizard
                                int i = lizard_matrix[x*WINDOW_SIZE + y];
                                lizards[i].score -= 10;
                            }
                        }
                            
                    }
                }
                else
                    screen.screen_roaches[b*4+3] = -1;
                b++;
            }


            roach_response.code = client->code;
            roach_response.status = 1;



            //update time to kick if inactive
            int n = 0;
            while(codes_kicker_matrix[n*2+1]!=client->code){
                n++;
            }
            time(&now);
            times_kicker_array[n] = now + time_to_kick;

        

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

        // process roach leave message
        if (client->msg_type == 5){

            int i = 0;
            while(i!=max_roaches){
                if (code_to_barataid[i*2+0]==client->code){
                    
                    //prepare message to screen, with the universal roach IDS of the leaving roaches
                    int b = code_to_barataid[i*2+1];
                    screen.screen_roaches[b*4+0] = -1;
                    screen.screen_roaches[b*4+1] = -1;
                    screen.screen_roaches[b*4+2] = -1;
                    screen.screen_roaches[b*4+3] = i;

                    //reset matrix that translates secret code to universal code
                    code_to_barataid[i*2+0] = 0;
                    code_to_barataid[i*2+1] = 0;

                    //reset 3D matrix that shows the roaches present in an xy spot, with z being the dimension that stacks roaches
                    //check if roach is eaten, if it is eaten it is not on the 3D matrix
                    if(barataid_to_pos[i*4+3] == 0){

                        int p = 0;
                        while( position_to_barata[xyz_to_p(barataid_to_pos[i*4+0],barataid_to_pos[i*4+1],p)]!=i){
                            p++;
                        }
                        position_to_barata[xyz_to_p(barataid_to_pos[i*4+0],barataid_to_pos[i*4+1],p)] = -1;

                    }

                    //reset matrix that gives x,y and v of a roach when inputing the universal roach ID
                    barataid_to_pos[i*4+0] = 0;
                    barataid_to_pos[i*4+1] = 0;
                    barataid_to_pos[i*4+2] = 0;
                    barataid_to_pos[i*4+3] = 0;

                }


                i++;
            }

            roach_response.status = 2;
            msg_len = response_to_client__get_packed_size(&roach_response);
            msg_buf = malloc(msg_len);
            response_to_client__pack(&roach_response, msg_buf);
            zmq_send (responder, msg_buf, msg_len, 0);
            free(msg_buf);



            screen.msg_type = 4;
            zmq_send(publisher, buffer, strlen(buffer), ZMQ_SNDMORE);
            msg_len = remote_screen__get_packed_size(&screen);
            msg_buf = malloc(msg_len);
            remote_screen__pack(&screen, msg_buf);
            zmq_send (publisher, msg_buf, msg_len, 0);

        
        }



    }

}

void *thread_timer( void *ptr ){

    long int thread_number = (long int)ptr;

    zmq_msg_t zmq_msg;
    zmq_msg_init(&zmq_msg);

    int i=0;

    int msg_len;
    void * msg_data;
    char * msg_buf;


    RemoteScreen screen = REMOTE_SCREEN__INIT;



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

    //prepare a roach move message to remote screen (used after roach client disconnects to respawn the roach)
    //int i = 0;
    //while(i!=10){
    //    screen.screen_roaches[i*4+3] = -1;
    //    i++;
    //} 
    

    while(1){
        
        //check time to see if a roach can respawn
        time(&now);
        int update = 0;

        if(now<r_respawn_list[c_r_index] && r_respawn_list[c_r_index]!=0){
            sleep(r_respawn_list[c_r_index]-now + 0.01);
        }
        else{
            sleep(4);
        }


        while(now>r_respawn_list[c_r_index] && r_respawn_list[c_r_index]!=0){
            int id = time_to_r_id[c_r_index];

            //must check if client of the roach did not disconnect before spawning a roach
            if (barataid_to_pos[id]!=0){
                
                //spawning the roach
                barataid_to_pos[id*4+3]=0;
                //roach x pos
                int x = 1+rand()%(WINDOW_SIZE-3);
                barataid_to_pos[id*4+0] = x;
                //roach y pos
                int y = 1+rand()%(WINDOW_SIZE-3);
                barataid_to_pos[id*4+1] = y;

                int v = barataid_to_pos[id*4+2];

                i=0;
                while(position_to_barata[xyz_to_p(x,y,i)]!=-1){
                    i++;
                }
                position_to_barata[xyz_to_p(x,y,i)] = id;



                //store information to update screen
                update = 1;

                int b = code_to_barataid[id*2+1];

                screen.screen_roaches[b*4+0] = x;
                screen.screen_roaches[b*4+1] = y;
                screen.screen_roaches[b*4+2] = v;
                screen.screen_roaches[b*4+3] = id;
            }

            r_respawn_list[c_r_index] = 0;
            c_r_index++;
        }
        if(c_r_index==max_roaches){
            c_r_index=0;
        }
        if(update==1){
            //send update to remote screen
            screen.msg_type = 3;
            zmq_send(publisher, buffer, strlen(buffer), ZMQ_SNDMORE);
            msg_len = remote_screen__get_packed_size(&screen);
            msg_buf = malloc(msg_len);
            remote_screen__pack(&screen, msg_buf);
            zmq_send (publisher, msg_buf, msg_len, 0);
        }

    }

}

void *thread_kicker( void *ptr ){

    
    void *requester_lizard = zmq_socket(context, ZMQ_REQ);
    zmq_connect(requester_lizard, "tcp://localhost:5555");

    void *requester_roach_wasp = zmq_socket(context, ZMQ_REQ);
    zmq_connect(requester_roach_wasp, "tcp://localhost:5556");

    ResponseToClient * client_response;

    ClientLizardMessage leave_lizard = CLIENT_LIZARD_MESSAGE__INIT;
    leave_lizard.has_msg_type = 1;
    leave_lizard.msg_type = 2;
    leave_lizard.has_code = 1;
    leave_lizard.has_direction = 1;

    ClientRoachesMessage leave_roach_wasp = CLIENT_ROACHES_MESSAGE__INIT;
    leave_roach_wasp.has_msg_type = 1;
    leave_roach_wasp.msg_type = 5;
    leave_roach_wasp.has_code = 1;
    leave_roach_wasp.has_n_roaches = 0;
    leave_roach_wasp.n_roaches = 0;

    int msg_len;
    char * msg_buf;
    void * msg_data;
    zmq_msg_t zmq_msg;
    zmq_msg_init(&zmq_msg);

    while(1){

        sleep(20);
        time(&now);

        int n = 0;
        while (n!= max_roaches+26){

            if(now>times_kicker_array[n] && times_kicker_array[n] != 0 && codes_kicker_matrix[n*2+0]==0){

                leave_lizard.code = codes_kicker_matrix[n*2+1];

                int i = 0;
                while(lizards[i].code != leave_lizard.code){
                    i++;
                }

                leave_lizard.ch = strdup(&lizards[i].ch);

                // send lizard kick message
                msg_len = client_lizard_message__get_packed_size(&leave_lizard);
                msg_buf = malloc(msg_len);
                client_lizard_message__pack(&leave_lizard, msg_buf);
                zmq_send (requester_lizard, msg_buf, msg_len, 0);
                free(msg_buf);

                // receive confirmation
                msg_len = zmq_recvmsg(requester_lizard, &zmq_msg, 0); 
                msg_data = zmq_msg_data(&zmq_msg);
                client_response = response_to_client__unpack(NULL, msg_len, msg_data);

                codes_kicker_matrix[n*2+0] = 0;
                codes_kicker_matrix[n*2+1] = 1;
                times_kicker_array[n] = 0;


            }

            if(now>times_kicker_array[n] && times_kicker_array[n] != 0 && codes_kicker_matrix[n*2+0]==1){

                leave_roach_wasp.code = codes_kicker_matrix[n*2+1];

                // send roach kick message
                msg_len = client_roaches_message__get_packed_size(&leave_roach_wasp);
                msg_buf = malloc(msg_len);
                client_roaches_message__pack(&leave_roach_wasp, msg_buf);
                zmq_send (requester_roach_wasp, msg_buf, msg_len, 0);
                free(msg_buf);

                // receive confirmation
                msg_len = zmq_recvmsg(requester_roach_wasp, &zmq_msg, 0); 
                msg_data = zmq_msg_data(&zmq_msg);
                client_response = response_to_client__unpack(NULL, msg_len, msg_data);

                codes_kicker_matrix[n*2+0] = 0;
                codes_kicker_matrix[n*2+1] = 1;
                times_kicker_array[n] = 0;
            }
            n++;

        }

    }
    

    


}


// display thread and functions

void update_window(WINDOW * my_win, RemoteScreen * screen, int mode){

    int i = 0;

    /*
    mode 0 delete lizard
    mode 1 move normal lizard
    mode 2 move winner lizard
    mode 3 move lizard that just turned loser
    mode 4 move loser lizard
    mode 5 move lizard that just turned normal
    */

    // matrix for things to put back

    // erase old lizard char
    mvwaddch(my_win, screen->old_x, screen->old_y, ' ');
    

    if(mode < 4){ // delete lizard tail at previous position

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

    if(mode==1 || mode==2 || mode==5){ // add lizard tail at new position

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

    }

    // draw new lizard char
    mvwaddch(my_win, screen->new_x, screen->new_y, *screen->ch);

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

            update_window(my_win, screen2, screen2->state);

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
                    if (v > 0){
                        mvwaddch(my_win, new_x, new_y, v + 48);
                    }
                    else{
                        mvwaddch(my_win, new_x, new_y, 35);
                    }
                    
                    roaches[ID][0] = new_x;
                    roaches[ID][1] = new_y;
                    roaches[ID][2] = v;
                }
            }
            
            

            box(my_win, 0 , 0);	
            wrefresh(my_win);
        }

        if(screen2->msg_type == 4){ // process roaches leave

            int old_x = 0;
            int old_y = 0;
            int ID = 0;

            for(i=0; i<10; i++){
                ID = screen2->screen_roaches[i*4 + 3];
                old_x = roaches[ID][0];
                old_y = roaches[ID][1];


                mvwaddch(my_win, old_x, old_y, ' ');
                    
                roaches[ID][0] = 0;
                roaches[ID][1] = 0;
                roaches[ID][2] = 0;
                
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

    // Socket to receive roaches/wasps requests
    responder = zmq_socket(context, ZMQ_REP);
    rc = zmq_bind(responder, "tcp://*:5556");
    assert(rc == 0);


    // Variable initialization

    lizard_matrix = (int *) malloc(WINDOW_SIZE*WINDOW_SIZE*sizeof(int));

    r_respawn_list = (time_t *) malloc(max_roaches*sizeof(time_t));

    time_to_r_id = (int *) malloc(max_roaches*sizeof(int));

    code_to_barataid = (int *) malloc(max_roaches*2*sizeof(int));

    barataid_to_pos = (int *) malloc(max_roaches*4*sizeof(int));

    position_to_barata = (int *) malloc(WINDOW_SIZE*WINDOW_SIZE*max_roaches*sizeof(int));

    times_kicker_array = (time_t *) malloc((max_roaches+26)*sizeof(time_t));

    codes_kicker_matrix = (int *) malloc((max_roaches+26)*2*sizeof(int));;

    



    for(int i=0; i < WINDOW_SIZE*WINDOW_SIZE; i++){
        lizard_matrix[i] = -1;
        if(i<25){
            lizards[i].code = 0;
            lizards[i].state = 1;
        }
    }

    int n = 0;

    while(n!=WINDOW_SIZE*WINDOW_SIZE*max_roaches){

        position_to_barata[n] = -1;

        n++;
    }

    n = 0;
    while(n!=max_roaches+26){
        times_kicker_array[n] = 0;
        codes_kicker_matrix[n*2+0] = 0;
        codes_kicker_matrix[n*2+1] = 0;
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

    // Spawn roach respawn thread
    pthread_t roach_respawn;
    pthread_create( &roach_respawn, NULL, thread_timer, (void *) worker_nbr);

    // Spawn kicker thread
    pthread_t kicker;
    pthread_create( &kicker, NULL, thread_kicker, (void *) 0);

    //  Start the proxy
    zmq_proxy (frontend, backend, NULL);

    pthread_mutex_destroy(&mux_next_random);

}