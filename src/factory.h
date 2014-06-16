#ifndef FACTORY_H
#define FACTORY_H

#include <gtk/gtk.h>
#include "awb.h"

Tab *factory_build_tab(Tab *tab);
GtkWidget *factory_build_statusbar();
GtkWidget *factory_build_inputbar();
GtkWidget *factory_build_main_window();

#endif
