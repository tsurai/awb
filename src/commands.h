#ifndef COMMANDS_H
#define COMMANDS_H

#include "awb.h"

int cmd_prepare_command(const char *cmd);
int cmd_switch_mode(Arg args);
int cmd_reset_mode();
int cmd_execute_command();
int cmd_openuri();
int cmd_openuri_tab();
int cmd_switch_tab();
void cmd_close_tab();
void cmd_history_back();
void cmd_history_forward();
void cmd_show_quick_history();
void cmd_close_quick_history();
void cmd_useragent();
void cmd_inspector();
void cmd_quick_history_up();
void cmd_quick_history_down();

#endif
