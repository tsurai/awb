#ifndef BINDS_H
#define BINDS_H

#include "commands.h"

gboolean match_binding(GdkEventKey *keyevent);

static const KeyBind KeyBinds[] = {
  { "open",             "o",        0,                MODE_NORMAL,    true },
  { "tabopen",          "t",        0,                MODE_NORMAL,    true },
  { "tabclose",         "d",        0,                MODE_NORMAL,    false },
  { "command_mode",     "colon",    GDK_SHIFT_MASK,   MODE_NORMAL,    false },
  { "insert_mode",      "i",        0,                MODE_NORMAL,    false },
  { "exec",             "Return",   0,                MODE_COMMAND,   false },
  { "historyopen",      "Tab",      0,                MODE_COMMAND,   false },
  { "historyclose",     "Escape",   0,                MODE_COMMAND,   false },
  { "historyup",        "Up",       0,                MODE_COMMAND,   false },
  { "historydown",      "Down",     0,                MODE_COMMAND,   false },
  { "history_back",     "h",        0,                MODE_NORMAL,    false },
  { "history_forward",  "l",        0,                MODE_NORMAL,    false },
  { "tabswitch",        "1",        GDK_MOD1_MASK,    MODE_NORMAL,    false },
  { "tabswitch",        "2",        GDK_MOD1_MASK,    MODE_NORMAL,    false },
  { "tabswitch",        "3",        GDK_MOD1_MASK,    MODE_NORMAL,    false },
  { "tabswitch",        "4",        GDK_MOD1_MASK,    MODE_NORMAL,    false },
  { "tabswitch",        "5",        GDK_MOD1_MASK,    MODE_NORMAL,    false },
  { "tabswitch",        "6",        GDK_MOD1_MASK,    MODE_NORMAL,    false },
  { "tabswitch",        "7",        GDK_MOD1_MASK,    MODE_NORMAL,    false },
  { "tabswitch",        "8",        GDK_MOD1_MASK,    MODE_NORMAL,    false },
  { "tabswitch",        "9",        GDK_MOD1_MASK,    MODE_NORMAL,    false },
  { "tabswitch",        "0",        GDK_MOD1_MASK,    MODE_NORMAL,    false },
  { "cancel",           "Escape",   0,                MODE_ALL,       false },
};

static FunctionMap CmdMaps[] = {
  { "open",             "Opens an uri",               ((Func)cmd_openuri),      { NULL } },
  { "tabopen",          "Opens an uri in a new tab",  ((Func)cmd_openuri_tab),  { NULL } },
  { "useragent",        "Gets or sets useragent",     ((Func)cmd_useragent),    { NULL } },
  { "inspector",        "Toggles inspector",          ((Func)cmd_inspector),    { NULL } },
};
static const int NUM_CMD = sizeof(CmdMaps)/sizeof(FunctionMap);

static FunctionMap ActMaps[] = {
  { "command_mode",     "Switch to command mode",           ((Func)cmd_switch_mode),        { .m = MODE_COMMAND } },
  { "insert_mode",      "Switch to insert mode",            ((Func)cmd_switch_mode),        { .m = MODE_INSERT } },
  { "tabclose",         "Closes current tab",               ((Func)cmd_close_tab),          { NULL } },
  { "cancel",           "Resets current mode",              ((Func)cmd_reset_mode),         { NULL } },
  { "exec",             "Executes a command",               ((Func)cmd_execute_command),    { .p = CmdMaps, .d = 3 } },
  { "tabswitch",        "Switches the current tab",         ((Func)cmd_switch_tab),         { NULL } },
  { "historyopen",      "Shows quick history",              ((Func)cmd_show_quick_history), { NULL } },
  { "historyclose",     "Closes quick history",             ((Func)cmd_close_quick_history),{ NULL } },
  { "historyup",        "Moves up in quick history",        ((Func)cmd_quick_history_up),   { NULL } },
  { "historydown",      "Moves down in quick history",      ((Func)cmd_quick_history_down), { NULL } },
  { "history_back",     "Goes back in history",             ((Func)cmd_history_back),       { NULL } },
  { "history_forward",  "Goes forward in history",          ((Func)cmd_history_forward),    { NULL } },
};
static const int NUM_ACT = sizeof(ActMaps)/sizeof(FunctionMap);

/*
static FunctionMap FMaps[] = {
  { "open",             "Opens an uri",               ((Func)cmd_prepare_command),    { .s = "open", .f = ((Func)cmd_openuri) } },
  { "tabopen",          "Opens an uri in a new tab",  ((Func)cmd_prepare_command),    { .s = "tapopen", .f = ((Func)cmd_openuri_tab) } },
  { "tabclose",         "Closes tab",                 ((Func)cmd_close_tab),          { NULL } },
  { "command_mode",     "Switch to command mode",     ((Func)cmd_switch_mode),        { .m = MODE_COMMAND } },
  { "insert_mode",      "Switch to insert mode",      ((Func)cmd_switch_mode),        { .m = MODE_INSERT } },
  { "cancel",           "Resets current mode",        ((Func)cmd_reset_mode),         { NULL } },
  { "exec",             "Executes a command",         ((Func)cmd_execute_command),    { NULL } },
  { "tabswitch",        "Switches the current tab",   ((Func)cmd_switch_tab),         { NULL } },
  { "history_back",     "Goes back in history",       ((Func)cmd_history_back),       { NULL } },
  { "history_forward",  "Goes forward in history",    ((Func)cmd_history_forward),    { NULL } },
};*/

#endif
