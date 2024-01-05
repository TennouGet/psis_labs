#include "../header.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <ctype.h> 
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <zmq.h>
#include <time.h>
#include <pthread.h>
#include "../messages.pb-c.h"


int random_direction(){
    return  random()%4;
}

int main(int argc, char **argv)
{

    if(argc == 3 && (atoi(argv[2]) < 0 || atoi(argv[2]) > 99999)){
        printf("Invalid REQ port, try again.\n");
        return 0;
    }

    char IP_n_PORT[40] = "";

    if(argc == 1){
        strcat(IP_n_PORT, "tcp://localhost:5555");
    }
    if(argc == 2){
        strcat(IP_n_PORT, "tcp://");
        strcat(IP_n_PORT, argv[1]);
        strcat(IP_n_PORT, ":");
        strcat(IP_n_PORT, "5555");
    }
    if(argc == 3){
        strcat(IP_n_PORT, "tcp://");
        strcat(IP_n_PORT, argv[1]);
        strcat(IP_n_PORT, ":");
        strcat(IP_n_PORT, argv[2]);
    }
    if(argc > 3){
        printf("Too many arguments, please insert IP and REQ port only.\n");
        return 0;
    }

    printf ("Connecting to server…");
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);
    int code = zmq_connect (requester, IP_n_PORT);
    printf ("\rConnected to server? %d\n", code);


    zmq_msg_t zmq_msg;
    zmq_msg_init(&zmq_msg);

    int msg_len;
    void * msg_data;


    ClientRoachesMessage * join;
    ClientRoachesMessage * move;

    ResponseToClient * roach_response;

    
    //struct client_message join, move;
    //struct response_to_client roach_response;

    join->msg_type = 3;
    join->code = 0;

    time_t t;
    srand((unsigned) time(&t));

    int n_roaches = 1 + rand()%9;
    //n_roaches=10;

    int i = 0;
    while (i!=n_roaches){
        join->r_scores[i] = rand()%4 + 1;
        printf("score %d\n",join->r_scores[i]);
        i++;
    }

    
    join->n_roaches = n_roaches;
    
    char * msg_buf;

    // send connection message
    msg_len = client_roaches_message__get_packed_size(join);
    msg_buf = malloc(msg_len);
    client_roaches_message__pack(join, msg_buf);
    zmq_send (requester, msg_buf, msg_len, 0);
    free(msg_buf);

    // receive server response
    msg_len = zmq_recvmsg(requester, &zmq_msg, 0); 
    msg_data = zmq_msg_data(&zmq_msg);
    roach_response = response_to_client__unpack(NULL, msg_len, msg_data);    

    if(roach_response->status == 1){
        printf("Server response OK, roaches created.\n");
    }
    else{
        printf("Too many roaches! They are everywhere!\n");
        exit(0);
    }


    
    printf("roaches: %d\n", n_roaches);

    move->msg_type = 4;
    move->code = roach_response->code;

    int sleep_delay;
    int  direction;

    while (1){
        sleep_delay = 1000+random()%700000;
        usleep(sleep_delay);

        
        int i = 0;
        while(i!=10){
            move->r_bool[i] = 0;
            i++;
        }
        i = 0;
        while (i!=n_roaches){
            if (random()%3==0){
                /* calculates new direction */
                direction = random_direction();
                printf("%d\n",direction);

                move->r_direction[i] = direction;
                move->r_bool[i] = 1;
                printf("roach %d moved %d\n",i,move->r_direction[i]);
            }
            else{
                move->r_bool[i] = 0;
            }
            
            i++;
        }

        


        //chosen_roach = random()%n_roaches;
       
        
        //move.id = chosen_roach;

        
        //printf("%d chosen roach\n",move.id);


        msg_len = client_roaches_message__get_packed_size(move);
        msg_buf = malloc(msg_len);
        client_roaches_message__pack(move, msg_buf);
        zmq_send (requester, msg_buf, msg_len, 0);
        free(msg_buf);


        //receive server response
        msg_len = zmq_recvmsg(requester, &zmq_msg, 0); 
        msg_data = zmq_msg_data(&zmq_msg);
        roach_response = response_to_client__unpack(NULL, msg_len, msg_data);  

        if(roach_response->status == 1)
            mvprintw(1, 0, "\rreceived: %d", roach_response->status);
        else
            mvprintw(1, 0, "\rError, server response: %d", roach_response->status);

    }


    zmq_close (requester);
    zmq_ctx_destroy (context);

	return 0;
}