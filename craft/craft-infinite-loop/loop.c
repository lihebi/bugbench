#include <stdio.h>
#include <string.h>

/*******************************
 ** Infinite loop
 ** Adapted from CVE-2004-0940 in Apache benchmark
 *******************************/

char nondet_char() {
  const char buf[] =
    "nKrQrE2CPOOEsBFwZhfDHWNXjr0AR9NFJlmX4hV7Go4IH26RNnLhAvmRZ277KWGLPc8belgRlhx8cYSWczFlarel0TkDCbR29m8myW7Bx8yHAkiI8gAbVCBUs8MywVW7MTdr3Oqd3r9tmY3TivMQcn7mNcBsd6mK";
  static int idx = -1;
  static int total = 0;
  total++;
  idx++;

  // the first should be a quote, and no other quote
  if (idx == 0) {
    return '"';
  }

  // the last one should be "\"
  if (total == BUFSIZ) return '\\';

  // recursive the given string
  if (idx == (int)strlen(buf)) idx = 0;
  return buf[idx];
}

int ap_isspace(char c)
{
  if (c == '\t'
      || c == '\n'
      || c == '\v'
      || c == '\f'
      || c == '\r'
      || c == ' ')
    return 1;

  return 0;
}

int ap_tolower(char c)
{
  /* do we have tolower() in our stubs? */
  return c;
}


char *get_tag(char *tag, int tagbuf_len)
{
  char *tag_val, c, term;
  char *t;

  t = tag;

  --tagbuf_len;

  /* do { */
  /*   c = nondet_char(); */
  /* } while (ap_isspace(c)); */

  /* if (c != '"' && c != '\'') { */
  /*   return NULL; */
  /* } */

  term = c;
  // precondition: buf[tagbuf_len = '\']
  // @HeliumLoop
  while (1) {
    // loop invariant: t != tag + tagbuf_len
    c = nondet_char();
    if (t == tag + tagbuf_len) { /* Suppose t == tag + tagbuf_len - 1 */
      *t = 0;
      return NULL;
    }  

    if (c == '\\') {
      // invariant: t = tag + tagbuf_len
      
      /* BAD */
      *t = c;
      t++;               /* Now t == tag + tagbuf_len */
      c = nondet_char();
    }
    else if (c == term) {
      break;
    }

    /* BAD */
    /* *t = c; */
    t++;                /* Now t == tag + tagbuf_len + 1 
                         * So the bounds check (t == tag + tagbuf_len) will fail */
  }
  /* BAD */
  *t = 0;

  return tag;
}

int main ()
{
  char tag[10];
  // precondition: None
  // Invariants: t != tag + tagbuf_len
  // transfer function: O_tagbuf_len = I_BUFSIZ
  /* The caller always passes in (tag, sizeof(tag)) */
  get_tag (tag, 10);

  return 0;
}
