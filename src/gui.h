#ifndef GUI_H
#define GUI_H

#include "awb.h"

void gui_block_tab(Tab *tab);
void gui_unblock_tab(Tab *tab);
void gui_set_command(const char *cmd);
void gui_switch_mode(void);
void gui_new_tab(Tab *tab);
void gui_close_tab(Tab *tab);
void gui_free(void);
void gui_show_quick_history(void);
void gui_close_quick_history(void);
void gui_quick_history_up(void);
void gui_quick_history_down(void);
int gui_init(void);

#endif
