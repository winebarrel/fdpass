#include "fdpass.h"

VALUE rb_mFDPass;
VALUE rb_eFDPassError;
extern VALUE rb_cFDPassServer;
extern VALUE rb_cFDPassClient;

static VALUE rb_fdpass_server(VALUE self, VALUE path) {
  return rb_funcall(rb_cFDPassServer, rb_intern("new"), 1, path);
}

static VALUE rb_fdpass_client(VALUE self, VALUE path) {
  return rb_funcall(rb_cFDPassClient, rb_intern("new"), 1, path);
}

void Init_fdpass() {
  rb_mFDPass = rb_define_module("FDPass");
  rb_define_module_function(rb_mFDPass, "server", rb_fdpass_server, 1);
  rb_define_module_function(rb_mFDPass, "client", rb_fdpass_client, 1);

  rb_eFDPassError = rb_define_class_under(rb_mFDPass, "Error", rb_eStandardError);

  Init_fdpass_server();
  Init_fdpass_client();
}
