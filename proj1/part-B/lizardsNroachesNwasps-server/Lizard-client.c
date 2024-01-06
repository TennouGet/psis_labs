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
	//initscr();
	//cbreak();
    //keypad(stdscr, TRUE);
	//noecho();


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

            if(screen2->score > 49)
                update_window(my_win, screen2, 2);
            else
                update_window(my_win, screen2, 1);

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
                ID = screen2->screen_roaches[i*10 + 3];
                old_x = roaches[ID][0];
                old_y = roaches[ID][1];
                new_x = screen2->screen_roaches[i*10 + 0];
                new_y = screen2->screen_roaches[i*10 + 1];
                v = screen2->screen_roaches[i*10 + 2];

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

        //zmq_msg_close(msg_data);
        //free(screen);
        //zmq_msg_close (&zmq_msg);
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

    // ncurses initialization
	initscr();
	cbreak();
    keypad(stdscr, TRUE);
	noecho();

    // Spawn display thread
    pthread_t display;
    pthread_create( &display, NULL, thread_display, (void *) PUB_PORT);

    ClientLizardMessage join = CLIENT_LIZARD_MESSAGE__INIT;
    ClientLizardMessage move = CLIENT_LIZARD_MESSAGE__INIT;
    ClientLizardMessage leave = CLIENT_LIZARD_MESSAGE__INIT;
    ResponseToClient * client_response;

    zmq_msg_t zmq_msg;
    zmq_msg_init(&zmq_msg);

    join.has_msg_type = 1;
    join.has_code = 1;
    join.has_direction = 1;

    move.has_msg_type = 1;
    move.has_code = 1;
    move.has_direction = 1;
    
    leave.has_msg_type = 1;
    leave.has_code = 1;
    leave.has_direction = 1;

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

    int ch;
    bool exit_program = false;
    bool valid_ch = false;

    int n = 0;
    do
    {
    	ch = getch();	
        n++;
        clear();
        switch (ch)
        {
            case KEY_LEFT:
                move.direction = 2;
                valid_ch = true;
                break;
            case KEY_RIGHT:
                move.direction = 3;
                valid_ch = true;
                break;
            case KEY_DOWN:
                move.direction = 1;
                valid_ch = true;
                break;
            case KEY_UP:
                move.direction = 0;
                valid_ch = true;
                break;
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

            //send byebye message to server
            leave.msg_type = 2;
            leave.code = move.code;
            leave.ch = move.ch;

            msg_len = client_lizard_message__get_packed_size(&leave);
            msg_buf = malloc(msg_len);
            client_lizard_message__pack(&leave, msg_buf);
            zmq_send (requester, msg_buf, msg_len, 0);
            free(msg_buf);

            msg_len = zmq_recvmsg(requester, &zmq_msg, 0); 
            msg_data = zmq_msg_data(&zmq_msg);
            client_response = response_to_client__unpack(NULL, msg_len, msg_data);

            if(client_response->status == 2){
                printf("exiting..\n");
                zmq_close (requester);
                zmq_ctx_destroy (context);
                printf("Disconnected from server.\n");
                break;
            }
            else{
                printf("error while trying to exit");
            }
        }

        if(valid_ch==true){

            //send the movement message
            msg_len = client_lizard_message__get_packed_size(&move);
            msg_buf = malloc(msg_len);
            client_lizard_message__pack(&move, msg_buf);
            zmq_send (requester, msg_buf, msg_len, 0);
            free(msg_buf);

            msg_len = zmq_recvmsg(requester, &zmq_msg, 0); 
            msg_data = zmq_msg_data(&zmq_msg);
            client_response = response_to_client__unpack(NULL, msg_len, msg_data);

            if(client_response->status == 1)
                printf("\rServer status: OK");
            if(client_response->status == -1 || client_response->status == -2){
                zmq_close (requester);
                zmq_ctx_destroy (context);
                if(client_response->status == -1)
                    printf("\rError -1: server full, try again.");
                if(client_response->status == -2)
                    printf("Error -2: no empty space to deploy lizard, try again.\n");
                return 0;
            }
            if(client_response->status != 1 && client_response->status != -1 && client_response->status != -2)
                printf("\rError, server response: %d", client_response->status);

        }

        
    }while(ch != 27);

    zmq_close (requester);
    zmq_ctx_destroy (context);

	return 0;
}