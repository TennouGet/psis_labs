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
#include <time.h>
#include <pthread.h>
#include "messages.pb-c.h"

int leave_ctrl;

int random_direction(){
    return  random()%4;
}

void *thread_disconnect( void *ptr ){

    int ch;
    bool exit_program = false;
    bool valid_ch = false;

    int ctrl = 1;
    while(ctrl)
    {
    	ch = getch();	

        clear();
        switch (ch)
        {
            case 'q':
                printf("q is pressed, sending exit message");
                exit_program = true;
            case 'Q':
                printf("Q is pressed, sending exit message");
                exit_program = true;
            default:
                ch = 'x';
                valid_ch = false;
                    break;
        }

        if(exit_program){ 
            leave_ctrl = 1;
            ctrl = 0;
        }

    }

}



int main(int argc, char **argv)
{

    if(argc == 3 && (atoi(argv[2]) < 0 || atoi(argv[2]) > 99999)){
        printf("Invalid REQ port, try again.\n");
        return 0;
    }

    char IP_n_PORT[40] = "";

    if(argc == 1){
        strcat(IP_n_PORT, "tcp://localhost:5556");
    }
    if(argc == 2){
        strcat(IP_n_PORT, "tcp://");
        strcat(IP_n_PORT, argv[1]);
        strcat(IP_n_PORT, ":");
        strcat(IP_n_PORT, "5556");
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


    ClientRoachesMessage join = CLIENT_ROACHES_MESSAGE__INIT;
    ClientRoachesMessage move = CLIENT_ROACHES_MESSAGE__INIT;
    


    ResponseToClient * roach_response;
    
    join.has_msg_type = 1;
    join.has_n_roaches = 1;
    join.has_code = 1;

    move.has_msg_type = 1;
    move.has_n_roaches = 1;
    move.has_code = 1;


    ClientRoachesMessage leave = CLIENT_ROACHES_MESSAGE__INIT;


    leave.has_msg_type = 1;
    leave.msg_type = 5;


    leave.has_code = 1;

    leave.has_n_roaches = 1;
    leave.n_roaches = 0;


    join.msg_type = 3;
    join.code = 0;

    time_t t;
    srand((unsigned) time(&t));

    int n_roaches = 1 + rand()%9;
    //n_roaches=10;

    join.r_scores = malloc(sizeof(int)*10);;

    int i = 0;
    while (i!=n_roaches){
        join.r_scores[i] = rand()%4 + 1;
        printf("score %d\n",join.r_scores[i]);
        i++;
    }
    join.n_r_scores = n_roaches;

    
    join.n_roaches = n_roaches;
    
    char * msg_buf;

    // send connection message
    msg_len = client_roaches_message__get_packed_size(&join);
    msg_buf = malloc(msg_len);
    client_roaches_message__pack(&join, msg_buf);
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


    leave.code = roach_response->code;
    leave_ctrl = 0;

    // Spawn disconnect thread
    long int worker_nbr;
    pthread_t disconnect;
    pthread_create( &disconnect, NULL, thread_disconnect, (void *) worker_nbr);

    
    printf("roaches: %d\n", n_roaches);

    move.msg_type = 4;
    move.code = roach_response->code;

    int sleep_delay;
    int  direction;

    move.r_bool = malloc(sizeof(int)*10);;
    move.n_r_bool = 10;

    move.r_direction = malloc(sizeof(int)*10);
    move.n_r_direction = n_roaches;


    // ncurses initialization
	initscr();
	cbreak();
    keypad(stdscr, TRUE);
	noecho();

    // creates a window and draws a border
    WINDOW * my_win = newwin(10, 30, 0, 0);
	wrefresh(my_win);

    while (1){
        sleep_delay = 1000+random()%700000;
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

                move.r_direction[i] = direction;
                move.r_bool[i] = 1;
                
                char str[30];
                snprintf(str, sizeof(str),"roach %d moved %d\n",i,move.r_direction[i]);

                mvwaddstr(my_win, 2+i, 0, str);
                wrefresh(my_win);
            }
            else{
                move.r_bool[i] = 0;
            }
            
            i++;
        }

        
        if (leave_ctrl == 1){
            endwin();
            int msg_len = client_roaches_message__get_packed_size(&leave);
            char * msg_buf = malloc(msg_len);
            client_roaches_message__pack(&leave, msg_buf);
            zmq_send (requester, msg_buf, msg_len, 0);
            free(msg_buf);





            msg_len = zmq_recvmsg(requester, &zmq_msg, 0); 
            msg_data = zmq_msg_data(&zmq_msg);
            roach_response = response_to_client__unpack(NULL, msg_len, msg_data);

            if(roach_response->status == 2){
                printf("exiting..\n");
                zmq_close (requester);
                zmq_ctx_destroy (context);
                printf("Disconnected from server.\n");
                break;
            }
            else{
                printf("error while trying to exit");
            }
            
            return 0;
        }
       

        




        msg_len = client_roaches_message__get_packed_size(&move);
        msg_buf = malloc(msg_len);
        client_roaches_message__pack(&move, msg_buf);
        zmq_send (requester, msg_buf, msg_len, 0);
        free(msg_buf);


        //receive server response
        msg_len = zmq_recvmsg(requester, &zmq_msg, 0); 
        msg_data = zmq_msg_data(&zmq_msg);
        roach_response = response_to_client__unpack(NULL, msg_len, msg_data);  

        if(roach_response->status != 1)
            //mvprintw(1, 0, "\rError, server response: %d", roach_response->status);
            return 0;

    }


    zmq_close (requester);
    zmq_ctx_destroy (context);

	return 0;
}