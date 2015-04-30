#include "shntool.h"
#include "split.h"

void song_edited(GtkCellRendererText *cell, char *id, char *value, GtkListStore *songs) {

	GtkTreeIter iter;
	GtkTreePath *path = gtk_tree_path_new_from_string(id);
	gtk_tree_model_get_iter(GTK_TREE_MODEL(songs), &iter, path);
	gtk_tree_path_free(path);
	gtk_list_store_set(songs, &iter, 1, value, -1);
}

void *thread_split(void *thread_data) {

	SplitAppWindowPrivate *data = thread_data;
	globals_init(EXE, data);
	modules_init();
	st_ops.output_format = find_format(FORMAT);
	process_file(data->album->file);

	//copy_songs(data);
	GtkWidget *icon1 = gtk_image_new_from_icon_name(OK_ICON, GTK_ICON_SIZE_BUTTON);
	GtkWidget *icon2 = gtk_image_new_from_icon_name(GTK_STOCK_SELECT_COLOR, GTK_ICON_SIZE_BUTTON);
	gdk_threads_enter();
	gtk_spinner_stop(GTK_SPINNER(data->widgets->spinner))
	//gtk_widget_hide(data->widgets->spinner);
	gtk_widget_destroy(data->widgets->spinner);
	gtk_table_attach(GTK_TABLE(data->widgets->controls), icon1, 0, 1, 0, 1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
	gtk_widget_show(icon1);
	gtk_widget_destroy(data->widgets->cover);
	gtk_table_attach(GTK_TABLE(data->widgets->controls), icon2, 2, 3, 0, 1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
	gtk_widget_show(icon2);
	gtk_button_set_label(GTK_BUTTON(data->widgets->button), "Done");
	//gtk_widget_set_sensitive(data->widgets->button, TRUE);
	//g_signal_connect(data->widgets->button, "clicked", G_CALLBACK(free_all), data);
	gdk_threads_leave();
	puts("DONE!");

	pthread_exit(NULL);
}

void button_clicked(GtkWidget *widget, SplitAppWindowPrivate *data) {

	//g_signal_handler_disconnect(widget, data->button_handler);
	gtk_spinner_start(GTK_SPINNER(data->widgets->spinner));
	gtk_widget_set_sensitive(data->widgets->button, FALSE);
	gtk_button_set_label(GTK_BUTTON(data->widgets->button), "Splitting...");
	pthread_t thread;
	pthread_create(&thread, NULL, thread_split, data);
}

void select_file(GtkWidget *widget, SplitAppWindowPrivate *data) {

	GtkWidget *dialog = gtk_file_chooser_dialog_new("Select Source File", GTK_WINDOW(data->widgets->window),
						        GTK_FILE_CHOOSER_ACTION_OPEN,
						        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						        GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		data->album->file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		//gtk_widget_hide(widget);
		gtk_widget_destroy(widget);
		data->widgets->spinner = gtk_spinner_new();
		gtk_table_attach(GTK_TABLE(data->widgets->controls), data->widgets->spinner, 0, 1, 0, 1,
				 (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
		gtk_widget_show(data->widgets->spinner);
	}

	gtk_widget_destroy(dialog);
}

void enter_cover(GtkWidget *widget, SplitAppWindowPrivate *data) {

	GtkWidget *entry = gtk_entry_new();
	if (data->album->cover) gtk_entry_set_text (GTK_ENTRY(entry), data->album->cover);
	GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(data->widgets->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_OTHER, GTK_BUTTONS_OK, "Enter cover art URL:");
	gtk_message_dialog_set_image(GTK_MESSAGE_DIALOG(dialog), gtk_image_new_from_icon_name(GTK_STOCK_SELECT_COLOR, GTK_ICON_SIZE_BUTTON));
	gtk_container_add(GTK_CONTAINER(gtk_message_dialog_get_message_area(GTK_MESSAGE_DIALOG(dialog))), entry);
	gtk_widget_show_all(gtk_message_dialog_get_message_area(GTK_MESSAGE_DIALOG(dialog)));
	gtk_dialog_run(GTK_DIALOG(dialog));
	data->album->cover = realloc(data->album->cover, gtk_entry_get_text_length(GTK_ENTRY(entry)));
	strcpy(data->album->cover, gtk_entry_get_text(GTK_ENTRY(entry)));
	gtk_widget_destroy(dialog);
}

#define UPC(c) ((c) = (isalpha(c) ? toupper(c) : c))
#define LOC(c) ((c) = (isalpha(c) ? tolower(c) : c))

void capitalise(char *s, size_t n) {

	UPC(*s);
	s++;

	while (--n) {
		if (*(s - 1) == ' ' || *(s - 1) == '(' || *(s - 1) == '-') UPC(*s);
		else LOC(*s);
		s++;
	}
}

char *get_info(char *s, char *tag, int eol) {

	char *r;
	int l, i = 0;
	if (*(s + strlen(tag)) == '\"') i++;
	l = MIN(SIZE, strlen(s) - strlen(tag) - 2 * i - eol);
	r = calloc(l + 1, 1);
	strncpy(r, s + strlen(tag) + i, l);
	capitalise(r, l);
	return r;
}

int parse_cue(char *filename, SplitAppWindowPrivate *data) {

	char *l, *song;
	char line[SIZE];
	FILE *fin, *fout, *ftest;
	fin = fopen(filename, "r");
	fout = fopen(TEMP_CUE, "w");

	if (!fin) {
		fprintf(stderr, "Error opening input CUE file, diaccana!\n");
		return 1;
	}

	if (!fout) {
		fprintf(stderr, "Error opening temp CUE file, diaccana!\n");
		return 1;
	}

	int j = 0, i = 1;
	fseek(fin, -2, SEEK_END);
	int eol = fgetc(fin) == 13 ? 2 : 1;
	rewind(fin);

	GtkTreeIter iter;
	char firstindex = 1, firstsong = 1;
	data->album->songs = gtk_list_store_new(2, G_TYPE_INT, G_TYPE_STRING);

	while (fgets(line, SIZE, fin)) {
		if (begins_with(line, ARTIST_TAG)) data->album->artist = get_info(line, ARTIST_TAG, eol);
		if (begins_with(line, GENRE_TAG)) data->album->genre = get_info(line, GENRE_TAG, eol);
		if (begins_with(line, TITLE_TAG)) data->album->title = get_info(line, TITLE_TAG, eol);
		if (begins_with(line, DATE_TAG)) data->album->year = get_info(line, DATE_TAG, eol);
		if (begins_with(line, FILE_TAG)) {
			song = get_info(line, FILE_TAG, eol + 5);
			if ((l = strrchr(filename, '/'))) j = l - filename + 1;
			data->album->file = calloc(strlen(song) + j + 1, 1);
			if (l) strncpy(data->album->file, filename, j);
			strcat(data->album->file, song);
			free(song);

			if ((ftest = fopen(data->album->file, "r"))) {
				fclose(ftest);
				data->widgets->spinner = gtk_spinner_new();
			} else {
				free(data->album->file);
				data->widgets->spinner = gtk_button_new();
				gtk_button_set_image(GTK_BUTTON(data->widgets->spinner), gtk_image_new_from_icon_name(GTK_STOCK_SELECT_COLOR, GTK_ICON_SIZE_BUTTON));
				gtk_button_set_image(GTK_BUTTON(data->widgets->spinner),
						     gtk_image_new_from_icon_name(GTK_STOCK_OPEN, GTK_ICON_SIZE_BUTTON));
				g_signal_connect(data->widgets->spinner, "clicked", G_CALLBACK(select_file), data);
			}
		}
		if (begins_with(line, SONG_TAG)) {
			song = get_info(line, SONG_TAG, eol);
			gtk_list_store_append(data->album->songs, &iter);
			gtk_list_store_set(data->album->songs, &iter, 0, i++, 1, song, -1);
			free(song);
		}

		if (strstr(line, "PERFORMER") || strstr(line, "TITLE") || strstr(line, "FILE") || strstr(line, "TRACK") || strstr(line, "INDEX")) {
			if (strstr(line, "INDEX") && firstsong) { if (firstindex) { fputs("    INDEX 00 00:00:00\n", fout); firstindex = 0; } }
			else fputs(line, fout);
			if (strstr(line, "TRACK") && !firstindex) firstsong = 0;
		}
	}

	if (!data->album->genre) {
		data->album->genre = malloc(strlen(DEFAULT_GENRE) + 1);
		strcpy(data->album->genre, DEFAULT_GENRE);
	}

	data->widgets->progress = gtk_progress_bar_new();
	gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(data->widgets->progress), 1.0 / (i - 1));
	fclose(fout);
	fclose(fin);

	return 0;
}

int main(int argc, char *argv[]) {

	return g_application_run(G_APPLICATION(split_app_new()), argc, argv);
}
