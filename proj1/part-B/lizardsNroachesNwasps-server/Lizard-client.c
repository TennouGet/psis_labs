#include <ncurses.h>
#include "../header.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <ctype.h> 
#include <stdlib.h>
#include <string.h>

#include <zmq.h>
#include <pthread.h>
#include "messages.pb-c.h"

int main(int argc, char* argv[])
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

    //ClientLizardMessage * join;
    ClientLizardMessage join = CLIENT_LIZARD_MESSAGE__INIT;
    ClientLizardMessage move = CLIENT_LIZARD_MESSAGE__INIT;
    ClientLizardMessage * leave;
    ResponseToClient * client_response;

    zmq_msg_t zmq_msg;
    zmq_msg_init(&zmq_msg);

    join.has_msg_type = 1;
    join.msg_type = 0;

    join.has_code = 1;
    join.code = 54;

    join.ch = 0;

    join.has_direction = 1;
    join.direction = 1;


    move.has_msg_type = 1;
    move.has_code = 1;
    move.has_direction = 1;
    
    

    
    //zmq_send (requester, &join, sizeof(join), 0);
    //zmq_recv (requester, &client_response, sizeof(client_response), 0);

    // send connection message
    int msg_len = client_lizard_message__get_packed_size(&join);
    char * msg_buf = malloc(msg_len);
    client_lizard_message__pack(&join, msg_buf);
    zmq_send (requester, msg_buf, msg_len, 0);
    //free(msg_buf);

    msg_len = zmq_recvmsg(requester, &zmq_msg, 0); 
    void * msg_data = zmq_msg_data(&zmq_msg);
    client_response = response_to_client__unpack(NULL, msg_len, msg_data);


    if(client_response->status == 1){
        printf("Server response OK, lizard created. Your char: %s\n", client_response->assigned_char);
    }
    if(client_response->status == -2){
        printf("Server ERROR: WINDOW FULL, TRY AGAIN. CODE: %d\n", client_response->status);
        return 0;
    }

    move.ch = client_response->assigned_char;
    move.msg_type = 1;
    move.code = client_response->code;


	initscr();			/* Start curses mode 		*/
	cbreak();				/* Line buffering disabled	*/
	keypad(stdscr, TRUE);		/* We get F1, F2 etc..		*/
	noecho();			/* Don't echo() while we do getch */

    mvprintw(1, 0, "\rConnected, you can now move using the arrow keys.");
    mvprintw(2, 0, "\rYour assigned char: %s", client_response->assigned_char);

    int ch;
    bool exit_program = false;

    int n = 0;
    do
    {
    	ch = getch();	
        n++;
        clear();
        switch (ch)
        {
            case KEY_LEFT:
                mvprintw(0,0,"%d Left arrow is pressed", n);
                move.direction = 2;
                break;
            case KEY_RIGHT:
                mvprintw(0,0,"%d Right arrow is pressed", n);
                move.direction = 3;
                break;
            case KEY_DOWN:
                mvprintw(0,0,"%d Down arrow is pressed", n);
                move.direction = 1;
                break;
            case KEY_UP:
                mvprintw(0,0,"%d Up arrow is pressed", n);

                move.direction = 0;
                break;
            case 'q':
                mvprintw(0,0,"q is pressed, sending exit message");
                exit_program = true;
            case 'Q':
                mvprintw(0,0,"Q is pressed, sending exit message");
                exit_program = true;
            default:
                ch = 'x';
                    break;
        }
        refresh();			// Print it on to the real screen
        
        if(exit_program){
            endwin();

            //send byebye message to server
            leave->msg_type = 2;
            leave->code = move.code;
            leave->ch = move.ch;
            //zmq_send (requester, &leave, sizeof(leave), 0);
            //zmq_recv (requester, &client_response, sizeof(client_response), 0);

            msg_len = client_lizard_message__get_packed_size(leave);
            msg_buf = malloc(msg_len);
            client_lizard_message__pack(leave, msg_buf);
            zmq_send (requester, msg_buf, msg_len, 0);
            free(msg_buf);

            msg_len = zmq_recvmsg(requester, &zmq_msg, 0); 
            msg_data = zmq_msg_data(&zmq_msg);
            client_response = response_to_client__unpack(NULL, msg_len, msg_data);

            if(client_response->status == 2){
                printf("exiting..\n");
                endwin();
                zmq_close (requester);
                zmq_ctx_destroy (context);
                printf("Disconnected from server.\n");
                return 0;
            }
            else{
                printf("error while trying to exit");
            }
        }

        //send the movement message
        //zmq_send (requester, &move, sizeof(move), 0);
        //zmq_recv (requester, &client_response, sizeof(client_response), 0);

        msg_len = client_lizard_message__get_packed_size(&move);
        msg_buf = malloc(msg_len);
        client_lizard_message__pack(&move, msg_buf);
        zmq_send (requester, msg_buf, msg_len, 0);
        free(msg_buf);

        msg_len = zmq_recvmsg(requester, &zmq_msg, 0); 
        msg_data = zmq_msg_data(&zmq_msg);
        client_response = response_to_client__unpack(NULL, msg_len, msg_data);

        if(client_response->status == 1)
            mvprintw(1, 0, "\rServer status: OK");
        if(client_response->status == -1 || client_response->status == -2){
            endwin();
            zmq_close (requester);
            zmq_ctx_destroy (context);
            if(client_response->status == -1)
                printf("\rError -1: server full, try again.");
            if(client_response->status == -2)
                printf("Error -2: no empty space to deploy lizard, try again.\n");
            return 0;
        }
        if(client_response->status != 1 && client_response->status != -1 && client_response->status != -2)
            mvprintw(1, 0, "\rError, server response: %d", client_response->status);

        mvprintw(2, 0, "\rYour assigned char: %s", client_response->assigned_char);
        mvprintw(3, 0, "\rYour score: %d", client_response->score);
        mvprintw(4, 0, "\r");
        
    }while(ch != 27);
    
    
  	endwin();			// End curses mode

    zmq_close (requester);
    zmq_ctx_destroy (context);

	return 0;
}