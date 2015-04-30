#include <gtk/gtk.h>

#include "split.h"

struct _SplitAppWindow {

	GtkApplicationWindow parent;
};

struct _SplitAppWindowClass {

	GtkApplicationWindowClass parent_class;
};

#include "types.h"

G_DEFINE_TYPE_WITH_PRIVATE(SplitAppWindow, split_app_window, GTK_TYPE_APPLICATION_WINDOW);

static void split_app_window_init(SplitAppWindow *app) {}

SplitAppWindow *split_app_window_new(SplitApp *app) {

	return g_object_new(SPLIT_APP_WINDOW_TYPE, "application", app, NULL);
}

void split_app_window_open(SplitAppWindow *win, GFile *file) {

	gtk_window_set_title(GTK_WINDOW(win), NAME);
	SplitAppWindowPrivate *data = split_app_window_get_instance_private(win);
	time(&(data->rawtime));
	data->widgets = malloc(sizeof(widgets_data));
	data->widgets->window = GTK_WIDGET(win);
	data->album = calloc(1, sizeof(album_data));
	char *filename = g_file_get_path(file);

	if (parse_cue(filename, data)) return;
	if (chdir(dirname(filename))) return;

	gtk_window_set_resizable(GTK_WINDOW(win), FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(win), 10);

	GtkWidget *box, *table, *bottom, *separator, *songlist, *title_label, *artist_label;
	GtkAdjustment *adj;

	// table

	table = gtk_table_new (2, 2, FALSE);

	title_label = gtk_label_new("Title");
	artist_label = gtk_label_new("Artist");

	gtk_misc_set_alignment(GTK_MISC(title_label), 1, 0.5);
	gtk_misc_set_alignment(GTK_MISC(artist_label), 1, 0.5);

	gtk_table_set_row_spacings(GTK_TABLE(table), 2);
	gtk_table_set_col_spacings(GTK_TABLE(table), 4);

	data->widgets->title_entry = gtk_entry_new();
	data->widgets->artist_entry = gtk_entry_new();

	if (data->album->title) {
		gtk_entry_set_text(GTK_ENTRY(data->widgets->title_entry), data->album->title);
		free(data->album->title);
	}

	if (data->album->artist) {
		gtk_entry_set_text(GTK_ENTRY(data->widgets->artist_entry), data->album->artist);
		free(data->album->artist);
	}

	gtk_table_attach(GTK_TABLE(table), data->widgets->artist_entry, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_table_attach(GTK_TABLE(table), data->widgets->title_entry, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_table_attach(GTK_TABLE(table), artist_label, 0, 1, 0, 1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_table_attach(GTK_TABLE(table), title_label, 0, 1, 1, 2, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	// songlist

	songlist = gtk_tree_view_new_with_model(GTK_TREE_MODEL(data->album->songs));
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(songlist), -1, "#", renderer, "text", 0, NULL);
	g_object_set(renderer, "editable", TRUE, NULL);
	g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK(song_edited), gtk_tree_view_get_model(GTK_TREE_VIEW(songlist)));
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(songlist), -1, "Title", renderer, "text", 1, NULL);

	// button

	data->widgets->button = gtk_button_new_with_mnemonic("_Split");
	g_signal_connect(data->widgets->button, "clicked", G_CALLBACK(button_clicked), data);

	// cover

	data->widgets->cover = gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(data->widgets->cover), gtk_image_new_from_icon_name(GTK_STOCK_SELECT_COLOR, GTK_ICON_SIZE_BUTTON));
	g_signal_connect(data->widgets->cover, "clicked", G_CALLBACK(enter_cover), data);

	// combobox

	data->widgets->combobox = gtk_combo_box_text_new();
	FILE *fin = fopen(GENRES_FILE, "r");

	if (!fin) {
		fprintf(stderr, "Error opening genres file, diaccana!\n");
		return;
	}

	int i = 0, j = 0;
	char line[SIZE];
	char set = 0;

	while (fgets(line, SIZE, fin)) {
		*strchr(line, '\n') = '\0';
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(data->widgets->combobox), NULL, line);

		if (data->album->genre && !strcmp(line, data->album->genre)) {
			set = 1;
			gtk_combo_box_set_active(GTK_COMBO_BOX(data->widgets->combobox), i);
		}

		if (!strcmp(DEFAULT_GENRE, line)) j = i;
		i++;
	}

	if (!set) gtk_combo_box_set_active(GTK_COMBO_BOX(data->widgets->combobox), j);
	if (data->album->genre) free(data->album->genre);
	fclose(fin);

	// spin_button

	adj = gtk_adjustment_new(data->album->year ? atoi(data->album->year) : DEFAULT_YEAR, DEFAULT_YEAR, localtime(&(data->rawtime))->tm_year + 1900, 1, 1, 0);
	data->widgets->spin_button = gtk_spin_button_new(adj, 1, 0);

	if (data->album->year) free(data->album->year);

	// offset

	data->widgets->offset = gtk_spin_button_new_with_range(0, 100, 1);

	// containers

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	bottom = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);

	// controls

	data->widgets->controls = gtk_table_new(1, 3, FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(data->widgets->controls), 10);
	gtk_table_attach(GTK_TABLE(data->widgets->controls), data->widgets->button, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_table_attach(GTK_TABLE(data->widgets->controls), data->widgets->spinner, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
	gtk_table_attach(GTK_TABLE(data->widgets->controls), data->widgets->cover, 2, 3, 0, 1, 
			(GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);

	// add

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	gtk_container_add(GTK_CONTAINER(bottom), data->widgets->combobox);
	gtk_container_add(GTK_CONTAINER(bottom), data->widgets->spin_button);
	gtk_container_add(GTK_CONTAINER(bottom), data->widgets->offset);
	gtk_container_add(GTK_CONTAINER(box), table);
	gtk_container_add(GTK_CONTAINER(box), songlist);
	gtk_container_add(GTK_CONTAINER(box), bottom);
	gtk_container_add(GTK_CONTAINER(box), data->widgets->progress);
	gtk_container_add(GTK_CONTAINER(box), separator);
	gtk_container_add(GTK_CONTAINER(box), data->widgets->controls);
	gtk_container_add(GTK_CONTAINER(win), box);
	gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
	gtk_widget_grab_focus(data->widgets->button);
	gtk_widget_show_all(GTK_WIDGET(win));
}

static void split_app_window_dispose(GObject *object)
{
	//SplitAppWindow *win = SPLIT_APP_WINDOW(object);
	//SplitAppWindowPrivate *data = split_app_window_get_instance_private(win);
	//gtk_widget_destroy(data->widgets->window);
	G_OBJECT_CLASS (split_app_window_parent_class)->dispose(object);
}

static void split_app_window_class_init(SplitAppWindowClass *class) {

	G_OBJECT_CLASS (class)->dispose = split_app_window_dispose;
}
