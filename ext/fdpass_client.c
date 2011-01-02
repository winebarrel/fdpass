#include "fdpass.h"
#include "fdpass_socket.h"

extern VALUE rb_mFDPass;
extern VALUE rb_eFDPassError;
VALUE rb_cFDPassClient;

static VALUE rd_fdpass_client_initialize(VALUE self, VALUE path) {
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

  if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
    rb_raise(rb_eFDPassError, "%s", strerror(errno));
  }

  p->sock = sock;
  p->closed = 0;
  p->path = path;
  p->is_server = 0;

  return Qnil;
}

static VALUE rb_fdpass_client_send(VALUE self, VALUE fd) {
  struct fdpass_socket *p;
  struct msghdr msg = { 0 };
  struct cmsghdr *cmsg;
  char cmsgbuf[CMSG_SPACE(sizeof(int))];
  int ifd, *cmsg_data;
  struct iovec iov;

  ifd = NUM2INT(fd);

  Data_Get_Struct(self, struct fdpass_socket, p);
  Check_Socket(p);

  iov.iov_base = "\0";
  iov.iov_len = 1;

  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = cmsgbuf;
  msg.msg_controllen = sizeof(cmsgbuf);

  cmsg = CMSG_FIRSTHDR(&msg);
  cmsg->cmsg_len = CMSG_LEN(sizeof(int));
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type = SCM_RIGHTS;

  cmsg_data = (int *) CMSG_DATA(cmsg);
  *cmsg_data = ifd;
  msg.msg_controllen = cmsg->cmsg_len;
  
  if (sendmsg(p->sock, &msg, 0) < 0) { 
    rb_raise(rb_eFDPassError, "%s", strerror(errno));
  }

  return Qnil;
}

void Init_fdpass_client() {
  rb_cFDPassClient = rb_define_class_under(rb_mFDPass, "Client", rb_cObject);
  rb_define_alloc_func(rb_cFDPassClient, fdpass_socket_alloc);
  rb_define_private_method(rb_cFDPassClient, "initialize", rd_fdpass_client_initialize, 1);
  rb_define_method(rb_cFDPassClient, "close", rd_fdpass_socket_close, 0);
  rb_define_method(rb_cFDPassClient, "closed?", rd_fdpass_socket_is_closed, 0);
  rb_define_method(rb_cFDPassClient, "send", rb_fdpass_client_send, 1);
}