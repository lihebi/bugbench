#include <stdio.h>
#include <stdlib.h>

#include <stdbool.h>

/*******************************
 ** Double Free
 ** Adapted from cvs-1.11.4 in bugbench
 *******************************/

char *global_pointer;
void bar() {
  if (global_pointer != NULL) {
    // @HeliumStmt
    free (global_pointer);
  }
}

int main(int argc, char *argv[]) {
  bool b = false;
  // pre-condition:
  // transfer: &global_pointer_O == &global_pointer_I && global_pointer is in free-d list
  global_pointer = (char*)malloc(BUFSIZ * sizeof(char));
  // pre: global_pointer!=NULL && not in free-d list, argc >= 3
  // transfer: &global_pointer_O == &global_pointer_I && global_pointer is in free-d list
  // invariant: &global_pointer_O in free-d list
  for (int i=0;i<argc;i++) {
    if (i >= 1) {
      b = true;
    } else {
      b = false;
    }
    if (b) {
      bar();
    }
  }
}
