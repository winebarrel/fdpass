#include "fdpass.h"

extern VALUE rb_mFDPass;
VALUE rb_cFDPassFD;

static void fdpass_fd_free(struct fdpass_fd *p) {
  if (!p->closed && p->fd >= 0) {
    close(p->fd);
  }

  xfree(p);
}

static VALUE fdpass_fd_alloc(VALUE klass) {
  struct fdpass_fd *p = ALLOC(struct fdpass_fd);
  p->fd = -1;
  p->closed = 1;
  return Data_Wrap_Struct(klass, 0, fdpass_fd_free, p);
}

static VALUE rd_fdpass_fd_close(VALUE self) {
  struct fdpass_fd *p;

  Data_Get_Struct(self, struct fdpass_fd, p);

  if (!p->closed && p->fd >= 0) {
    close(p->fd);
    p->fd = -1;
    p->closed = 1;
  }

  return Qnil;
}

static VALUE rd_fdpass_fd_is_closed(VALUE self) {
  struct fdpass_fd *p;
  Data_Get_Struct(self, struct fdpass_fd, p);
  return p->closed ? Qtrue : Qfalse;
}

static VALUE rd_fdpass_fd_to_int(VALUE self) {
  struct fdpass_fd *p;
  Data_Get_Struct(self, struct fdpass_fd, p);
  return INT2FIX(p->fd);
}

static VALUE rd_fdpass_fd_to_s(VALUE self) {
  struct fdpass_fd *p;
  VALUE str;
  size_t len;
  const char *cname;
  char *cstr;

  Data_Get_Struct(self, struct fdpass_fd, p);

  cname = rb_obj_classname(self);
  len = strlen(cname) + 6 + 16 + 25;
  str = rb_str_new(0, len - 1);
  cstr = RSTRING_PTR(str);
  snprintf(cstr, len, "#<%s:0x%lx @fd=%d>", cname, self, p->fd);
  RSTRING_LEN(str) = strlen(cstr);

  return str;
}

static VALUE rd_fdpass_fd_initialize(VALUE self, VALUE fd) {
  struct fdpass_fd *p;

  Data_Get_Struct(self, struct fdpass_fd, p);

  p->fd = NUM2INT(fd);
  p->closed = 0;

  return Qnil;
}

void Init_fdpass_fd() {
  rb_cFDPassFD = rb_define_class_under(rb_mFDPass, "FD", rb_cObject);
  rb_define_alloc_func(rb_cFDPassFD, fdpass_fd_alloc);
  rb_define_private_method(rb_cFDPassFD, "initialize", rd_fdpass_fd_initialize, 1);
  rb_define_method(rb_cFDPassFD, "close", rd_fdpass_fd_close, 0);
  rb_define_method(rb_cFDPassFD, "closed?", rd_fdpass_fd_is_closed, 0);
  rb_define_method(rb_cFDPassFD, "to_int", rd_fdpass_fd_to_int, 0);
  rb_define_method(rb_cFDPassFD, "to_i", rd_fdpass_fd_to_int, 0);
  rb_define_method(rb_cFDPassFD, "to_s", rd_fdpass_fd_to_s, 0);
}
