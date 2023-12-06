#include <ncurses.h>
#include "header.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <ctype.h> 
#include <stdlib.h>
#include <string.h>

#include <zmq.h>

int main()
{

    printf ("Connecting to server…");
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);
    int code = zmq_connect (requester, "tcp://localhost:5555");
    printf ("\rConnected to server? %d\n", code);

    struct remote_char_t join, move, leave;
    struct response_to_client client_response;

    join.msg_type = 0;
    join.ch = 0;
    join.code = 0;

    // send connection message
    zmq_send (requester, &join, sizeof(join), 0);
    zmq_recv (requester, &client_response, sizeof(client_response), 0);
    if(client_response.status == 1){
        printf("Server response OK, lizard created. Your char: %c\n", client_response.assigned_char);
    }

    move.ch = client_response.assigned_char;
    move.msg_type = 1;
    move.code = client_response.code;


	initscr();			/* Start curses mode 		*/
	cbreak();				/* Line buffering disabled	*/
	keypad(stdscr, TRUE);		/* We get F1, F2 etc..		*/
	noecho();			/* Don't echo() while we do getch */

    int ch;

    bool exit_program = false;

    int n = 0;
    do
    {
    	ch = getch();		
        n++;
        switch (ch)
        {
            case KEY_LEFT:
                mvprintw(0,0,"%d Left arrow is pressed", n);
                move.direction = LEFT;
                break;
            case KEY_RIGHT:
                mvprintw(0,0,"%d Right arrow is pressed", n);
                move.direction = RIGHT;
                break;
            case KEY_DOWN:
                mvprintw(0,0,"%d Down arrow is pressed", n);
                move.direction = DOWN;
                break;
            case KEY_UP:
                mvprintw(0,0,"%d :Up arrow is pressed, char: %c", n, move.ch);

                move.direction = UP;
                break;
            case 'q':
                mvprintw(0,0,"q is pressed, sending exit message");
                exit_program = true;
            default:
                ch = 'x';
                    break;
        }
        refresh();			// Print it on to the real screen
        
        if(exit_program){
            endwin();

            //send byebye message to server
            leave.msg_type = 2;
            leave.code = move.code;
            leave.ch = move.ch;
            zmq_send (requester, &leave, sizeof(leave), 0);
            zmq_recv (requester, &client_response, sizeof(client_response), 0);
            if(client_response.status == 2){
                printf("exiting..\n");
                endwin();
                zmq_close (requester);
                zmq_ctx_destroy (context);
                printf("Disconnected from server.\n");
                return 0;
            }
            else{
                printf("error exiting");
            }
        }

        //send the movement message
        zmq_send (requester, &move, sizeof(move), 0);
        zmq_recv (requester, &client_response, sizeof(client_response), 0);

        if(client_response.status == 1)
            mvprintw(1, 0, "\rServer status: Ok");
        else
            mvprintw(1, 0, "\rError, server response: %d", client_response.status);
        
    }while(ch != 27);
    
    
  	endwin();			// End curses mode

    zmq_close (requester);
    zmq_ctx_destroy (context);

	return 0;
}