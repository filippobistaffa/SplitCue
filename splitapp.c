#include <gtk/gtk.h>

#include "splitapp.h"
#include "splitappwindow.h"

struct _SplitApp {

	GtkApplication parent;
};

struct _SplitAppClass {

	GtkApplicationClass parent_class;
};

G_DEFINE_TYPE(SplitApp, split_app, GTK_TYPE_APPLICATION);

static void split_app_init(SplitApp *app) {}

static void split_app_activate(GApplication *app) {

	SplitAppWindow *win;
	win = split_app_window_new(SPLIT_APP(app));
	gtk_window_present(GTK_WINDOW(win));
}

static void split_app_open(GApplication *app, GFile **files, gint n_files, const gchar *hint) {

	GList *windows;
	SplitAppWindow *win;
	int i;

	windows = gtk_application_get_windows(GTK_APPLICATION(app));
	if (windows)
		win = SPLIT_APP_WINDOW(windows->data);
	else
		win = split_app_window_new(SPLIT_APP(app));

	for (i = 0; i < n_files; i++)
		split_app_window_open(win, files[i]);

	gtk_window_present(GTK_WINDOW(win));
}

static void split_app_class_init(SplitAppClass *class) {

	G_APPLICATION_CLASS (class)->activate = split_app_activate;
	G_APPLICATION_CLASS (class)->open = split_app_open;
}

SplitApp *split_app_new(void) {

	return g_object_new(SPLIT_APP_TYPE, "application-id", "org.gtk.SplitApp", "flags", G_APPLICATION_HANDLES_OPEN | G_APPLICATION_NON_UNIQUE, NULL);
}
