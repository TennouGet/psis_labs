#include <ncurses.h>
#include "header.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <ctype.h> 
#include <stdlib.h>
#include <string.h>

#include <math.h>

#include <zmq.h>
#include <pthread.h>
#include "messages.pb-c.h"

char PUB_PORT[20] = "tcp://localhost:5557";

// display thread and functions

void update_window(WINDOW * my_win, RemoteScreen * screen, int mode){

    int i = 0;

    if(mode==0 || mode==1 || mode==2){ //delete lizard at previous position

        mvwaddch(my_win, screen->old_x, screen->old_y, ' ');

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

    if(mode==1 || mode==2){ //add lizard at new position
        
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

        mvwaddch(my_win, screen->new_x, screen->new_y, *screen->ch);
    }

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

    // Socket to talk to screns
    void *context = zmq_ctx_new();
    void *subscriber = zmq_socket(context, ZMQ_SUB);
    zmq_connect(subscriber, PUB_PORT);
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
        //zmq_recv (subscriber, &screen, sizeof(screen), 0);
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

            mvwaddstr(text_win, *screen->ch - 97, 0, str);
            wrefresh(text_win);
            wrefresh(my_win);
        }

        if(screen->msg_type == 2){ // process lizard leave

            wmove(my_win, screen->new_x, screen->new_y);
            waddch(my_win,' ');
            update_window(my_win, screen, 0); // erase leaving lizard
            box(my_win, 0 , 0);
            wmove(text_win, *screen->ch - 97, 0);
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
                ID = screen->screen_roaches[i*10 + 3];
                old_x = roaches[ID][0];
                old_y = roaches[ID][1];
                new_x = screen->screen_roaches[i*10 + 0];
                new_y = screen->screen_roaches[i*10 + 1];
                v = screen->screen_roaches[i*10 + 2];

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
}


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

    // Spawn display thread
    //pthread_t display;
    //pthread_create( &display, NULL, thread_display, (void *) PUB_PORT);

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