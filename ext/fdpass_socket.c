#include "fdpass_socket.h"

extern VALUE rb_eFDPassError;

static void unlink_socket(struct fdpass_socket *p) {
#ifdef S_ISSOCK
  char *cpath;
  struct stat st;

  if (!p->is_server || NIL_P(p->path)) {
    return;
  }

  Check_Type(p->path, T_STRING);
  cpath = RSTRING_PTR(p->path);

  if (stat(cpath, &st) == 0 && S_ISSOCK(st.st_mode)) {
    unlink(cpath);
  }
#endif
}

static void fdpass_socket_mark(struct fdpass_socket *p) {
  rb_gc_mark(p->path);
}

static void fdpass_socket_free(struct fdpass_socket *p) {
  if (!p->closed) {
    close(p->sock);
    unlink_socket(p);
  }

  xfree(p);
}

VALUE fdpass_socket_alloc(VALUE klass) {
  struct fdpass_socket *p = ALLOC(struct fdpass_socket);
  p->sock = -1;
  p->closed = 1;
  p->path = Qnil;
  return Data_Wrap_Struct(klass, fdpass_socket_mark, fdpass_socket_free, p);
}

VALUE rd_fdpass_socket_close(VALUE self) {
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

VALUE rd_fdpass_socket_is_closed(VALUE self) {
  struct fdpass_socket *p;
  Data_Get_Struct(self, struct fdpass_socket, p);
  return p->closed ? Qtrue : Qfalse;
}
