#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int sprintf_parsed(char *out, const char *fmt, va_list *args);

int printf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char s[100];
  sprintf_parsed(s, fmt, &args);
  va_end(args);
  char *p = s;
  while (*p) {
    putch(*p);
    p++;
  }
  return 0;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  return 0;
}

static struct {
  int lpad;
} pref;

void sprint_basic_format(char** pout, char** pin, va_list* args) {
  if (**pin == 's') {
    char *p = va_arg(*args, char*);
    while (*p) *(*pout)++ = *p++;
  } else if (**pin == 'd') {
    int val = va_arg(*args, int);
    int f = 1;
    if (val < 0) {
      *(*pout)++ = '-';
      f = -1;
    }
    int buf[24] = {0};
    int i = 0;
    for (; i < 10 && val; i++) {
      buf[i] = (val % 10) * f;
      val /= 10;
    }
    if (i < pref.lpad) i = pref.lpad;
    if (i == 0) i++;
    for (i--; i >= 0; i--) {
      *(*pout)++ = buf[i] + '0';
    }
  } else if (**pin == 'u') {
    unsigned int val = va_arg(*args, unsigned int);
    int buf[24] = {0};
    int i = 0;
    for (; i < 10 && val; i++) {
      buf[i] = (val % 10);
      val /= 10;
    }
    if (i < pref.lpad) i = pref.lpad;
    if (i == 0) i++;
    for (i--; i >= 0; i--) {
      *(*pout)++ = buf[i] + '0';
    }
  } else if (**pin == 'x') {
    const char hex_char[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    int val = va_arg(*args, int);
    int buf[24] = {0};
    int i = 0;
    for (; i < 10 && val; i++) {
      buf[i] = (val % 16);
      val /= 16;
    }
    if (i < pref.lpad) i = pref.lpad;
    if (i == 0) i++;
    for (i--; i >= 0; i--) {
      *(*pout)++ = hex_char[buf[i]];
    }
  } else {
    putch(**pin);
    assert(false);
  }
  (*pin)++;
}

int sprint_read_pad(char** pout, char** pin, va_list* args) {
  int sum = **pin - '0';
  if (sum < '0' || sum > '9') return 0;
  (*pin)++;
  // int ans = sprint_read_pad(pout, pin, args);
  // if (ans) return sum * 10 + ans;
  return sum;
}

void sprint_format(char** pout, char** pin, va_list* args) {
  switch (**pin) {
    // To be implemented
    case '%':
      *(*pout)++ = '%';
      break;

    case '0':
      (*pin)++;
      pref.lpad = sprint_read_pad(pout, pin, args);
      sprint_format(pout, pin, args);
      break;

    case 'l':
      (*pin)++;
      sprint_format(pout, pin, args);
      break;

    default:
      sprint_basic_format(pout, pin, args);
  }
}

int sprintf_parsed(char *out, const char *fmt, va_list *args) {
  pref.lpad = 0;

  char *pout = out;
  char *pin = (void*)fmt;
  while (*pin) {
    switch (*pin) {
      case '%':
        pin++;
        sprint_format(&pout, &pin, args);
      default:
        *pout = *pin;
        pin++;
        pout++;
    }
  }
  
  *pout = 0;
  return pout - out;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int len = sprintf_parsed(out, fmt, &args);
  va_end(args);
  return len;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  return 0;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  return 0;
}

#endif
