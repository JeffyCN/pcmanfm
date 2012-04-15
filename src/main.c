/*
 * Initial main.c file generated by Glade. Edit as required.
 * Glade will not overwrite this file.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <glib.h>

#include <stdlib.h>

/* socket is used to keep single instance */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "file-icon.h"

#include "main-window.h"
#include "folder-content.h"
#include "mime-description.h"

#include "glade-support.h"

#include "settings.h"

static char* init_path = NULL;

static int sock;
GIOChannel* io_channel = NULL;

static FMMainWindow* create_main_window();

static gboolean on_socket_event( GIOChannel* ioc,
                                 GIOCondition cond,
                                 gpointer data )
{
  int client, r;
  socklen_t addr_len = 0;
  struct sockaddr_un client_addr = {0};
  FMMainWindow* main_window;
  static char path[PATH_MAX];

  if( cond & G_IO_IN ) {
    client = accept(g_io_channel_unix_get_fd(ioc), &client_addr, &addr_len);
    if( client != -1 ) {
      r = read( client, path, PATH_MAX );
      if( r != -1 ) {
        /* upath = g_filename_to_utf8( path, -1, NULL, NULL, NULL ); */
        path[r] = '\0';
        main_window = create_main_window();
        fm_main_window_add_new_tab( main_window, path,
                                    appSettings.showSidePane,
                                    appSettings.sidePaneMode );

      }
      close( client );
    }
  }
  return TRUE;
}

static void single_instance_init()
{
  struct sockaddr_un addr;
  int addr_len;

  if( (sock = socket( AF_UNIX, SOCK_STREAM, 0 )) == -1)
    exit(1);

  addr.sun_family = AF_UNIX;
  sprintf(addr.sun_path, "/tmp/.pcmanfm-socket-%s", g_get_user_name() );
  addr_len = strlen(addr.sun_path) + sizeof(addr.sun_family);

  if (connect(sock, (struct sockaddr*)&addr, addr_len) != -1) {
    /* connected successfully */
    write( sock, init_path, strlen(init_path) );
    close( sock );
    gdk_notify_startup_complete();
    exit(0);
  }

  /* There is no existing server. So, we are in the first instance. */
  unlink(addr.sun_path); /* delete old socket file if it exists. */
  if (bind(sock, (struct sockaddr*)&addr, addr_len) == -1) {
    exit(1);
  }

  io_channel = g_io_channel_unix_new( sock );
  g_io_channel_set_encoding( io_channel, NULL, NULL );
  g_io_channel_set_buffered( io_channel, FALSE );

  g_io_add_watch( io_channel, G_IO_IN,
                  (GIOFunc)on_socket_event, NULL );

  if (listen(sock, 5) == -1) {
    exit(1);
  }
}

gboolean delayed_open_new_tab( FMMainWindow* mainWindow )
{
  fm_main_window_add_new_tab( mainWindow, init_path,
                              appSettings.showSidePane,
                              appSettings.sidePaneMode );
  return FALSE;
}

FMMainWindow* create_main_window()
{
  FMMainWindow* main_window = fm_main_window_new ();
  gtk_window_set_default_size( GTK_WINDOW(main_window),
                               appSettings.width, appSettings.height );
  gtk_widget_show ( GTK_WIDGET(main_window) );
  return main_window;
}

int
main (int argc, char *argv[])
{
  FMMainWindow* main_window;
  GtkSettings *settings;

  /* FIXME: If the argv[1] path is not encoded in UTF-8,
  we should do some conversion. */
  if( argc > 1 && *argv[1] )
    init_path = argv[1];
  else if( ! (init_path = g_get_home_dir()) )
    init_path = "/";

  load_settings();

  gtk_init (&argc, &argv);
  gtk_set_locale ();

  if( appSettings.singleInstance ) {
    single_instance_init();
  }

#ifdef ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  /* Use multithreading */
  g_thread_init(NULL);

  file_icon_init( appSettings.iconTheme );
  folder_content_init();
  mime_description_init();

  main_window = create_main_window();
  g_idle_add( (GSourceFunc)delayed_open_new_tab, main_window );

  gtk_main ();

  if( appSettings.singleInstance ) {
    g_io_channel_unref( io_channel );
    close( sock );
  }

  save_settings();
  free_settings();

  file_icon_clean();
  folder_content_clean();
  mime_description_clean();

  return 0;
}

