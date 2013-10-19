#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>

#define SIZE 100
#define OK_ICON "gtk-ok"
#define ICON GTK_STOCK_CUT
#define NAME "Cue File Splitter"

#define DEFAULT_YEAR 1900.0
#define DEFAULT_GENRE "Heavy Metal"

#define ARTIST_TAG "PERFORMER "
#define GENRE_TAG "REM GENRE "
#define SONG_TAG "    TITLE "
#define DATE_TAG "REM DATE "
#define TITLE_TAG "TITLE "
#define FILE_TAG "FILE "

#define TEMP_CUE "/tmp/temp.cue"
#define SPLIT_PREFIX "split-track"
#define DESTINATION "/media/music"
#define GENRES_FILE "/home/filippo/Dropbox/apps/.genres"

#define FORMAT "flac"
#define EXE "splitcue"
#define PACKAGE "shntool"

#define TAG_NUMBER 6

typedef struct
{
	char *artist;
	char *title;
	char *genre;
	char *cover;
	char *year;
	char *file;
	GtkListStore *songs;
} album_data;

typedef struct
{
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

typedef struct
{
	widgets_data *widgets;
	album_data *album;
//	GtkListStore *genres;
} prog_data;
