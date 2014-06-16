#include "adblock.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <regex.h>
#include <string.h>
#include <stdbool.h>
#include <glib.h>

#define BUFFER_SIZE 8192

struct {
  const char *listfilename;
  GList *regexrules;
} adblock;

typedef struct {
  char *s;
  unsigned int offset;
} buffer_t;

bool convert_to_regex(buffer_t *buf) {
  int ret;
  regex_t regex;

  if(buf->offset == 0)
    return false;

  ret = regcomp(&regex, buf->s, REG_NOSUB | REG_EXTENDED);
  if(ret == 0) {
    adblock.regexrules = g_list_append(adblock.regexrules, (gpointer)&regex);
    return true;
  }

  return false;
}

void skip_line(char *s, long int *pos, unsigned int *offset) {
  char c;
  do {
    c = s[*pos];
    *pos = *pos + 1;
  } while(c != '\n' && c != EOF);
  *offset = 0;
}

bool is_peek(char *s, long int *pos, char c, bool consume) {
  if(s[*pos+1] == c) {
    if(consume)
      *pos = *pos+1;
    return true;
  }
  return false;
}

void reset_buffer(buffer_t *buf) {
  memset(buf->s, '\0', BUFFER_SIZE);
  buf->offset = 0;
}

void add_n_to_buffer(buffer_t *buf, const char *s, int n) {
  strncpy(buf->s+buf->offset, s, n);
  buf->offset += n;
}

void add_to_buffer(buffer_t *buf, char c) {
  *(buf->s+buf->offset) = c;
  buf->offset += 1;
}

bool parse_list(const char *filename) {
  FILE *f;
  char c;
  long int pos = 0;
  int listsize = 0;
  char *filterlist;
  buffer_t buf;
  buf.offset = 0;
  buf.s = (char *)malloc(BUFFER_SIZE);

  if((f = fopen(filename, "r")) == NULL) {
    fprintf(stderr, "Can't open file");
    return false;
  }

  fseek(f, 0, SEEK_END);
  listsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  filterlist = (char *)malloc(listsize*sizeof(char));
  fread(filterlist, 1, listsize, f);

  fclose(f);

  while((c = filterlist[pos++]) != EOF) {
    /* line start */
    if(buf.offset == 0) {
      /* clear buffer */
      reset_buffer(&buf);

      switch(c) {
        case '!':
        case '[':
          skip_line(filterlist, &pos, &buf.offset);
          break;
        case '|':
          if(is_peek(filterlist, &pos, '|', true))
            add_n_to_buffer(&buf, "^(http://|https://)(www.)?", 26);
          else
            add_to_buffer(&buf, '^');
          break;
        case '@':
          /* exception */
          break;
        case '#':
          /* whatever this does */
          break;
        default:
          add_n_to_buffer(&buf, "^.*", 3);
          add_to_buffer(&buf, c);
          break;
      }
      continue;
    }

    /* is like end? */
    if(is_peek(filterlist, &pos, '\n', false)) {
      if(c == '|') {
        add_to_buffer(&buf, '$');
      } else {
        add_to_buffer(&buf, c);
        add_n_to_buffer(&buf, ".*$", 3);
      }
      continue;
    }

    /* everything else */
    switch(c) {
      case '\r':
      case '\n':
        convert_to_regex(&buf);
        reset_buffer(&buf);
        break;
      case '*':
        add_n_to_buffer(&buf, ".*", 2);
        break;
      case '^':
        add_n_to_buffer(&buf, "(/|:|?|=|&)", 11);
        break;
      case '#':
        /* exceptions */
        break;
      case '.':
        add_n_to_buffer(&buf, "\\.", 2);
        break;
      default:
        add_to_buffer(&buf, c);
        break;
    }
  }
  free(buf.s);
  free(filterlist);
  return true;
}
/*
void frame_created_cb(WebKitWebView *v, WebKitWebFrame *f, gpointer data) {
}

void adblock_connect(WebKitWebView *v) {
  g_signal_connect(G_OBJECT(v), "frame-created", G_CALLBACK(frame_created_cb), NULL);
}
*/
void adblock_init(const char *file) {
  adblock.listfilename = file;
  adblock.regexrules = NULL;
  if(!parse_list(file)) {
    fprintf(stderr, "Couldn't parse filterlist\n");
  }
}
