#include "gui.h"
#include "factory.h"
#include "awb.h"
#include <webkit/webkit.h>
#include <string.h>
#include "commands.h"
#include "binds.h"
#include <string.h>
#include <strings.h>

gboolean button_press_cb(WebKitWebView *w, GdkEventButton *btnevent, gpointer data)
{
  UNUSED(w);
  UNUSED(data);
  
  if(btnevent->type == GDK_BUTTON_PRESS) {
    WebKitHitTestResult *hitres = webkit_web_view_get_hit_test_result(w, btnevent);
    WebKitHitTestResultContext context;
    g_object_get(hitres, "context", &context, NULL);

    if(context & WEBKIT_HIT_TEST_RESULT_CONTEXT_EDITABLE)
      awb_switch_mode(MODE_INSERT);
    else
      if(awb_is_mode(MODE_INSERT))
        awb_switch_mode(MODE_NORMAL);
  }

  return false;
}

gboolean key_press_cb(WebKitWebView *w, GdkEventKey *keyevent, gpointer data)
{
  UNUSED(w);
  UNUSED(data);

  if(match_binding(keyevent))
    return true;

  if(awb_is_mode(MODE_INSERT) || awb_is_mode(MODE_COMMAND))
    return false;

  /* add to buffer */
  /* match buffer commands */

  return true;
}

gboolean tab_click_cb(GtkWidget *w, GdkEventButton *btnevent, Tab *tab)
{
  UNUSED(w);

  if(btnevent->type == GDK_BUTTON_PRESS) {
    awb_change_tab(tab);
  }

  return false;
}

gboolean title_change_cb(GtkWidget *w, GParamSpec *pspec, GtkWidget *label)
{
  UNUSED(pspec);

  gtk_label_set_text(GTK_LABEL(label), webkit_web_view_get_title(WEBKIT_WEB_VIEW(w)));

  return false;
}

static gboolean input_focus_out_cb(GtkWidget *w, GParamSpec *pspec, gpointer *data)
{
  UNUSED(w);
  UNUSED(pspec);
  UNUSED(data);

  if(awb_is_mode(MODE_COMMAND))
    awb_switch_mode(MODE_NORMAL);

  return false;
}

/* check this shit */
static void hovering_link_cb(GtkWidget *w, char *title, char *uri, gpointer data)
{
  UNUSED(data);
  UNUSED(title);
  
  if(uri)
    gtk_label_set_text(GTK_LABEL(awb.gui.statusbar.uri), uri);
  else
    /* gtk_label_set_text(GTK_LABEL(awb.gui.statusbar.uri), webkit_web_view_get_uri(WEBKIT_WEB_VIEW(awb.gui.view))); */
    gtk_label_set_text(GTK_LABEL(awb.gui.statusbar.uri), webkit_web_view_get_uri(WEBKIT_WEB_VIEW(w)));
}

gboolean input_length_cb(GtkWidget *w, GParamSpec *pspec, gpointer data)
{
  UNUSED(pspec);
  UNUSED(data);

  const char *input = gtk_entry_get_text(GTK_ENTRY(w));
 /*&& gtk_widget_is_visible(awb.gui.history)*/
  if((strstr(input, ":open ") != NULL || strstr(input, ":tabopen ") != NULL)) {

    int cursor_pos = gtk_editable_get_position(GTK_EDITABLE(awb.gui.inputbar.input));
    char *url = index(input, ' ')+1;

    if(strncmp(url, "", strlen(url)) == 0) {
      return false;
    }
    int offset = url-input;
    char *term = (char *)malloc(cursor_pos-offset+7);
/*
    printf("url: %s\ninput: %s\ncursor: %d\nlen: %d\n", url, input, cursor_pos, cursor_pos-offset+7);
    fflush(NULL);

    sprintf(term, "%%://%.*s%%", cursor_pos-offset+1, url);
    printf("%s\n", term);
*/
    sqlite3_stmt *stmt;
    int error = -1;
    const char *query;

    query = "SELECT uri, title FROM `history` WHERE baseurl LIKE ?1 OR title LIKE ?2";
    if(sqlite3_prepare_v2(awb.sqldb, query, strlen(query), &stmt, NULL) != SQLITE_OK) {
      printf("History stmt %d\n", error);
      fflush(stdout);
    }
    sqlite3_bind_text(stmt, 1, term, strlen(term), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, term, strlen(term), SQLITE_STATIC);

    unsigned const char *rowuri;
    unsigned const char *rowtitle;
    GtkListStore *store;
    GtkTreeIter iter;

    store = (GtkListStore *)gtk_tree_view_get_model(GTK_TREE_VIEW(awb.gui.history));
    gtk_list_store_clear(store);
    gtk_tree_model_get_iter_first((GtkTreeModel *)store, &iter);

    while((error = sqlite3_step(stmt)) == SQLITE_ROW) {
      rowuri = sqlite3_column_text(stmt, 0);
      rowtitle = sqlite3_column_text(stmt, 1);

      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, (char *)rowuri, -1);
      gtk_list_store_set(store, &iter, 1, (char *)rowtitle, -1);
    }
    free(term);

    GValue value = {0,};
    if(gtk_tree_model_get_iter_first((GtkTreeModel *)store, &iter)) {
      gtk_tree_model_get_value((GtkTreeModel *)store, &iter, 0, &value);
      const char *str = index(g_value_get_string(&value), '/')+2;

      char out[256];
      sprintf(out, ":tabopen %s", str);
      gtk_entry_set_text(GTK_ENTRY(awb.gui.inputbar.input), out);
      g_value_unset(&value);

      gtk_editable_set_position(GTK_EDITABLE(awb.gui.inputbar.input), cursor_pos+1);

      return true;
    }
  }

  if(gtk_entry_get_text_length(GTK_ENTRY(w)) == 0)
    awb_switch_mode(MODE_NORMAL);

  return false;
}

gboolean scroll_cb(GtkAdjustment *adj, GParamSpec *pspec, gpointer data)
{
  UNUSED(pspec);
  UNUSED(data);

  char scroll[3];
  int upper = gtk_adjustment_get_upper(adj);
  int size = gtk_adjustment_get_page_size(adj);

  if((upper - size) == 0) {
    sprintf(scroll, "All");
  } else {
    int percent = (int)(gtk_adjustment_get_value(adj) / (upper - size) * 100);

    if(percent == 0) {
      sprintf(scroll, "Top");
    } else if(percent == 100) {
      sprintf(scroll, "Bot");
    } else {
      sprintf(scroll, "%2.d%%", percent);
    }
  }
  gtk_label_set_text(GTK_LABEL(awb.gui.statusbar.scroll), scroll);

  return false;
}

static void load_progress_cb(WebKitWebView *v, GParamSpec *pspec, gpointer data)
{
  UNUSED(pspec);
  UNUSED(data);

  char progress[6];
  sprintf(progress, "(%d%%)", (int)(webkit_web_view_get_progress(v) * 100));
  gtk_label_set_text(GTK_LABEL(awb.gui.statusbar.progress), progress);
}

static void load_changed_cb(WebKitWebView *v, GParamSpec *pspec, gpointer data)
{
  UNUSED(pspec);
  UNUSED(data);

  switch(webkit_web_view_get_load_status(v)) {
    case WEBKIT_LOAD_PROVISIONAL:
      gtk_widget_set_visible(awb.gui.statusbar.progress, true);
      gtk_label_set_text(GTK_LABEL(awb.gui.statusbar.uri), webkit_web_view_get_uri(v));
      break;
    case WEBKIT_LOAD_COMMITTED:
      awb_get_ssl_state();
      gtk_label_set_text(GTK_LABEL(awb.gui.statusbar.uri), webkit_web_view_get_uri(v));
      break;
    case WEBKIT_LOAD_FINISHED:
      gtk_widget_set_visible(awb.gui.statusbar.progress, false);

      const char *uri = webkit_web_view_get_uri(v);
      const char *title = webkit_web_view_get_title(v);
      sqlite3_stmt *stmt;
      sqlite3_stmt *stmt2;
      int error = -1;
      const char *query;

      /* encapsule into awb.c which calls db.c (or something like that)*/
      /* check against whitelist */
      /* save whitelist as url md5 to prevent reading */
      if(strcmp(uri, "about:blank") == 0)
          break;

      /* parse url */
      char *protocol = (char *)strndup(uri, strstr(uri, "://") - uri);
      uri += strlen(protocol) + 3;
      char *baseurl = (char *)strndup(uri, strchr(uri, '/') - uri);
      char *request = (char* )(uri + strlen(baseurl));
      printf("Protocol: %s\nBaseurl: %s\nRequest: %s\n", protocol, baseurl, request);

      query = "SELECT id FROM `history` WHERE protocol = ?1 AND baseurl = replace(?2,\"&\",\"&amp;\") AND request = replace(?3,\"&\",\"&amp;\")";
      if((error = sqlite3_prepare_v2(awb.sqldb, query, strlen(query), &stmt, NULL)) != SQLITE_OK) {
        printf("Exists stmt. %s\n", sqlite3_errstr(error));
        fflush(stdout);
      }
      sqlite3_bind_text(stmt, 1, protocol, strlen(protocol), SQLITE_STATIC);
      sqlite3_bind_text(stmt, 2, baseurl, strlen(baseurl), SQLITE_STATIC);
      sqlite3_bind_text(stmt, 3, request, strlen(request), SQLITE_STATIC);

      if(sqlite3_step(stmt) == SQLITE_DONE) {
        query = "INSERT INTO `history` (protocol, baseurl, request, title, hits, lasthit) VALUES (?1, ?2, replace(?3,\"&\",\"&amp;\"), replace(?4,\"&\",\"&amp;\"), 1, datetime('now'))";

        if((error = sqlite3_prepare_v2(awb.sqldb, query, strlen(query), &stmt2, NULL)) != SQLITE_OK) {
          printf("%s\nINSERT STMT ERROR %s", query, sqlite3_errstr(error));
          fflush(stdout);
        }

        sqlite3_bind_text(stmt, 1, protocol, strlen(protocol), SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, baseurl, strlen(baseurl), SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, request, strlen(request), SQLITE_STATIC);
        sqlite3_bind_text(stmt2, 4, title, strlen(title), SQLITE_STATIC);

      } else {
        int id = sqlite3_column_int(stmt, 0);
        query = "UPDATE `history` SET title = replace(?1, \"&\", \"&amp;\"), hits = (hits + 1), lasthit = datetime('now') WHERE id = ?2";

        if(sqlite3_prepare_v2(awb.sqldb, query, 512, &stmt2, NULL) != SQLITE_OK) {
          printf("INSERT 2 STMT ERROR");
          fflush(stdout);
        }
        sqlite3_bind_text(stmt2, 1, title, strlen(title), SQLITE_STATIC);
        sqlite3_bind_int(stmt2, 2, id);
      }
      break;
    default:
      break;
  }
}

static void destroy_cb(GtkWidget *w, gpointer data)
{
  UNUSED(w);
  UNUSED(data);

  gui_free();
  gtk_main_quit();
}

void gui_resize_history_columns(int space)
{
  gtk_tree_view_column_set_sizing(gtk_tree_view_get_column(GTK_TREE_VIEW(awb.gui.history), 0), GTK_TREE_VIEW_COLUMN_FIXED);
  gtk_tree_view_column_set_fixed_width(gtk_tree_view_get_column(GTK_TREE_VIEW(awb.gui.history), 0), space/2);

  gtk_tree_view_column_set_sizing(gtk_tree_view_get_column(GTK_TREE_VIEW(awb.gui.history), 1), GTK_TREE_VIEW_COLUMN_FIXED);
  gtk_tree_view_column_set_fixed_width(gtk_tree_view_get_column(GTK_TREE_VIEW(awb.gui.history), 1), space/2);
}

static void resize_cb(GtkWidget *w, GdkRectangle *allocation, gpointer data)
{
  UNUSED(w);
  UNUSED(data);

  gui_resize_history_columns(allocation->width);
}

void gui_show_quick_history(void)
{
  if(!gtk_widget_is_visible(awb.gui.history))
    gtk_widget_set_visible(awb.gui.history, true);

  input_length_cb(awb.gui.inputbar.input, NULL, NULL);

  GtkTreeIter iter;
  if(gtk_tree_model_get_iter_first(gtk_tree_view_get_model(GTK_TREE_VIEW(awb.gui.history)), &iter))
    gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(awb.gui.history)), &iter);
}

void gui_close_quick_history(void)
{
  if(gtk_widget_is_visible(awb.gui.history))
    gtk_widget_set_visible(awb.gui.history, false);
}

void gui_quick_history_up(void)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(awb.gui.history));
  
  if(gtk_tree_selection_get_selected(selection, &model, &iter) == true) {
    if(gtk_tree_model_iter_previous(model, &iter)) {
      gtk_tree_selection_select_iter(selection, &iter);
    } else {
      gtk_tree_selection_select_path(selection, gtk_tree_path_new_from_indices(gtk_tree_model_iter_n_children(model, NULL), -1, -1));
      gtk_tree_selection_get_selected(selection, NULL, &iter);
    }

    gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(awb.gui.history), gtk_tree_model_get_path(model, &iter), NULL, true, 0.5, 0.0);
  }
}

void gui_quick_history_down(void)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(awb.gui.history));
  
  if(gtk_tree_selection_get_selected(selection, &model, &iter) == true) {
    if(gtk_tree_model_iter_next(model, &iter)) {
      gtk_tree_selection_select_iter(selection, &iter);
    } else {
      gtk_tree_model_get_iter_first(model, &iter);
      gtk_tree_selection_select_iter(selection, &iter);
    }

    gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(awb.gui.history), gtk_tree_model_get_path(model, &iter), NULL, true, 1.0, 0.0);
    
    int pos = 10;
    GValue value = {0,};
    gtk_tree_model_get_value(model, &iter, 0, &value);
    const char *str = g_value_get_string(&value);
   
    gtk_editable_delete_text(GTK_EDITABLE(awb.gui.inputbar.input), pos, -1);
    gtk_editable_insert_text(GTK_EDITABLE(awb.gui.inputbar.input), str, strlen(str), &pos);
    g_value_unset(&value);
  }
}

void gui_switch_mode(void)
{
  switch(awb.mode) {
    case MODE_NORMAL:
      gtk_widget_set_visible(awb.gui.inputbar.widget, false);
      gtk_widget_set_visible(awb.gui.history, false);
      break;
    case MODE_COMMAND:
      gtk_widget_set_visible(awb.gui.inputbar.mode, false);
      gtk_widget_set_visible(awb.gui.inputbar.input, true);
      gtk_entry_set_text(GTK_ENTRY(awb.gui.inputbar.input), ":");
      gtk_widget_grab_focus(awb.gui.inputbar.input);
      gtk_editable_set_position(GTK_EDITABLE(awb.gui.inputbar.input), true);
      gtk_widget_set_visible(awb.gui.inputbar.widget, true);
      break;
    case MODE_INSERT:
      gtk_widget_set_visible(awb.gui.inputbar.input, false);
      gtk_widget_set_visible(awb.gui.history, false);
      gtk_widget_set_visible(awb.gui.inputbar.mode, true);
      gtk_widget_set_visible(awb.gui.inputbar.widget, true);
      break;
    default:
      break;
  }
}

void gui_set_command(const char *cmd)
{
  gint pos = 1;
  char cmd_str[64];

  sprintf(cmd_str, "%s ", cmd);
  gtk_editable_insert_text(GTK_EDITABLE(awb.gui.inputbar.input), cmd_str, strlen(cmd_str), &pos);
  gtk_editable_set_position(GTK_EDITABLE(awb.gui.inputbar.input), pos);
}

void gui_block_tab(Tab *tab)
{
  int i;
  int nSig = sizeof(tab->sigIds)/sizeof(unsigned long);
  for(i = 0; i < nSig; i++) {
    g_signal_handler_block(G_OBJECT(tab->view), tab->sigIds[i]);
  }
}

void gui_unblock_tab(Tab *tab)
{
  int i;
  int nSig = sizeof(tab->sigIds)/sizeof(unsigned long);
  for(i = 0; i < nSig; i++) {
    if(tab->sigIds[i] == 0)
      continue;

    g_signal_handler_unblock(G_OBJECT(tab->view), tab->sigIds[i]);
  }
}

void gui_new_tab(Tab *tab)
{
  factory_build_tab(tab);

  awb_change_tab(tab);

  g_signal_connect(G_OBJECT(tab->view), "notify::title", G_CALLBACK(title_change_cb), tab->label);
  g_signal_connect(G_OBJECT(tab->ebox), "button-press-event", G_CALLBACK(tab_click_cb), tab);
  tab->sigIds[0] = g_signal_connect(G_OBJECT(tab->view), "notify::progress", G_CALLBACK(load_progress_cb), NULL);
  tab->sigIds[1] = g_signal_connect(G_OBJECT(tab->view), "notify::load-status", G_CALLBACK(load_changed_cb), NULL);
  tab->sigIds[2] = g_signal_connect(G_OBJECT(tab->view), "hovering-over-link", G_CALLBACK(hovering_link_cb), NULL);
  tab->sigIds[3] = g_signal_connect(G_OBJECT(tab->view), "button-press-event", G_CALLBACK(button_press_cb), NULL);
  tab->sigIds[4] = g_signal_connect(G_OBJECT(awb.gui.view), "focus-in-event", G_CALLBACK(input_focus_out_cb), NULL);

  if(g_list_length(awb.tabs) > 0)
      gtk_widget_show_all(awb.gui.tabbar);
  else
      gtk_widget_set_visible(awb.gui.tabbar, false);
}

void gui_close_tab(Tab *tab)
{
  if(!tab->link->prev && !tab->link->next)
    return;

  gtk_widget_destroy(tab->view);
  gtk_widget_destroy(tab->ebox);
  awb.current_tab = NULL;

  /* adjust tab nr label */
  char num[3] = "";
  int pos = g_list_position(awb.tabs, tab->link)+1;
  Tab *nextTab = tab;
  while(nextTab->link->next != NULL) {
    nextTab = nextTab->link->next->data;
    sprintf(num, "%d", pos++);
    gtk_label_set_text(GTK_LABEL(nextTab->number), num);
  }

  if(tab->link->prev)
    awb_change_tab((Tab *)tab->link->prev->data);
  else
    awb_change_tab((Tab *)tab->link->next->data);
}

void gui_free(void)
{
  GList *entry = NULL;
  Tab *tab = NULL;
  while((entry = g_list_first(awb.tabs)) != NULL) {
    tab = (Tab *)entry->data;
    gtk_widget_destroy(tab->view);
    awb.tabs = g_list_remove(awb.tabs, tab);
  };
}

int gui_init(void)
{
  /* build all gui elements */
  factory_build_main_window();

  gtk_accelerator_set_default_mod_mask(GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK | GDK_SUPER_MASK | GDK_HYPER_MASK | GDK_META_MASK);

  /* hook up signals */
  g_signal_connect(G_OBJECT(awb.gui.window), "destroy", G_CALLBACK(destroy_cb), NULL);
  g_signal_connect(G_OBJECT(awb.gui.window), "key-press-event", G_CALLBACK(key_press_cb), NULL);
  g_signal_connect(G_OBJECT(awb.gui.window), "size-allocate", G_CALLBACK(resize_cb), NULL);
  g_signal_connect(G_OBJECT(gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(awb.gui.scrollarea))), "notify::page-size", G_CALLBACK(scroll_cb), NULL);
  g_signal_connect(G_OBJECT(gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(awb.gui.scrollarea))), "value-changed", G_CALLBACK(scroll_cb), NULL);
  g_signal_connect(G_OBJECT(awb.gui.inputbar.input), /*"notify::text-length"*/"notify::text", G_CALLBACK(input_length_cb), NULL);
  g_signal_connect(webkit_web_view_get_main_frame(WEBKIT_WEB_VIEW(awb.gui.view)), "scrollbars-policy-changed", G_CALLBACK(gtk_true), NULL);

  /* show window */
  /*gtk_widget_show_all(awb.gui.window);
  gtk_widget_set_visible(awb.gui.statusbar.ssl, false);
  gtk_widget_set_visible(awb.gui.inputbar.widget, false);
  gtk_widget_set_visible(awb.gui.tabbar, false);
  gtk_widget_set_visible(awb.gui.history, false);*/
  return 1;
}
