#include <ncurses.h>
#include "remote-char.h"
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


   
    //TODO_4
    // create and open the FIFO for writing

    printf ("Connecting to server…");
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);
    zmq_connect (requester, "tcp://localhost:5555");
    printf ("\rConnected to server!\n");

    //TODO_5
    // read the character from the user

    char character[10];

    printf("Select a character to use:");
	fgets(character, 10, stdin);
    printf("Selected character: %s.", character);

    struct remote_char_t join, move;

    join.msg_type = 0;
    join.ch = character[0];

    move.ch = character[0];
    move.msg_type = 1;

    // TODO_6
    // send connection message


    char buffer[10];
    zmq_send (requester, &join, sizeof(join), 0);
    zmq_recv (requester, buffer, 10, 0);
    printf("received: %s", buffer);



	initscr();			/* Start curses mode 		*/
	cbreak();				/* Line buffering disabled	*/
	keypad(stdscr, TRUE);		/* We get F1, F2 etc..		*/
	noecho();			/* Don't echo() while we do getch */

    int ch;

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
                mvprintw(0,0,"%d :Up arrow is pressed", n);
                move.direction = UP;
                break;
            default:
                ch = 'x';
                    break;
        }
        refresh();			/* Print it on to the real screen */
        //TODO_9
        // prepare the movement message
        
        // done!
        
        //TODO_10
        //send the movement message

        zmq_send (requester, &move, sizeof(move), 0);
        zmq_recv (requester, buffer, 10, 0);
        printf("\rreceived: %s", buffer);
        
    }while(ch != 27);
    
    
  	endwin();			/* End curses mode		  */

    zmq_close (requester);
    zmq_ctx_destroy (context);

	return 0;
}