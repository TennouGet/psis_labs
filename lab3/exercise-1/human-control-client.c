#include <ncurses.h>
#include "remote-char.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <ctype.h> 
#include <stdlib.h>
#include <string.h>

int main()
{


   
    //TODO_4
    // create and open the FIFO for writing

    int write_fd = 0;

    while((write_fd = open(SERVER_INPUT, O_WRONLY))== -1){
	  if(mkfifo(SERVER_INPUT, 0666)!=0){
			printf("problem creating the write fifo\n");
			exit(-1);
	  }else{
		  printf("write fifo created\n");
	  }
	}

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

    write(write_fd, &join, sizeof(join));


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

        write(write_fd, &move, sizeof(move));
        
    }while(ch != 27);
    
    
  	endwin();			/* End curses mode		  */

	return 0;
}