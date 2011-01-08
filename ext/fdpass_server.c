#include "fdpass.h"

extern VALUE rb_mFDPass;
extern VALUE rb_eFDPassError;
VALUE rb_cFDPassServer;

static void unlink_socket(struct fdpass_socket *p) {
#ifdef S_ISSOCK
  char *cpath;
  struct stat st;

  if (NIL_P(p->path)) {
    return;
  }

  Check_Type(p->path, T_STRING);
  cpath = RSTRING_PTR(p->path);

  if (stat(cpath, &st) == 0 && S_ISSOCK(st.st_mode)) {
    unlink(cpath);
  }
#endif
}

static void fdpass_server_mark(struct fdpass_socket *p) {
  rb_gc_mark(p->path);
}

static void fdpass_server_free(struct fdpass_socket *p) {
  if (!p->closed) {
    close(p->sock);
    unlink_socket(p);
  }

  xfree(p);
}

static VALUE fdpass_server_alloc(VALUE klass) {
  struct fdpass_socket *p = ALLOC(struct fdpass_socket);
  p->sock = -1;
  p->closed = 1;
  p->path = Qnil;
  return Data_Wrap_Struct(klass, fdpass_server_mark, fdpass_server_free, p);
}

static VALUE rd_fdpass_server_close(VALUE self) {
  struct fdpass_socket *p;

  Data_Get_Struct(self, struct fdpass_socket, p);

  if (!p->closed) {
    close(p->sock);
    unlink_socket(p);
    p->sock = -1;
    p->closed = 1;
    p->path = Qnil;
  }

  return Qnil;
}

static VALUE rd_fdpass_server_is_closed(VALUE self) {
  struct fdpass_socket *p;
  Data_Get_Struct(self, struct fdpass_socket, p);
  return p->closed ? Qtrue : Qfalse;
}

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

  return Qnil;
}

static VALUE rd_fdpass_server_fd_close(VALUE self) {
  int ifd;

  ifd = NUM2INT(self);
  close(ifd);

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
  VALUE fd;

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

  fd = INT2NUM(*cmsg_data);
  rb_define_singleton_method(fd, "close", rd_fdpass_server_fd_close, 0);

  return fd;
}

void Init_fdpass_server() {
  rb_cFDPassServer = rb_define_class_under(rb_mFDPass, "Server", rb_cObject);
  rb_define_alloc_func(rb_cFDPassServer, fdpass_server_alloc);
  rb_define_private_method(rb_cFDPassServer, "initialize", rd_fdpass_server_initialize, 1);
  rb_define_method(rb_cFDPassServer, "close", rd_fdpass_server_close, 0);
  rb_define_method(rb_cFDPassServer, "closed?", rd_fdpass_server_is_closed, 0);
  rb_define_method(rb_cFDPassServer, "recv", rb_fdpass_server_recv, 0);
}
