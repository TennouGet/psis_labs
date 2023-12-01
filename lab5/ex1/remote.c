#include <stdio.h>
#include <zmq.h>
#include <stdlib.h>

#include <string.h>
#include "remote-char.h"

int main(){
    char line[100];
    char dpt_name[100];
    printf("What is the department of this building? (DEEC, DEI, ...)");
    fgets(line, 100, stdin);
    sscanf(line, "%s", &dpt_name);
    printf("We will broadcast all messages from the president of IST and %s\n", dpt_name);

    // Connect to the server using ZMQ_SUB

    void *context = zmq_ctx_new ();
    void *subscriber = zmq_socket (context, ZMQ_SUB);
    zmq_connect(subscriber, "tcp://localhost:5557");
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, dpt_name, strlen(dpt_name));

    printf("Connected to server");
    
    // subscribe to topics

    struct remote_screen screen;

    char buffer[20];

    zmq_recv (subscriber, buffer, strlen(buffer), 0);
    printf("buffer: %s", buffer);
    
    char message[200];
    while(1){

        // receive messages

        //char *string = s_recv (subscriber);
        zmq_recv (subscriber, &screen, sizeof(screen), 0);

        printf("message from server pox_y=%d\n", screen.pos_y);
        //free(string);
        //free(incoming_dpt);
        
    }

    zmq_close (subscriber);
    zmq_ctx_destroy (context);

}