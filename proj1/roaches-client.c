#include "header.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <ctype.h> 
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <zmq.h>


direction_t random_direction(){
    return  random()%4;
}

int main(int argc, char **argv)
{

    printf ("Connecting to server…");
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);
    int code = zmq_connect (requester, "tcp://localhost:5555");
    printf ("\rConnected to server? %d\n", code);

    
    struct remote_char_t join, move;
    struct response_to_client roach_response;

    join.msg_type = 3;
    join.code = 0;

    int n_roaches = 1 + rand()%9;
    //n_roaches=10;

    int i = 0;
    while (i!=n_roaches){
        join.r_scores[i] = rand()%4 + 1;
        printf("score %d\n",join.r_scores[i]);
        i++;
    }

    
    join.n_roaches = n_roaches;
    

    // send connection message
    char buffer[10];
    zmq_send (requester, &join, sizeof(join), 0);
    zmq_recv (requester, &roach_response, sizeof(roach_response), 0);
    if(roach_response.status == 1){
        printf("Server response OK, roaches created.\n");
    }
    else{
        printf("Too many roaches! They are everywhere!\n");
        exit(0);
    }


    
    printf("roaches: %d\n", n_roaches);

    move.msg_type = 4;
    move.code = roach_response.code;


    int ch;
    int sleep_delay;
    int chosen_roaches[10];
    direction_t  direction;

    while (1){
        sleep_delay = random()%700000;
        usleep(sleep_delay);

        
        int i = 0;
        while(i!=10){
            move.r_bool[i] = 0;
            i++;
        }
        i = 0;
        while (i!=n_roaches){
            if (random()%3==0){
                /* calculates new direction */
                direction = random_direction();
                printf("%d\n",direction);
                switch (direction)
                {
                case LEFT:
                    move.r_direction[i] = LEFT;
                    break;
                case RIGHT:
                    move.r_direction[i] = RIGHT;
                    break;
                case DOWN:
                    move.r_direction[i] = DOWN;
                    break;
                case UP:
                    move.r_direction[i] = UP;
                    break;
                }
                move.r_bool[i] = 1;
                printf("roach %d moved %d\n",i,move.r_direction[i]);
            }
            else{
                move.r_bool[i] = 0;
            }
            
            i++;
        }

        


        //chosen_roach = random()%n_roaches;
       
        
        //move.id = chosen_roach;

        
        //printf("%d chosen roach\n",move.id);

        


        
        zmq_send (requester, &move, sizeof(move), 0);
        zmq_recv (requester, &roach_response, sizeof(roach_response), 0);
        if(roach_response.status == 1)
            mvprintw(1, 0, "\rreceived: %d", roach_response.status);
        else
            mvprintw(1, 0, "\rError, server response: %d", roach_response.status);

    }


    zmq_close (requester);
    zmq_ctx_destroy (context);

	return 0;
}