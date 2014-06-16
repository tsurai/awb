#ifndef AWB_H
#define AWB_H

#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdbool.h>
#include "database.h"
#define BUF_SIZE 32

#define UNUSED(x) (void)(x)

typedef struct _Awb Awb;
typedef struct _Gui Gui;
typedef struct _Tab Tab;
typedef struct _Arg Arg;
typedef struct _KeyBind KeyBind;
typedef struct _FunctionMap FunctionMap;

typedef gboolean (*Func)(Arg argv);

typedef enum {
  MODE_ALL         = 0x0,
  MODE_NORMAL      = 0x1,
  MODE_COMMAND     = 0x2,
  MODE_INSERT      = 0x3,
  MODE_PRIVATE     = 0x4,
  MODE_PASSTHROUGH = 0x5
} MODE;

typedef enum {
  VSPLIT  = 0x0,
  HSPLIT  = 0x1
} SPLIT_POLICY;

struct _Tab {
  GList *link;
  GtkWidget *ebox;
  GtkWidget *number;
  GtkWidget *label;
  GtkWidget *view;
  unsigned long sigIds[5];
};

struct _Arg {
  void *p;
  int d;
  const char *s;
  MODE m;
  GdkEventKey *key;
  Func f;
};

struct _KeyBind {
  const char *action;
  char *key;
  guint mod;
  MODE mode;
  bool metaCmd;
};

struct _FunctionMap {
  const char *action;
  const char *desp;
  Func func;
  Arg argv;
};

struct _Gui {
  struct _Statusbar {
    GtkWidget *eboxLeft;
    GtkWidget *widget;
    GtkWidget *uri;
    GtkWidget *hist;
    GtkWidget *progress;
    GtkWidget *buf;
    GtkWidget *ssl;
    GtkWidget *tabs;
    GtkWidget *scroll;
  } statusbar;

  struct _Inputbar {
    GtkWidget *widget;
    GtkWidget *mode;
    GtkWidget *input;
  } inputbar;

  GtkWidget *history;
  GtkWidget *tabbar;
  GtkWidget *window;
  GtkWidget *inbox;
  GtkWidget *scrollarea;
  GtkWidget *view;
};

struct _Awb {
  Gui gui;
  char buf[BUF_SIZE];
  int buf_offset;
  MODE mode;
  GList *tabs;
  Tab *current_tab;
  sqlite3 *sqldb;
};

Awb awb;

gboolean awb_change_to_nth_tab(unsigned int nr);
void awb_change_tab(Tab *tab);
void awb_get_ssl_state(void);
gboolean awb_is_mode(MODE mode);
void awb_switch_mode(MODE mode);
Tab *awb_new_tab(const char *uri);
void awb_close_tab(Tab *tab);
void awb_open_uri(const char *uri);
void awb_history_back(void);
void awb_history_forward(void);
void awb_toggle_inspector(void);

#endif
