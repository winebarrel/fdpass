#ifndef __FDPASS_SOCKET_H__
#define __FDPASS_SOCKET_C__

#include "fdpass.h"

struct fdpass_socket {
  int sock;
  int closed;
  VALUE path;
  int is_server;
};

#define Check_Socket(p) do { \
  if ((p)->sock < 0 || (p)->closed || NIL_P((p)->path)) { \
    rb_raise(rb_eFDPassError, "Invalid socket"); \
  } \
  Check_Type((p)->path, T_STRING); \
} while(0)

VALUE fdpass_socket_alloc(VALUE klass);
VALUE rd_fdpass_socket_close(VALUE self);
VALUE rd_fdpass_socket_is_closed(VALUE self);

#endif // __FDPASS_SOCKET_H__
