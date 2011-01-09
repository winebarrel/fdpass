#ifndef __FDPASS_H__
#define __FDPASS_C__

#include <string.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <errno.h>

#include <ruby.h>

#ifndef RSTRING_PTR
#define RSTRING_PTR(s) (RSTRING(s)->ptr)
#endif
#ifndef RSTRING_LEN
#define RSTRING_LEN(s) (RSTRING(s)->len)
#endif

struct fdpass_socket {
  int sock;
  int closed;
  VALUE path;
};

struct fdpass_fd {
  int fd;
  int closed;
};

#define Check_Socket(p) do { \
  if ((p)->sock < 0 || (p)->closed || NIL_P((p)->path)) { \
    rb_raise(rb_eFDPassError, "Invalid socket"); \
  } \
  Check_Type((p)->path, T_STRING); \
} while(0)

void Init_fdpass_server();
void Init_fdpass_client();
void Init_fdpass_fd();

#endif // __FDPASS_H__
