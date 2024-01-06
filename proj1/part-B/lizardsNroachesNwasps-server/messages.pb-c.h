/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: messages.proto */

#ifndef PROTOBUF_C_messages_2eproto__INCLUDED
#define PROTOBUF_C_messages_2eproto__INCLUDED

#include <protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1000000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1003003 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif


typedef struct _ClientLizardMessage ClientLizardMessage;
typedef struct _ClientRoachesMessage ClientRoachesMessage;
typedef struct _RemoteScreen RemoteScreen;
typedef struct _ResponseToClient ResponseToClient;


/* --- enums --- */

typedef enum _DirectionT {
  DIRECTION_T__UP = 0,
  DIRECTION_T__DOWN = 1,
  DIRECTION_T__LEFT = 2,
  DIRECTION_T__RIGHT = 3
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(DIRECTION_T)
} DirectionT;

/* --- messages --- */

struct  _ClientLizardMessage
{
  ProtobufCMessage base;
  protobuf_c_boolean has_msg_type;
  int32_t msg_type;
  protobuf_c_boolean has_code;
  int32_t code;
  char *ch;
  protobuf_c_boolean has_direction;
  DirectionT direction;
};
#define CLIENT_LIZARD_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&client_lizard_message__descriptor) \
    , 0, 0, 0, 0, NULL, 0, DIRECTION_T__UP }


struct  _ClientRoachesMessage
{
  ProtobufCMessage base;
  protobuf_c_boolean has_msg_type;
  int32_t msg_type;
  protobuf_c_boolean has_code;
  int32_t code;
  protobuf_c_boolean has_n_roaches;
  int32_t n_roaches;
  size_t n_r_scores;
  int32_t *r_scores;
  size_t n_r_direction;
  DirectionT *r_direction;
  size_t n_r_bool;
  int32_t *r_bool;
};
#define CLIENT_ROACHES_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&client_roaches_message__descriptor) \
    , 0, 0, 0, 0, 0, 0, 0,NULL, 0,NULL, 0,NULL }


struct  _RemoteScreen
{
  ProtobufCMessage base;
  protobuf_c_boolean has_msg_type;
  int32_t msg_type;
  char *ch;
  protobuf_c_boolean has_old_x;
  int32_t old_x;
  protobuf_c_boolean has_old_y;
  int32_t old_y;
  protobuf_c_boolean has_new_x;
  int32_t new_x;
  protobuf_c_boolean has_new_y;
  int32_t new_y;
  protobuf_c_boolean has_score;
  int32_t score;
  protobuf_c_boolean has_old_direction;
  DirectionT old_direction;
  protobuf_c_boolean has_new_direction;
  DirectionT new_direction;
  size_t n_screen_roaches;
  int32_t *screen_roaches;
};
#define REMOTE_SCREEN__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&remote_screen__descriptor) \
    , 0, 0, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, DIRECTION_T__UP, 0, DIRECTION_T__UP, 0,NULL }


struct  _ResponseToClient
{
  ProtobufCMessage base;
  protobuf_c_boolean has_status;
  int32_t status;
  protobuf_c_boolean has_code;
  int32_t code;
  char *assigned_char;
  protobuf_c_boolean has_score;
  int32_t score;
};
#define RESPONSE_TO_CLIENT__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&response_to_client__descriptor) \
    , 0, 0, 0, 0, NULL, 0, 0 }


/* ClientLizardMessage methods */
void   client_lizard_message__init
                     (ClientLizardMessage         *message);
size_t client_lizard_message__get_packed_size
                     (const ClientLizardMessage   *message);
size_t client_lizard_message__pack
                     (const ClientLizardMessage   *message,
                      uint8_t             *out);
size_t client_lizard_message__pack_to_buffer
                     (const ClientLizardMessage   *message,
                      ProtobufCBuffer     *buffer);
ClientLizardMessage *
       client_lizard_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   client_lizard_message__free_unpacked
                     (ClientLizardMessage *message,
                      ProtobufCAllocator *allocator);
/* ClientRoachesMessage methods */
void   client_roaches_message__init
                     (ClientRoachesMessage         *message);
size_t client_roaches_message__get_packed_size
                     (const ClientRoachesMessage   *message);
size_t client_roaches_message__pack
                     (const ClientRoachesMessage   *message,
                      uint8_t             *out);
size_t client_roaches_message__pack_to_buffer
                     (const ClientRoachesMessage   *message,
                      ProtobufCBuffer     *buffer);
ClientRoachesMessage *
       client_roaches_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   client_roaches_message__free_unpacked
                     (ClientRoachesMessage *message,
                      ProtobufCAllocator *allocator);
/* RemoteScreen methods */
void   remote_screen__init
                     (RemoteScreen         *message);
size_t remote_screen__get_packed_size
                     (const RemoteScreen   *message);
size_t remote_screen__pack
                     (const RemoteScreen   *message,
                      uint8_t             *out);
size_t remote_screen__pack_to_buffer
                     (const RemoteScreen   *message,
                      ProtobufCBuffer     *buffer);
RemoteScreen *
       remote_screen__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   remote_screen__free_unpacked
                     (RemoteScreen *message,
                      ProtobufCAllocator *allocator);
/* ResponseToClient methods */
void   response_to_client__init
                     (ResponseToClient         *message);
size_t response_to_client__get_packed_size
                     (const ResponseToClient   *message);
size_t response_to_client__pack
                     (const ResponseToClient   *message,
                      uint8_t             *out);
size_t response_to_client__pack_to_buffer
                     (const ResponseToClient   *message,
                      ProtobufCBuffer     *buffer);
ResponseToClient *
       response_to_client__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   response_to_client__free_unpacked
                     (ResponseToClient *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*ClientLizardMessage_Closure)
                 (const ClientLizardMessage *message,
                  void *closure_data);
typedef void (*ClientRoachesMessage_Closure)
                 (const ClientRoachesMessage *message,
                  void *closure_data);
typedef void (*RemoteScreen_Closure)
                 (const RemoteScreen *message,
                  void *closure_data);
typedef void (*ResponseToClient_Closure)
                 (const ResponseToClient *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCEnumDescriptor    direction_t__descriptor;
extern const ProtobufCMessageDescriptor client_lizard_message__descriptor;
extern const ProtobufCMessageDescriptor client_roaches_message__descriptor;
extern const ProtobufCMessageDescriptor remote_screen__descriptor;
extern const ProtobufCMessageDescriptor response_to_client__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_messages_2eproto__INCLUDED */
