#include <common.h>
#include "syscall.h"

#define call_return(val) c->GPRx = val;

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;
  Log("[Syscall] Received syscall (id = %d)", a[0]);

  switch (a[0]) {
    case SYS_exit:
      halt(0);
      break;

    case SYS_yield:
      yield();
      call_return(0);
      break;

    case SYS_write:
      if (c->GPR2 == 1 || c->GPR2 == 2) {
        char *buf = (void*)a[2];
        for (int i = 0; i < a[3]; i++) {
          putch(buf[i]);
        }
        call_return(a[3]);
      } else {
        call_return(-1);
      }
      break;

    case SYS_brk:
      call_return(0);
      break;

    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
