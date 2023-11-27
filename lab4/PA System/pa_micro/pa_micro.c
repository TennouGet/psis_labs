#include <stdio.h>
#include <zmq.h>
#include "zhelpers.h"

typedef struct message {

   char dept[50];

   char content[100];

} message;

int main(){
    char line[100];
    char dpt_name[100];
    char buffer[200];
    printf("What is your department? (DEEC, DEI, ...)");
    fgets(line, 100, stdin);
    sscanf(line, "%s", &dpt_name);
    printf("Hello your Honor, the President of %s\n", dpt_name);

    struct message msg;

    strcpy(msg.dept, dpt_name);

    void *context = zmq_ctx_new ();
    // Connect to the server using ZMQ_REQ
    void *requester = zmq_socket (context, ZMQ_REQ);
    zmq_connect (requester, "ipc:///tmp/s1");
    
    char message[100];
    while(1){

        printf("Please write the message to the students and staff on your buildings! ");
        fgets(message, 100, stdin);

        //send message to server

        strcpy(msg.content, message);

        zmq_send (requester, &msg, sizeof(msg), 0);

        printf("Forwarding this message to all: %s", message);
        
    }

    zmq_close (requester);
    zmq_ctx_destroy (context);
    return 0;
}