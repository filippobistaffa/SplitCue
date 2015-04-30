#ifndef __TYPES_H
#define __TYPES_H

typedef struct {

	char *artist;
	char *title;
	char *genre;
	char *cover;
	char *year;
	char *file;
	GtkListStore *songs;
} album_data;

typedef struct {

	GtkWidget *artist_entry;
	GtkWidget *title_entry;
	GtkWidget *spin_button;
	GtkWidget *combobox;
	GtkWidget *progress;
	GtkWidget *controls;
	GtkWidget *spinner;
	GtkWidget *window;
	GtkWidget *button;
	GtkWidget *offset;
	GtkWidget *cover;
} widgets_data;

typedef struct _SplitAppWindowPrivate SplitAppWindowPrivate;

struct _SplitAppWindowPrivate {

	widgets_data *widgets;
	album_data *album;
	time_t rawtime;
};

#endif
