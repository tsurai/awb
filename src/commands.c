#include "commands.h"
#include "gui.h"
#include <webkit/webkitwebview.h>
#include <string.h>

gboolean cmd_switch_tab(Arg args)
{
  unsigned int tabnr = atoi(gdk_keyval_name(args.key->keyval));

  awb_change_to_nth_tab(tabnr);

  return 0;
}

gboolean cmd_prepare_command(const char *cmd)
{
  awb_switch_mode(MODE_COMMAND);
  gui_set_command(cmd);

  return 0;
}

gboolean cmd_switch_mode(Arg args)
{
  awb_switch_mode(args.m);

  return 0;
}

gboolean cmd_reset_mode()
{
  awb_switch_mode(MODE_NORMAL);

  return 0;
}

int cmd_execute_command(Arg argv)
{
  FunctionMap *FMcmd = (FunctionMap *)argv.p;
  int i;
  int nCmd = argv.d+1;
  const char *cmd = gtk_entry_get_text(GTK_ENTRY(awb.gui.inputbar.input));

  for(i = 0; i < nCmd; i++) {
    if(strncmp((const char *)cmd+1, FMcmd[i].action, strlen(FMcmd[i].action)) == 0) {
      FMcmd[i].func(FMcmd[i].argv);
      break;
    }
  }

  awb_switch_mode(MODE_NORMAL);

  return 0;
}

int cmd_openuri()
{
  char uri[1024];
  if(sscanf(gtk_entry_get_text(GTK_ENTRY(awb.gui.inputbar.input)), ":open %s", uri) != 1)
    return -1;

  awb_open_uri(uri);

  return 0;
}

int cmd_openuri_tab()
{
  char uri[1024];
  if(sscanf(gtk_entry_get_text(GTK_ENTRY(awb.gui.inputbar.input)), ":tabopen %s", uri) != 1)
    return -1;

  awb_new_tab(uri);

  return 0;
}

void cmd_useragent()
{
  fprintf(stderr, "useragent\n");
/*
  char input[256];
  if(sscanf(gtk_entry_get_text(GTK_ENTRY(awb.gui.inputbar.input)), ":useragent %s", input) != 1)
    return -1;
*/
}

void cmd_inspector()
{
  fprintf(stderr, "cmd_inspector\n");
  awb_toggle_inspector();
}

void cmd_close_tab()
{
  awb_close_tab(NULL);
}

void cmd_history_back()
{
  awb_history_back();
}

void cmd_history_forward()
{
  awb_history_forward();
}

void cmd_show_quick_history()
{
  gui_show_quick_history();
}

void cmd_close_quick_history()
{
  gui_close_quick_history();
}

void cmd_quick_history_up()
{
  gui_quick_history_up();
}

void cmd_quick_history_down()
{
  gui_quick_history_down();
}
