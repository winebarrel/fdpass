#include "fdpass.h"
#include "fdpass_socket.h"

extern VALUE rb_mFDPass;
extern VALUE rb_eFDPassError;
VALUE rb_cFDPassServer;

static VALUE rd_fdpass_server_initialize(VALUE self, VALUE path) {
  struct fdpass_socket *p;
  char *cpath;
  int sock;
  struct sockaddr_un addr;

  Data_Get_Struct(self, struct fdpass_socket, p);

  Check_Type(path, T_STRING);
  cpath = RSTRING_PTR(path);

  sock = socket(PF_UNIX, SOCK_DGRAM, 0);

  if (sock < 0) {
    rb_raise(rb_eFDPassError, "%s", strerror(errno));
  }

  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, cpath, sizeof(addr.sun_path) - 1);
  addr.sun_path[sizeof(addr.sun_path) - 1] = '\0';

  if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
    rb_raise(rb_eFDPassError, "%s", strerror(errno));
  }

  p->sock = sock;
  p->closed = 0;
  p->path = path;
  p->is_server = 1;

  return Qnil;
}

static VALUE rb_fdpass_server_recv(VALUE self) {
  struct fdpass_socket *p;
  struct msghdr msg = { 0 };
  struct cmsghdr *cmsg;
  char cmsgbuf[CMSG_SPACE(sizeof(int))];
  int *cmsg_data;
  struct iovec iov;
  char iov_data[1];

  Data_Get_Struct(self, struct fdpass_socket, p);
  Check_Socket(p);

  iov.iov_base = iov_data;
  iov.iov_len = sizeof(iov_data);

  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = cmsgbuf;
  msg.msg_controllen = sizeof(cmsgbuf);

  if (recvmsg(p->sock, &msg, MSG_WAITALL) < 0) { 
    rb_raise(rb_eFDPassError, "%s", strerror(errno));
  }

  cmsg = CMSG_FIRSTHDR(&msg);
  cmsg_data = (int *) CMSG_DATA(cmsg);

  return INT2NUM(*cmsg_data);
}

void Init_fdpass_server() {
  rb_cFDPassServer = rb_define_class_under(rb_mFDPass, "Server", rb_cObject);
  rb_define_alloc_func(rb_cFDPassServer, fdpass_socket_alloc);
  rb_define_private_method(rb_cFDPassServer, "initialize", rd_fdpass_server_initialize, 1);
  rb_define_method(rb_cFDPassServer, "close", rd_fdpass_socket_close, 0);
  rb_define_method(rb_cFDPassServer, "closed?", rd_fdpass_socket_is_closed, 0);
  rb_define_method(rb_cFDPassServer, "recv", rb_fdpass_server_recv, 0);
}