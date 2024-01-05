
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
#include <math.h>

#include "../messages.pb-c.h"

void update_window(WINDOW * my_win, RemoteScreen * screen, int mode){

    int i = 0;

    if(mode==0 || mode==1 || mode==2){ //delete lizard at previous position

        mvwaddch(my_win, screen->old_x, screen->old_y, ' ');

        switch (screen->old_direction)
        {
        case 1:                         // VERIFY IF THIS WORKS!!
            for(i=0; i < 6; i++){
                if(mode==0)
                    mvwaddch(my_win, screen->new_x+i, screen->new_y, ' ');
                else
                    mvwaddch(my_win, screen->old_x+i, screen->old_y, ' ');
            }
            break;
        case 2:
            for(i=0; i < 6; i++){
                if(mode==0)
                    mvwaddch(my_win, screen->new_x-i, screen->new_y, ' ');
                else
                    mvwaddch(my_win, screen->old_x-i, screen->old_y, ' ');
            }
            break;
        case 3:
            for(i=0; i < 6; i++){
                if(mode==0)
                    mvwaddch(my_win, screen->new_x, screen->new_y+i, ' ');
                else
                    mvwaddch(my_win, screen->old_x, screen->old_y+i, ' ');
            }
            break;
        case 4:
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
        case 1:
            for(i=0; i < 6; i++){
                if(mode == 2)
                    mvwaddch(my_win,screen->new_x+i, screen->new_y, '*');
                else
                mvwaddch(my_win, screen->new_x+i, screen->new_y, '.');
            }
            break;
        case 2:
            for(i=0; i < 6; i++){
                if(mode == 2)
                    mvwaddch(my_win, screen->new_x-i, screen->new_y, '*');
                else
                mvwaddch(my_win, screen->new_x-i, screen->new_y, '.');
            }
            break;
        case 3:
            for(i=0; i < 6; i++){
                if(mode == 2)
                    mvwaddch(my_win,screen->new_x, screen->new_y+i, '*');
                else
                mvwaddch(my_win, screen->new_x, screen->new_y+i, '.');
            }
            break;
        case 4:
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

        mvwaddch(my_win, screen->new_x, screen->new_y, screen->ch);
    }

    mvprintw(WINDOW_SIZE + 4 + (screen->ch - 97), 4, "\rLizard %s score: %d.", screen->ch, screen->score);

}

int main(void *ptr, char* IP_n_PORT[])
{	

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

    // Socket to talk to screns
    void *context = zmq_ctx_new();
    void *subscriber = zmq_socket(context, ZMQ_SUB);
    zmq_connect(subscriber, IP_n_PORT);
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, sub_name, strlen(sub_name));

    zmq_msg_t zmq_msg;
    zmq_msg_init(&zmq_msg);
    int msg_len;
    void * msg_data;
    RemoteScreen * screen;

    printf("Connected to server");

    printf("sub_name: %s", sub_name);

    char buffer[20];

    zmq_recv (subscriber, buffer, strlen(buffer), 0);
    printf("buffer: %s", buffer);
    
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
        zmq_recv (subscriber, &screen, sizeof(screen), 0);
        msg_len = zmq_recvmsg(subscriber, &zmq_msg, 0);
        msg_data = zmq_msg_data(&zmq_msg);
        screen = remote_screen__unpack(NULL, msg_len, msg_data);
        
        // process the movement message

        int i = 0;
        
        if(screen->msg_type == 1){ // process lizard movement

            if(screen->score > 49)
                update_window(my_win, screen, 2);
            else
                update_window(my_win, screen, 1);

            box(my_win, 0 , 0);
            char str[40];
            snprintf(str, sizeof(str), "\rLizard %s score: %d.", screen->ch, screen->score);

            mvwaddstr(text_win, screen->ch - 97, 0, str);
            wrefresh(text_win);
            wrefresh(my_win);
        }

        if(screen->msg_type == 2){ // process lizard leave

            wmove(my_win, screen->new_x, screen->new_y);
            waddch(my_win,' ');
            update_window(my_win, screen, 0); // erase leaving lizard
            box(my_win, 0 , 0);
            wmove(text_win, screen->ch - 97, 0);
            wclrtoeol(text_win);
            wrefresh(text_win);
            wrefresh(my_win);

        }

        if(screen->msg_type == 3){ // process roaches movement

            int old_x = 0;
            int old_y = 0;
            int new_x = 0;
            int new_y = 0;
            int v = 0;
            int ID = 0;

            for(i=0; i<10; i++){
                ID = screen->screen_roaches[i][3];
                old_x = roaches[ID][0];
                old_y = roaches[ID][1];
                new_x = screen->screen_roaches[i][0];
                new_y = screen->screen_roaches[i][1];
                v = screen->screen_roaches[i][2];

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

    }
  	endwin();			// End curses mode

	return 0;
}