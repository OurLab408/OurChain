#include <ourcontract.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

struct stateBuf {
  long mtype;
  char buf[1024];
};

int contract_main(int argc, char **argv) {
  err_printf("start contract\n");
  struct stateBuf buf;
  if (state_read(&buf, sizeof(struct stateBuf)) == -1) {
    err_printf("read state error\n");
  };
  err_printf("get state %s\n", buf.buf);
  buf.mtype = 1;
  strcpy(buf.buf, "Hello World!");
  if (state_write(&buf, sizeof(struct stateBuf)) == -1) {
    err_printf("send state error\n");
  };
  return 0;
}