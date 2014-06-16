#ifndef ADBLOCK_H
#define ADBLOCK_H

#include <webkit/webkitwebview.h>

void adblock_connect(WebKitWebView *v);
void adblock_init(const char *file);

#endif
