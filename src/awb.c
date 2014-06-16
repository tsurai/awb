#include <gtk/gtk.h>
#include "awb.h"
#include "gui.h"
#include "factory.h"
#include "adblock.h"
#include <webkit/webkit.h>
#include <unistd.h>
#include <string.h>
#include <libsoup/soup-message.h>
#include <libsoup/soup-uri.h>
#include <gio/gio.h>
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

void awb_reset_buffer(void)
{
  awb.buf_offset = 0;
  memset((void *)awb.buf, '\0', BUF_SIZE);
}

void awb_add_to_buffer(char c)
{
  awb.buf[awb.buf_offset++] = c;
}

void awb_switch_mode(MODE mode)
{
  if(mode == MODE_ALL)
    awb.mode = MODE_NORMAL;
  else
    awb.mode = mode;

  gui_switch_mode();
}

static char *awb_parse_uri(const char *uri)
{
  if(!uri || !uri[0])
    return "about:blank";

  if(strstr(uri, "://"))
      return (char *)uri;

  if(access(uri, F_OK) == 0) {
    if(g_path_is_absolute(uri))
        return g_strdup_printf("file://%s", uri);
    else {
      /* make absolute */
      printf("\nrelative path\n");
      fflush(stdout);
    }
  }

  return g_strdup_printf("http://%s", uri);
}

void awb_request_started_cb(SoupSession *session, SoupMessage *msg, SoupSocket *s, gpointer data) {
    UNUSED(session);
    UNUSED(s);
    UNUSED(data);

    GTlsCertificate *cert = NULL;
    GTlsCertificateFlags errors;

    if((soup_message_get_https_status(msg, &cert, &errors) != true)) {
      printf("failed to fetch cert\nCert: %p\nError: %d\n", cert, errors);
      fflush(stdout);
    }

    GByteArray *crtPem;
    g_object_get(G_OBJECT(cert), "certificate", &crtPem, NULL);

    gnutls_datum_t certdata;
    certdata.data = crtPem->data;
    certdata.size = crtPem->len;

    gnutls_x509_crt_t x509_cert;
    if(0 != gnutls_x509_crt_init(&x509_cert)) {
      printf("failed!");
      fflush(stdout);
    }
    if(0 != gnutls_x509_crt_import(x509_cert, &certdata, GNUTLS_X509_FMT_DER)) {
      printf("failed!");
      fflush(stdout);
    }

    char buf[512];
    unsigned int bufsize = 512;

    gnutls_x509_crt_get_issuer_dn(x509_cert, buf, &bufsize);
    printf("\nissuer dn: %s\n", buf);
    fflush(stdout);
}

void awb_get_ssl_state(void)
{
  const char *uri = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(awb.gui.view));
  if(uri && !strncmp(uri, "https", 5)) {
    WebKitWebFrame *frame = webkit_web_view_get_main_frame(WEBKIT_WEB_VIEW(awb.gui.view));
    WebKitWebDataSource *src = webkit_web_frame_get_data_source(frame);
    WebKitNetworkRequest *req = webkit_web_data_source_get_request(src);

    if(!frame || ! src || ! req)
        return;

    SoupMessage *msg = webkit_network_request_get_message(req);
    if(!msg)
        return;

    bool ssl = ((soup_message_get_flags(msg) & SOUP_MESSAGE_CERTIFICATE_TRUSTED) ? true : false);
    if(ssl) {
      gtk_label_set_text(GTK_LABEL(awb.gui.statusbar.ssl), "(trust)");
      gtk_widget_set_name(awb.gui.statusbar.ssl, "ssltrust");
    } else {
      gtk_label_set_text(GTK_LABEL(awb.gui.statusbar.ssl), "(notrust)");
      gtk_widget_set_name(awb.gui.statusbar.ssl, "sslnotrust");
    }

    gtk_widget_set_visible(awb.gui.statusbar.ssl, true);
    return;
  }

  gtk_widget_set_visible(awb.gui.statusbar.ssl, false);
}

void awb_open_uri(const char *uri)
{
  uri = awb_parse_uri(uri);
  webkit_web_view_load_uri(WEBKIT_WEB_VIEW(awb.gui.view), uri);
  gtk_label_set_text(GTK_LABEL(awb.gui.statusbar.uri), uri);
}

gboolean awb_is_mode(MODE mode)
{
  return (mode == MODE_ALL) ? 1 : (awb.mode == mode);
}

gboolean awb_change_to_nth_tab(unsigned int nr)
{
  if(nr > g_list_length(awb.tabs))
      return false;

  Tab *tab = (Tab *)g_list_nth_data(awb.tabs, ((nr == 0) ? 10 : nr - 1));

  if(tab != NULL)
    awb_change_tab(tab);
  else
    return false;

  return true;
}

void awb_change_tab(Tab *tab)
{
 if(awb.current_tab != NULL) {
    gtk_widget_set_name(awb.current_tab->ebox, "tab");
    gui_block_tab(awb.current_tab);
    g_object_ref(awb.gui.view);
    gtk_container_remove(GTK_CONTAINER(awb.gui.scrollarea), awb.gui.view);
 }

  gtk_widget_set_name(tab->ebox, "tab-active");
  awb.gui.view = tab->view;
  gtk_container_add(GTK_CONTAINER(awb.gui.scrollarea), tab->view);
  awb.current_tab = tab;
  gui_unblock_tab(tab);

  if(webkit_web_view_get_load_status(WEBKIT_WEB_VIEW(tab->view)) != WEBKIT_LOAD_FINISHED)
    gtk_widget_set_visible(awb.gui.statusbar.progress, true);
  else
    gtk_widget_set_visible(awb.gui.statusbar.progress, false);

  char tabs_label[10] = "";
  sprintf(tabs_label, "[%s/%d]", gtk_label_get_text(GTK_LABEL(tab->number)), g_list_length(awb.tabs));
  gtk_label_set_text(GTK_LABEL(awb.gui.statusbar.tabs), tabs_label);
  gtk_label_set_text(GTK_LABEL(awb.gui.statusbar.uri), webkit_web_view_get_uri(WEBKIT_WEB_VIEW(tab->view)));

  gtk_widget_show(tab->view);
}

Tab *awb_new_tab(const char *uri)
{
  Tab *tab = (Tab *)calloc(1, sizeof(Tab));
  awb.tabs = g_list_append(awb.tabs, tab);
  tab->link = g_list_last(awb.tabs);
  gui_new_tab(tab);

  awb_open_uri(uri);

  return tab;
}

void awb_close_tab(Tab *tab)
{
  if(!tab)
    tab = awb.current_tab;

  if(!tab->link->prev && !tab->link->next)
    return;

  gui_close_tab(tab);

  awb.tabs = g_list_remove(awb.tabs, tab);
}

void awb_history_back(void)
{
  if(!webkit_web_view_can_go_back(WEBKIT_WEB_VIEW(awb.gui.view)))
    return;

  WebKitWebBackForwardList *bfList = webkit_web_view_get_back_forward_list(WEBKIT_WEB_VIEW(awb.gui.view));
  WebKitWebHistoryItem *item = webkit_web_back_forward_list_get_back_item(bfList);

  if(item) {
    /*awb_open_uri(webkit_web_history_item_get_original_uri(item));
 */   webkit_web_view_go_back(WEBKIT_WEB_VIEW(awb.gui.view));
  }
}

void awb_history_forward(void)
{
  if(!webkit_web_view_can_go_forward(WEBKIT_WEB_VIEW(awb.gui.view)))
    return;

  WebKitWebBackForwardList *bfList = webkit_web_view_get_back_forward_list(WEBKIT_WEB_VIEW(awb.gui.view));
  WebKitWebHistoryItem *item = webkit_web_back_forward_list_get_forward_item(bfList);

  if(item) {
    /*awb_open_uri(webkit_web_history_item_get_original_uri(item));
   */ webkit_web_view_go_forward(WEBKIT_WEB_VIEW(awb.gui.view));
  }
}

void awb_toggle_inspector(void)
{
  fprintf(stderr, "inspector\n");
  WebKitWebSettings *settings = webkit_web_view_get_settings(WEBKIT_WEB_VIEW(awb.gui.view));
  g_object_set(G_OBJECT(settings), "enable-developer-extras", true, NULL);

  webkit_web_view_reload(WEBKIT_WEB_VIEW(awb.gui.view));

  WebKitWebInspector *inspector = webkit_web_view_get_inspector(WEBKIT_WEB_VIEW(awb.gui.view));
  awb.gui.inbox = GTK_WIDGET(inspector);
  webkit_web_inspector_show(inspector);
  gtk_widget_show_all(awb.gui.inbox);
}

void awb_free(void)
{

}

int awb_init(void)
{
  awb.tabs = NULL;
  awb.current_tab = NULL;
  awb.mode = MODE_NORMAL;

  /* init sqlite */
  if(!(awb.sqldb = db_open("./data.db"))) {
    printf("failed to open database");
    fflush(stdout);
  }

  /*adblock_init("easylist");*/
  gui_init();

  /* init suppe */
  SoupSession *soup = webkit_get_default_session();
  /*g_signal_connect(soup, "request-started", G_CALLBACK(awb_request_started_cb), NULL);*/
  g_object_set(soup, SOUP_SESSION_SSL_USE_SYSTEM_CA_FILE, true, NULL),
  g_object_set(soup, SOUP_SESSION_SSL_STRICT, false, NULL);

  awb_open_uri("https://duckduckgo.com");

  return 0;
}

int main(int argc, char **argv)
{
  gtk_init(&argc, &argv);

  awb_init();

  gtk_main();

  awb_free();

  return 0;
}
