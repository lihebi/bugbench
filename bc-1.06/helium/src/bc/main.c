/* main.c: The main program for bc.  */

/*  This file is part of GNU bc.
    Copyright (C) 1991-1994, 1997, 1998, 2000 Free Software Foundation, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License , or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING.  If not, write to
      The Free Software Foundation, Inc.
      59 Temple Place, Suite 330
      Boston, MA 02111 USA

    You may contact the author by:
       e-mail:  philnelson@acm.org
      us-mail:  Philip A. Nelson
                Computer Science Department, 9062
                Western Washington University
                Bellingham, WA 98226-9062
       
*************************************************************************/

#include "bcdefs.h"
#include <signal.h>
#include "global.h"
#include "proto.h"
#include "getopt.h"


/* Variables for processing multiple files. */
static char first_file;

/* Points to the last node in the file name list for easy adding. */
static file_node *last = NULL;

/* long option support */
static struct option long_options[] =
{
  {"compile",  0, &compile_only, TRUE},
  {"help",     0, 0,             'h'},
  {"interactive", 0, 0,          'i'},
  {"mathlib",  0, &use_math,     TRUE},
  {"quiet",    0, &quiet,        TRUE},
  {"standard", 0, &std_only,     TRUE},
  {"version",  0, 0,             'v'},
  {"warn",     0, &warn_not_std, TRUE},

  {0, 0, 0, 0}
};



void
parse_args (argc, argv)
     int argc;
     char **argv;
{
  int optch;
  int long_index;
  file_node *temp;

  /* Force getopt to initialize.  Depends on GNU getopt. */
  optind = 0;

  /* Parse the command line */
  while (1)
    {
      optch = getopt_long (argc, argv, "chilqswv", long_options, &long_index);

      if (optch == EOF)  /* End of arguments. */
	break;

    }

  /* Add file names to a list of files to process. */
  while (optind < argc)
    {
      temp = (file_node *) bc_malloc(sizeof(file_node));
      temp->name = argv[optind];
      temp->next = NULL;
      if (last == NULL)
	file_names = temp;
      else
	last->next = temp;
      last = temp;
      optind++;
    }
}


/* The main program for bc. */
int
main (argc, argv)
     int argc;
     char *argv[];
{
  char *env_value;
 
  /* Command line arguments. */
  parse_args (argc, argv);


  env_value = getenv ("BC_LINE_LENGTH");
  if (env_value != NULL)
    {
      line_size = atoi (env_value);
      if (line_size < 2)
	line_size = 70;
    }
  else
    line_size = 70;

  /* Initialize the machine.  */
  init_storage();
  init_load();

  /* Initialize the front end. */
  init_tree();
  init_gen ();
  is_std_in = FALSE;
  first_file = TRUE;
  if (!open_new_file ())
    exit (1);


  /* Do the parse. */
  yyparse ();


  exit (0);
}

void game_over()
{
}


/* This is the function that opens all the files. 
   It returns TRUE if the file was opened, otherwise
   it returns FALSE. */

int
open_new_file ()
{
  FILE *new_file;
  file_node *temp;

  /* Set the line number. */
  line_no = 1;

  /* Check to see if we are done. */
  if (is_std_in) return (FALSE);
  
  /* One of the argv values. */
  if (file_names != NULL)
    {
      new_file = fopen (file_names->name, "r");
      if (new_file != NULL)
	{
	  new_yy_file (new_file);
	  temp = file_names;
	  file_name  = temp->name;
	  file_names = temp->next;
	  free (temp);
	  return TRUE;
	}
    }
  return TRUE;
}


/* Set yyin to the new file. */

void
new_yy_file (file)
     FILE *file;
{
  if (!first_file) fclose (yyin);
  yyin = file;
  first_file = FALSE;
}


/* Message to use quit.  */

void
use_quit (sig)
     int sig;
{
  printf ("\n(interrupt) use quit to exit.\n");
  signal (SIGINT, use_quit);
}

