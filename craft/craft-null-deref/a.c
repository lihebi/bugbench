#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
typedef struct _demo {
  int id;
  struct _demo *f;
} demo;

bool b;
bool c;

demo a;
void bar(demo *z, demo *x) {
  demo *p = z->f;
  demo *y;
  int d;
  if (c) {
    d = 0;
  } else {
    d = 1;
  }
  if (b) {
    y = z;
  } else {
    y = x->f;
  }
  // @HeliumStmt
  *y = a;
}

demo *z;
demo *x;
int main(int argc, char* argv[]) {
  bar(z, x);
}

