#include "fdpass.h"

extern VALUE rb_mFDPass;
extern VALUE rb_eFDPassError;
VALUE rb_cFDPassClient;

static void fdpass_client_mark(struct fdpass_socket *p) {
  rb_gc_mark(p->path);
}

static void fdpass_client_free(struct fdpass_socket *p) {
  if (!p->closed) {
    close(p->sock);
  }

  xfree(p);
}

static VALUE fdpass_client_alloc(VALUE klass) {
  struct fdpass_socket *p = ALLOC(struct fdpass_socket);
  p->sock = -1;
  p->closed = 1;
  p->path = Qnil;
  return Data_Wrap_Struct(klass, fdpass_client_mark, fdpass_client_free, p);
}

static VALUE rd_fdpass_client_close(VALUE self) {
  struct fdpass_socket *p;

  Data_Get_Struct(self, struct fdpass_socket, p);

  if (!p->closed) {
    close(p->sock);
    p->sock = -1;
    p->closed = 1;
    p->path = Qnil;
  }

  return Qnil;
}

static VALUE rd_fdpass_client_is_closed(VALUE self) {
  struct fdpass_socket *p;
  Data_Get_Struct(self, struct fdpass_socket, p);
  return p->closed ? Qtrue : Qfalse;
}

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
  rb_define_alloc_func(rb_cFDPassClient, fdpass_client_alloc);
  rb_define_private_method(rb_cFDPassClient, "initialize", rd_fdpass_client_initialize, 1);
  rb_define_method(rb_cFDPassClient, "close", rd_fdpass_client_close, 0);
  rb_define_method(rb_cFDPassClient, "closed?", rd_fdpass_client_is_closed, 0);
  rb_define_method(rb_cFDPassClient, "send", rb_fdpass_client_send, 1);
}
