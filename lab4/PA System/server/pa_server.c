#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include "zhelpers.h"

typedef struct message {

   char dept[50];

   char content[100];

} message;

int main (void){

    void *rep_context = zmq_ctx_new ();
    // Bind a ZMQ_REP socket

    void *responder = zmq_socket (rep_context, ZMQ_REP);
    int rc_rep = zmq_bind (responder, "ipc:///tmp/s1");
    assert (rc_rep == 0);

    char buffer[200], buffer2[200];

    struct message msg;

    // Bind a ZMQ_PUB socket
    
    void *pub_context = zmq_ctx_new ();
    void *publisher = zmq_socket (pub_context, ZMQ_PUB);
    int rc_pub = zmq_bind (publisher, "tcp://*:5556");
    assert(rc_pub == 0);

    strcpy(msg.dept, "DEPT");
    strcpy(msg.content, "HI from server");

    char strn[20] = "SDEEC";

    zmq_send (publisher, strn, strlen (strn), ZMQ_SNDMORE);

    zmq_send (publisher, &msg, sizeof(msg), 0);


    while (1) {

        // receive messages from the microphones

        int n;
        zmq_recv (responder, &msg, sizeof(msg), 0);

        printf("department %s message %s\n", msg.dept, msg.content);

        struct message msg2;

        strcpy(msg2.dept, msg.dept);
        strcpy(msg2.content, msg.content);

        // publish message to speakers

        //s_send (publisher, buffer2);
        zmq_send (publisher, &msg2, sizeof(msg2), 0);

        sleep(1);

    }

    zmq_close (responder);
    zmq_ctx_shutdown (rep_context);
    zmq_close (publisher);
    zmq_ctx_shutdown (pub_context);

    return 0;
}