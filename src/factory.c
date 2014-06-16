#define _GNU_SOURCE
#include "factory.h"
#include "awb.h"
#include <webkit/webkit.h>
#include <string.h>

Tab *factory_build_tab(Tab *tab)
{
  char tabnr[3] = "";
  GtkWidget *vbox;
  GtkWidget *ebox;
  GtkWidget *number;
  GtkWidget *label;
  GtkWidget *view;

  vbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  ebox = gtk_event_box_new();
  gtk_widget_set_name(ebox, "tab");
  sprintf(tabnr, "%d", g_list_length(awb.tabs));
  number = gtk_label_new(tabnr);
  gtk_widget_set_name(number, "tabnr");
  label = gtk_label_new("about:blank");
  view = webkit_web_view_new();

  tab->ebox = ebox;
  tab->number = number;
  tab->label = label;
  tab->view = view;

  gtk_box_pack_start(GTK_BOX(awb.gui.tabbar), ebox, true, true, 0);
  gtk_container_add(GTK_CONTAINER(ebox), vbox);

  gtk_box_pack_start(GTK_BOX(vbox), number, false, false, 0);
  gtk_box_pack_start(GTK_BOX(vbox), label, true, true, 0);
  gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
  gtk_misc_set_padding(GTK_MISC(number), 3, 0);

  g_object_ref(view);

  return tab;
}

GtkWidget *factory_build_statusbar()
{
  GtkWidget *statusbar;
  GtkWidget *filler;

  /* left side */
  GtkWidget *layoutLeft;
  GtkWidget *eboxLeft;
  GtkWidget *uri;
  GtkWidget *hist;
  GtkWidget *progress;

  /* right side */
  GtkWidget *layoutRight;
  GtkWidget *eboxRight;
  GtkWidget *buf;
  GtkWidget *ssl;
  GtkWidget *tabs;
  GtkWidget *scroll;

  /* setup elements */
  statusbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  filler = gtk_event_box_new();
  gtk_widget_set_name(statusbar, "statusbar");

  layoutLeft = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  eboxLeft = gtk_event_box_new();
  uri = gtk_label_new("about:blank");
  gtk_label_set_ellipsize(GTK_LABEL(uri), PANGO_ELLIPSIZE_END);
  hist = gtk_label_new("");
  progress = gtk_label_new("100%");
  gtk_widget_set_name(progress, "progress");

  layoutRight = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  eboxRight = gtk_event_box_new();
  buf = gtk_label_new("");
  ssl = gtk_label_new("(trust)");
  tabs = gtk_label_new("[1/1]");
  scroll = gtk_label_new("All");
  gtk_widget_set_name(ssl, "ssltrust");

  /* link to gui struct */
  awb.gui.statusbar.eboxLeft = eboxLeft;
  awb.gui.statusbar.widget = statusbar;
  awb.gui.statusbar.uri = uri;
  awb.gui.statusbar.hist = hist;
  awb.gui.statusbar.progress = progress;
  awb.gui.statusbar.buf = buf;
  awb.gui.statusbar.ssl = ssl;
  awb.gui.statusbar.tabs = tabs;
  awb.gui.statusbar.scroll = scroll;

  /* packing */
  gtk_box_pack_start(GTK_BOX(layoutLeft), uri, 0, 0, 0);
  gtk_box_pack_start(GTK_BOX(layoutLeft), hist, 0, 0, 0);
  gtk_box_pack_start(GTK_BOX(layoutLeft), progress, 0, 0, 0);
  gtk_container_add(GTK_CONTAINER(eboxLeft), layoutLeft);

  gtk_box_pack_start(GTK_BOX(layoutRight), buf, 0, 0, 0);
  gtk_box_pack_start(GTK_BOX(layoutRight), ssl, 0, 0, 0);
  gtk_box_pack_start(GTK_BOX(layoutRight), tabs, 0, 0, 0);
  gtk_box_pack_start(GTK_BOX(layoutRight), scroll, 0, 0, 0);
  gtk_container_add(GTK_CONTAINER(eboxRight), layoutRight);

  gtk_box_pack_start(GTK_BOX(statusbar), eboxLeft, 0, 0, 0);
  gtk_box_pack_start(GTK_BOX(statusbar), filler, 1, 1, 0);
  gtk_box_pack_start(GTK_BOX(statusbar), eboxRight, 0, 0, 0);

  return statusbar;
}

GtkWidget *factory_build_inputbar() {
  GtkWidget *inputbar;
  GtkWidget *ebox;
  GtkWidget *mode;
  GtkWidget *input;

  /* setup elements */
  ebox = gtk_event_box_new();
  inputbar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  mode = gtk_label_new("-- INSERT --");
  input = gtk_entry_new();
  gtk_widget_set_name(inputbar, "inputbar");
  gtk_entry_set_has_frame(GTK_ENTRY(input), 0);

  /* link to gui struct */
  awb.gui.inputbar.widget = inputbar;
  awb.gui.inputbar.mode = mode;
  awb.gui.inputbar.input = input;

  /* packing */
  gtk_container_add(GTK_CONTAINER(ebox), mode);
  gtk_box_pack_start(GTK_BOX(inputbar), ebox, 1, 1, 0);
  gtk_box_pack_start(GTK_BOX(inputbar), input, 1, 1, 0);
  gtk_misc_set_alignment(GTK_MISC(mode), 0, 0);

  return inputbar;
}

enum { TEXT_COLUMN, COLOR_COLUMN, N_COLUMNS };

GtkWidget *factory_build_history()
{
  GtkListStore *store;
 
  store = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);

  GtkWidget *scrollarea = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollarea), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrollarea), 132);
  GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
  gtk_widget_set_can_focus(tree, false);
  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), false);

  gtk_widget_set_name(tree, "history");
  g_object_set(G_OBJECT(tree), "rules-hint", true, NULL);
  g_object_unref(G_OBJECT(store));

  GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
  g_object_set(G_OBJECT(renderer), "font", "monospace normal 9", NULL);
  GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Uri", renderer, "text", 0, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

  column = gtk_tree_view_column_new_with_attributes("Title", renderer, "text", 1, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

  gtk_container_add(GTK_CONTAINER(scrollarea), tree);

  /* link to gui struct */
  awb.gui.history = tree;

  return scrollarea;
}

GtkWidget *factory_build_main_window()
{
  GtkCssProvider *provider;
  GdkScreen *screen;

  GtkWidget *tabbar;
  GtkWidget *window;
  GtkWidget *scroll;
  GtkWidget *vbox;
  GtkWidget *ebox;

  /* load css and apply style */
  provider = gtk_css_provider_new();
  screen = gdk_screen_get_default();
  gtk_css_provider_load_from_file(provider, g_file_new_for_path("style.css"), NULL);
  gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

  /* setup elements */
  tabbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_set_homogeneous(GTK_BOX(tabbar), true);
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "awb");
  gtk_window_set_resizable(GTK_WINDOW(window), false);
  scroll = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_NEVER, GTK_POLICY_NEVER);
  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  ebox = gtk_event_box_new();

  /* link to gui struct */
  awb.gui.tabbar = tabbar;
  awb.gui.window = window;
  awb.gui.scrollarea = scroll;
  awb.gui.inbox = ebox;

  GtkWidget *scrollarea = factory_build_history();

  /* packing */
  gtk_box_pack_start(GTK_BOX(vbox), tabbar, 0, 0, 0);
  gtk_box_pack_start(GTK_BOX(vbox), scroll, 1, 1, 0);
  gtk_box_pack_end(GTK_BOX(vbox), ebox, 0, 0, 0);
  gtk_box_pack_end(GTK_BOX(vbox), factory_build_inputbar(), 0, 0, 0);
  gtk_box_pack_end(GTK_BOX(vbox), scrollarea /*factory_build_history()*/, 0, 0, 0);
  gtk_box_pack_end(GTK_BOX(vbox), factory_build_statusbar(), 0, 0, 0);
  gtk_container_add(GTK_CONTAINER(window), vbox);

  awb_new_tab("about:blank");

  gtk_widget_show_all(awb.gui.window);
  gtk_widget_set_visible(awb.gui.statusbar.ssl, false);
  gtk_widget_set_visible(awb.gui.inputbar.widget, false);
  gtk_widget_set_visible(awb.gui.tabbar, false);
  gtk_widget_set_visible(awb.gui.history, false);

gtk_widget_set_visible(GTK_WIDGET(gtk_scrolled_window_get_vscrollbar(GTK_SCROLLED_WINDOW(scrollarea))), false);

  return window;
}
