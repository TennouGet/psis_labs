#include <stdio.h>
#include <zmq.h>
#include <stdlib.h>
#include "zhelpers.h"

typedef struct message {

   char dept[50];

   char content[100];

} message;

int main(){
    char line[100];
    char dpt_name[100];
    printf("What is the department of this building? (DEEC, DEI, ...)");
    fgets(line, 100, stdin);
    sscanf(line, "%s", &dpt_name);
    printf("We will broadcast all messages from the president of IST and %s\n", dpt_name);

    struct message msg;

    // Connect to the server using ZMQ_SUB

    void *context = zmq_ctx_new ();
    void *subscriber = zmq_socket (context, ZMQ_SUB);
    zmq_connect(subscriber, "tcp://localhost:5556");
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE,"odd", 3);

    printf("Connected to server");
    
    // subscribe to topics

    char strn[20];

    zmq_recv (subscriber, strn, strlen(strn), 0);

    
    char message[200];
    while(1){

        // receive messages

        //char *string = s_recv (subscriber);
        zmq_recv (subscriber, &msg, sizeof(msg), 0);

        printf("message from  %s - %s", msg.dept, msg.content);
        //free(string);
        //free(incoming_dpt);
        
    }

    zmq_close (subscriber);
    zmq_ctx_destroy (context);

}