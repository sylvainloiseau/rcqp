/*
 *  IMS Open Corpus Workbench (CWB)
 *  Copyright (C) 1993-2006 by IMS, University of Stuttgart
 *  Copyright (C) 2007-     by the respective contributers (see file AUTHORS)
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2, or (at your option) any later
 *  version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 *  Public License for more details (in the file "COPYING", or available via
 *  WWW at http://www.gnu.org/copyleft/gpl.html).
 */


#include <math.h>

#include "../cl/globals.h"
#include "../cl/cl.h"
#include "../cl/corpus.h"
#include "../cl/attributes.h"
#include "../cl/macros.h"

/** String set to the name of this program. */
char *progname;

int print_nr = 0;           /**< boolean: flag whether we should print line numbers */
int print_freqs = 0;        /**< boolean: print the frequencies of the words? */
int print_len = 0;          /**< boolean: print the word length s? */
int sort = 0;               /**< boolean: print the lexicon in a sorted order? */
int input_are_numbers = 0;  /**< boolean: read lexicon IDs from file? */
int show_size_only = 0;     /**< boolean: do_show should just print the size of the lexicon and exit? */
int freq_0_if_unknown = 0;  /**< boolean: print out unknown words with freq 0? */

Corpus *corpus = NULL;
char *corpus_id = NULL;
char *input_filename = NULL;

/* ---------------------------------------------------------------------- */

/**
 * Prints information about a specified item on a P-attribute
 *
 * @param attr        The attribute to search
 * @param id          The id number of the item (type) in question
 * @param fallback_s  String to print if the item is not found
 *                    (use NULL to use a default fallback string)
 */
void
lexdecode_print_item_info(Attribute *attr, int id, char *fallback_s)
{
  char *item;
  int freq, slen;

  if (id >= 0) {
    if (fallback_s == NULL)
      fallback_s = "(none)";

    item = cl_id2all(attr, id, &freq, &slen);
    if (cl_errno != CDA_OK) {
      cl_error("(aborting) get_id_info() failed");
      rcqp_receive_error(1);
    }
  }
  else {
    id = -1;
    freq = 0;
    slen = 0;
    item = NULL;
  }

  if (print_nr)   Rprintf("%7d\t", id);
  if (print_freqs)Rprintf("%7d\t", freq);
  if (print_len)  Rprintf("%7d\t", slen);
 Rprintf("%s\n", item ? item : fallback_s);
}

/**
 * Prints out the lexicon of a P-attribute.
 *
 * This is the business end of the cwb-lexdecode program.
 *
 * @param attr_name  Name of the attribute to decode.
 * @param rx         A regex that items must match to be printed.
 *                   NULL if no regex is to be specified.
 * @param rx_flags   IGNORE_CASE; IGNORE_DIAC; both; or neither.
 */
void
lexdecode_show(char *attr_name, char *rx, int rx_flags)
{
  int i, k, len, size;
  int attr_size;

  int *idlist = NULL;
  FILE *input_fd;

  char s[CL_MAX_LINE_LENGTH];

  Attribute *attr = NULL;

  if ((attr = cl_new_attribute(corpus, attr_name, ATT_POS)) == NULL) {
   Rprintf( "Attribute %s.%s does not exist. Aborted.\n",
            corpus_id, attr_name);
    rcqp_receive_error(1);
  }

  attr_size = cl_max_cpos(attr);
  if (cl_errno != CDA_OK) {
    cl_error("(aborting) cl_max_cpos() failed");
    rcqp_receive_error(1);
  }

  size = cl_max_id(attr);
  if (cl_errno != CDA_OK) {
    cl_error("(aborting) cl_max_id() failed");
    rcqp_receive_error(1);
  }

  if (show_size_only) {
   Rprintf("Tokens:\t%d\n", attr_size);
   Rprintf("Types:\t%d\n", size);
  }
  else {                        /* without -S option */

    if (input_filename != NULL) { /* with -F <file> option */

      if (strcmp(input_filename, "-") == 0) {
        input_fd = stdin;
      }
      else if ((input_fd = fopen(input_filename, "r")) == NULL) {
        perror(input_filename);
        rcqp_receive_error(1);
      }

      while (fgets(s, CL_MAX_LINE_LENGTH, input_fd) != NULL) {

        len = strlen(s);
        if (len <= 0) {
         Rprintf( "%s Warning: read empty string from input file (ignored)\n",
                  progname);
        }
        else {
          if (s[len-1] == '\n') { /* chomp; # :o) */
            s[len-1] = '\0';
            len--;
          }

          if (input_are_numbers)
            i = atoi(s);
          else
            i = cl_str2id(attr, s);

          if ((i < 0) && (!freq_0_if_unknown))
             Rprintf( "%s Warning: ``%s'' not found in lexicon (ignored)\n", progname, s);
            else
              lexdecode_print_item_info(attr, i, (input_are_numbers) ? NULL : s);
        }

      }

      if (input_fd != stdin)
        fclose(input_fd);

    }
    else {                        /* without -F option */

      if (rx != NULL) {
        idlist = cl_regex2id(attr, rx, rx_flags, &size);
        if ((size == 0) || (idlist == NULL))
          return;                /* no matches */
      }

      for (k = 0; k < size; k++) {

        if (idlist != NULL) {
          i = idlist[k];
        }
        else if (sort) {
          i = cl_sort2id(attr, k);
          if (cl_errno != CDA_OK) {
            cl_error("(aborting) cl_sort2id() failed");
            rcqp_receive_error(1);
          }
        }
        else {
          i = k;
        }

        lexdecode_print_item_info(attr, i, NULL);
      }
    }

  } /* ends: without -S option */
}


/**
 * Prints a message describing how to use the program to STDERR and then exits.
 */
void
lexdecode_usage(void)
{
 Rprintf( "\n");
 Rprintf( "Usage:  %s [options] <corpus>\n\n", progname);
 Rprintf( "Prints the lexicon (or part of it) of a positional attribute on NULL,\n");
 Rprintf( "optionally with frequency information. The output line format is\n");
 Rprintf( "  [ <lexicon id> TAB ] [ <frequency> TAB ] [<length> TAB ] <string>\n\n");
 Rprintf( "Options:\n");
 Rprintf( "  -P <att>  use p-attribute <att> [default: word]\n");
 Rprintf( "  -S        only show lexicon and corpus size\n");
 Rprintf( "  -r <dir>  set registry directory\n");
 Rprintf( "  -f        show frequency (number of occurrences)\n");
 Rprintf( "  -n        show internal lexicon ID\n");
 Rprintf( "  -l        show length of annotation string\n");
 Rprintf( "  -s        print in (lexically) sorted order\n");
 Rprintf( "  -p <rx>   show lexicon entries matching regexp <rx> only\n");
 Rprintf( "  -c        [with -p <rx>] ignore case\n");
 Rprintf( "  -d        [with -p <rx>] ignore diacritics\n");
 Rprintf( "  -F <file> lookup strings read from <file> ('-' for stdin)\n");
 Rprintf( "  -0        [with -F <file>] show non-existing strings with frequency 0\n");
 Rprintf( "  -N        [with -F <file>] read lexicon IDs from <file>\n");
 Rprintf( "  -h        this help page\n\n");
 Rprintf( "Part of the IMS Open Corpus Workbench v" VERSION "\n\n");
  rcqp_receive_error(2);
}


/* *************** *\
 *      MAIN()     *
\* *************** */

/**
 * Main function for cwb-lexdecode.
 *
 * @param argc   Number of command-line arguments.
 * @param argv   Command-line arguments.
 */
int
main(int argc, char **argv) {
  char *registry_directory = NULL;
  char *attr_name = DEFAULT_ATT_NAME;
  char *rx = NULL;
  int   rx_flags = 0;

  extern int optind;
  extern char *optarg;

  int c;

  /* ------------------------------------------------- PARSE ARGUMENTS */

  progname = argv[0];

  if (argc == 1)
    lexdecode_usage();

  /* parse arguments */
  while ((c = getopt(argc, argv, "+P:Sr:fnlsp:cdF:O0Nh")) != EOF) {

    switch (c) {
    case 'S':                        /* S: show lexicon size only */
      show_size_only++;
      break;

    case 'P':                        /* P: attribute name */
      attr_name = optarg;
      break;

    case 'r':                        /* r: registry directory */
      if (registry_directory == NULL)
        registry_directory = optarg;
      else {
       Rprintf( "%s: -r option used twice\n", progname);
        rcqp_receive_error(2);
      }
      break;

    case 'f':                        /* f: print frequencies */
      print_freqs++;
      break;

    case 'n':                        /* n: print line numbers */
      print_nr++;
      break;

    case 'l':                        /* l: print word length */
      print_len++;
      break;

    case 's':                        /* s: print sorted */
      sort++;
      break;

    case 'p':                        /* p: match regular expression */
      rx = optarg;
      break;

    case 'c':                        /* c: ignore case */
      rx_flags |= IGNORE_CASE;
      break;

    case 'd':                        /* d: ignore diacritics */
      rx_flags |= IGNORE_DIAC;
      break;

    case 'F':                        /* F: read strings from file ('-' for stdin) */
      input_filename = optarg;
      sort = 0;                      /* reading strings from file disables sorting and pattern matching */
      rx = NULL;
      break;

    case 'O':                        /* O: _secret_ optimisation flag (turns on CL optimisations) */
      cl_set_optimize(1);
      break;

    case '0':                        /* 0: just issue 0 as freq for unknown ids */
      freq_0_if_unknown++;
      break;

    case 'N':                        /* N: read lexicon IDs from file (with -F) */
      input_are_numbers++;
      break;

    default:
    case 'h':
      lexdecode_usage();
      break;
    }

  }

  /* single argument: corpus id */
  if (optind < argc) {

    corpus_id = argv[optind++];

    if ((corpus = cl_new_corpus(registry_directory, corpus_id)) == NULL) {
     Rprintf( "%s: Corpus %s not found in registry %s . Aborted.\n",
              progname, corpus_id,
              (registry_directory ? registry_directory : central_corpus_directory()));
      rcqp_receive_error(1);
    }

    if (optind < argc) {
     Rprintf( "Too many arguments. Try \"%s -h\" for more information.\n",
              progname);
      rcqp_receive_error(2);
    }

    lexdecode_show(attr_name, rx, rx_flags);

    cl_delete_corpus(corpus);
  }
  else {
   Rprintf( "No corpus specified. Try \"%s -h\" for more information.\n",
            progname);
    rcqp_receive_error(2);
  }

  rcqp_receive_error(0);
}
