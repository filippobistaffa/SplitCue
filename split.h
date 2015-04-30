#include <time.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
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
#define DESTINATION "/media/filippo/music"
#define GENRES_FILE "/home/filippo/Dropbox/apps/.genres"

#define FORMAT "flac"
#define EXE "splitcue"
#define PACKAGE "shntool"

#define TAG_NUMBER 6

#define begins_with(s, tag) (s == strstr(s, tag))

#include "splitapp.h"
#include "splitappwindow.h"
#include "types.h"

int parse_cue(char *filename, SplitAppWindowPrivate *data);
void song_edited(GtkCellRendererText *cell, char *id, char *value, GtkListStore *songs);
void button_clicked(GtkWidget *widget, SplitAppWindowPrivate *data);
void select_file(GtkWidget *widget, SplitAppWindowPrivate *data);
void enter_cover(GtkWidget *widget, SplitAppWindowPrivate *data);
