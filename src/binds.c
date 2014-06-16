#include "binds.h"
#include "awb.h"
#include "string.h"

void get_map_data(bool metaCmd, FunctionMap **maps, int *nMaps){
  if(!metaCmd) {
    *maps = ActMaps;
    *nMaps = NUM_ACT;
  } else {
    *maps = CmdMaps;
    *nMaps = NUM_CMD;
  }
}

gboolean match_binding(GdkEventKey *keyevent)
{
  const char *key = gdk_keyval_name(keyevent->keyval);
  int i, n, nKeyBinds, nMaps;
  nKeyBinds = sizeof(KeyBinds)/sizeof(KeyBind);
/*
  printf("Key: %s, Mod: %u\n", key, keyevent->state & GDK_MODIFIER_MASK);
  fflush(stdout);
*/
   for(i = 0; i < nKeyBinds; i++) {
    if(awb_is_mode(KeyBinds[i].mode) && (strcmp(KeyBinds[i].key, key) == 0) && (KeyBinds[i].mod == (keyevent->state & gtk_accelerator_get_default_mod_mask()))) {
      const char *action = KeyBinds[i].action;
      FunctionMap *maps = NULL;
      get_map_data(KeyBinds[i].metaCmd, &maps, &nMaps);

      for(n = 0; n < nMaps; n++) {
        if(strcmp(action, maps[n].action) == 0) {
          if(KeyBinds[i].metaCmd) {
            cmd_prepare_command(maps[n].action);
            return 1;
          }

          /* make all bindings boolean type and allow them to return true
           * to make the loop execute all bindings that match and not only
           * one of them */
          maps[n].argv.key = keyevent;
          maps[n].func(maps[n].argv);
          return 1;
        }
      }
    }
  }

  return 0;
}
