#include "shntool.h"

static void destroy()
{
	gtk_main_quit();
}

static gboolean free_all(GtkWidget *window, GdkEventKey *e, prog_data *data)
{	
	gtk_widget_destroy(window);
	
	//if (album->artist) free(data->album->artist);
	//if (album->title) free(data->album->title);
	//if (album->genre) free(data->album->genre);
	//if (album->year) free(data->album->year);
	
	if (data->album->file) free(data->album->file);
	g_object_unref(data->album->songs);
	
	free(data->widgets);
	free(data->album);
	free(data);
	
	return FALSE;
}

static void song_edited(GtkCellRendererText *cell, char *id, char *value, GtkListStore *songs)
{
	GtkTreeIter iter;
	GtkTreePath *path = gtk_tree_path_new_from_string(id);
	gtk_tree_model_get_iter(GTK_TREE_MODEL(songs), &iter, path);
	gtk_tree_path_free(path);
	gtk_list_store_set(songs, &iter, 1, value, -1);
}

/*

int copy_songs(prog_data *data)
{
	const char *artist = gtk_entry_get_text(GTK_ENTRY(data->widgets->artist_entry));
	const char *title = gtk_entry_get_text(GTK_ENTRY(data->widgets->title_entry));	
	char *genre = gtk_combo_box_get_active_text(GTK_COMBO_BOX(data->widgets->combobox));
	int year = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data->widgets->spin_button));
		
	char *path = malloc(strlen(DESTINATION) + 
			    strlen(artist) * 2 +
			    strlen(title) +
			    13); // " - YEAR - " + / + / + \0 
	
	sprintf(path, "%s%s/%s - %i - %s/", DESTINATION, artist, artist, year, title);
	mkdir(path, S_IRWXU);
	
	GtkTreeIter iter;
	gboolean valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(data->album->songs), &iter);
	
	char *song, *infile, *outfile, *buffer;
	FILE *fin, *fout;
	int n, size;
	
	while (valid)
	{
		gtk_tree_model_get(GTK_TREE_MODEL(data->album->songs), &iter, 0, &n, 1, &song, -1);
		
		outfile = malloc(strlen(path) +
				 strlen(artist) +
				 strlen(song) +
				 strlen(FORMAT) +
				 10); // " - NN - " + "." + \0
		
		infile = malloc(strlen(SPLIT_DIR) + 
				strlen(SPLIT_PREFIX) + 
				strlen(FORMAT) + 
				4); // "NN" + "." + \0
		
		sprintf(infile, "%s%s%02d.%s", SPLIT_DIR, SPLIT_PREFIX, n, FORMAT);
		sprintf(outfile, "%s%s - %02d - %s.%s", path, artist, n, song, FORMAT);
		
		fout = fopen(outfile, "wb");
		fin = fopen(infile, "rb");
		
		fseek(fin, 0, SEEK_END);
		size = ftell(fin);
		buffer = malloc(size);		
		rewind(fin);
		fread(buffer, 1, size, fin);
		fwrite(buffer, 1, size, fout);
		
		fclose(fout);
		fclose(fin);
		remove(infile);
		
		free(outfile);
		free(infile);
		free(song);
		
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(data->album->songs), &iter);
	}
	
	g_free(genre);
	free(path);
	
	return 0;
}

*/

void *thread_split(void *thread_data)
{
	prog_data *data = thread_data;
	globals_init(EXE, data);
	modules_init();	
	st_ops.output_format = find_format(FORMAT);
	process_file(data->album->file);
	
	//copy_songs(data);
	GtkWidget *icon = gtk_image_new_from_icon_name(OK_ICON, GTK_ICON_SIZE_BUTTON);
	gdk_threads_enter();
	gtk_spinner_stop(GTK_SPINNER(data->widgets->spinner));	
	//gtk_widget_hide(data->widgets->spinner);
	gtk_widget_destroy(data->widgets->spinner);
	gtk_table_attach(GTK_TABLE(data->widgets->controls), icon, 0, 1, 0, 1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
	gtk_widget_show(icon);
	gtk_button_set_label(GTK_BUTTON(data->widgets->button), "Done");
	//gtk_widget_set_sensitive(data->widgets->button, TRUE);
	//g_signal_connect(data->widgets->button, "clicked", G_CALLBACK(free_all), data);
	gdk_threads_leave();
	puts("DONE!");
	
	pthread_exit(NULL);
}

static void button_clicked(GtkWidget *widget, prog_data *data)
{
	//g_signal_handler_disconnect(widget, data->button_handler);
	gtk_spinner_start(GTK_SPINNER(data->widgets->spinner));
	gtk_widget_set_sensitive(data->widgets->button, FALSE);
	gtk_button_set_label(GTK_BUTTON(data->widgets->button), "Splitting...");
	pthread_t thread;	
	pthread_create(&thread, NULL, thread_split, data);
}

static void select_file(GtkWidget *widget, prog_data *data)
{
	GtkWidget *dialog = gtk_file_chooser_dialog_new("Select Source File", NULL,
						        GTK_FILE_CHOOSER_ACTION_OPEN,
						        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						        GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
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

int begins_with(char *s, char *tag)
{
	return (s == strstr(s, tag));
}

char *get_info(char *s, char *tag, int eol)
{	
	char *r;
	int l, i = 0;
	
	if (*(s + strlen(tag)) == '\"') i++;
	l = MIN(SIZE, strlen(s) - strlen(tag) - 2 * i - eol);
	r = calloc(l + 1, 1);
	strncpy(r, s + strlen(tag) + i, l);
	return r;
}

int parse_cue(char *filename, prog_data *data)
{
	char *l, *song;
	char line[SIZE];
	FILE *fin, *fout, *ftest;	
	fin = fopen(filename, "r");
	fout = fopen(TEMP_CUE, "w");
	
	if (!fin) 
	{
		fprintf(stderr, "Error opening input CUE file, diaccana!\n");
		return 1;
	}
	
	if (!fout) 
	{
		fprintf(stderr, "Error opening temp CUE file, diaccana!\n");
		return 1;
	}
	
	int j = 0, i = 1;
	fseek(fin, -2, SEEK_END);
	int eol = fgetc(fin) == 13 ? 2 : 1;
	rewind(fin);
	
	GtkTreeIter iter;
	data->album->songs = gtk_list_store_new(2, G_TYPE_INT, G_TYPE_STRING);
	
	while (fgets(line, SIZE, fin))
	{
		if (begins_with(line, ARTIST_TAG)) data->album->artist = get_info(line, ARTIST_TAG, eol);
		if (begins_with(line, GENRE_TAG)) data->album->genre = get_info(line, GENRE_TAG, eol);
		if (begins_with(line, TITLE_TAG)) data->album->title = get_info(line, TITLE_TAG, eol);
		if (begins_with(line, DATE_TAG)) data->album->year = get_info(line, DATE_TAG, eol);
		if (begins_with(line, FILE_TAG)) 
		{
			song = get_info(line, FILE_TAG, eol + 5);
			if ((l = strrchr(filename, '/'))) j = l - filename + 1;	
			data->album->file = calloc(strlen(song) + j + 1, 1);
			if (l) strncpy(data->album->file, filename, j);
			strcat(data->album->file, song);
			free(song);
			
			if ((ftest = fopen(data->album->file, "r"))) 
			{
				fclose(ftest);
				data->widgets->spinner = gtk_spinner_new();
			}
			else
			{
				free(data->album->file);
				data->widgets->spinner = gtk_button_new();
				gtk_button_set_image(GTK_BUTTON(data->widgets->spinner), 
						     gtk_image_new_from_icon_name(GTK_STOCK_OPEN, GTK_ICON_SIZE_BUTTON));
				g_signal_connect(data->widgets->spinner, "clicked", G_CALLBACK(select_file), data);
			}
		}
		if (begins_with(line, SONG_TAG)) 
		{
			song = get_info(line, SONG_TAG, eol);
			gtk_list_store_append(data->album->songs, &iter);
			gtk_list_store_set(data->album->songs, &iter, 0, i++, 1, song, -1);
			free(song);
		}
		
		if (strstr(line, "PERFORMER") || strstr(line, "TITLE") || strstr(line, "FILE") || strstr(line, "TRACK") || strstr(line, "INDEX"))
		if (!strstr(line, "00:00:32"))
		fputs((const char *)line, fout);
	}
	
	if (!data->album->genre)
	{
		data->album->genre = malloc(strlen(DEFAULT_GENRE) + 1);
		strcpy(data->album->genre, DEFAULT_GENRE);
	}
	
	gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(data->widgets->progress), 1.0 / (i - 1));
	fclose(fout);
	fclose(fin);
	
	return 0;
}

int main(int argc, char *argv[])
{
	// progress

	g_type_init();
	gdk_threads_init();
	
	prog_data *data = malloc(sizeof(prog_data));
	data->widgets = malloc(sizeof(widgets_data));
	data->album = calloc(1, sizeof(album_data));
	//data->album->genre = NULL;
	//data->album->year = NULL;
	data->widgets->progress = gtk_progress_bar_new();
	
	if (parse_cue(argv[1], data)) return 1;
	
	time_t rawtime;
	time(&rawtime);
	
	gtk_init(&argc, &argv);	
	g_set_application_name(NAME);
	gtk_window_set_default_icon_name(ICON);
	
	GtkWidget *window, *box, *table, *bottom, *separator, *songlist, *title_label, *artist_label;
	GtkAdjustment *adj;
	
	// window
	
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(window), 10);
	
	g_signal_connect(window, "delete-event", G_CALLBACK(free_all), data);
	g_signal_connect(window, "destroy", G_CALLBACK (destroy), NULL);
	
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
	
	if (data->album->title) 
	{
		gtk_entry_set_text(GTK_ENTRY(data->widgets->title_entry), data->album->title);
		free(data->album->title);
	}
	if (data->album->artist) 
	{
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
	
	// combobox
	
	data->widgets->combobox = gtk_combo_box_text_new();
	FILE *fin = fopen(GENRES_FILE, "r");
	
	if (!fin) 
	{
		fprintf(stderr, "Error opening genres file, diaccana!\n");
		return 1;
	}
	
	int i = 0, j;
	char line[SIZE];
	bool set = FALSE;
	
	while (fgets(line, SIZE, fin))
	{
		*strchr(line, '\n') = '\0';
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(data->widgets->combobox), NULL, line);
		
		if (data->album->genre && !strcmp(line, data->album->genre)) {
			set = TRUE;
			gtk_combo_box_set_active(GTK_COMBO_BOX(data->widgets->combobox), i);
		}
		
		if (!strcmp(DEFAULT_GENRE, line)) j = i;		
		i++;
	}
	
	if (!set) gtk_combo_box_set_active(GTK_COMBO_BOX(data->widgets->combobox), j);
	if (data->album->genre) free(data->album->genre);
	fclose(fin);	
	
	// spin_button
	
	adj = gtk_adjustment_new(data->album->year ? atoi(data->album->year) : DEFAULT_YEAR, DEFAULT_YEAR, localtime(&rawtime)->tm_year + 1900, 1, 1, 0);
	data->widgets->spin_button = gtk_spin_button_new(adj, 1, 0);

	if (data->album->year) free(data->album->year);
	
	// offset
	
	data->widgets->offset = gtk_spin_button_new_with_range(0, 100, 1);
	
	// containers
	
	box = gtk_vbox_new(FALSE, 10);
	bottom = gtk_hbox_new(FALSE, 10);
	separator = gtk_hseparator_new();
	
	// controls
	
	data->widgets->controls = gtk_table_new(1, 2, FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(data->widgets->controls), 10);
	gtk_table_attach(GTK_TABLE(data->widgets->controls), data->widgets->button, 1, 2, 0, 1, 
			(GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_table_attach(GTK_TABLE(data->widgets->controls), data->widgets->spinner, 0, 1, 0, 1, 
			(GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
	
	// add
	box = gtk_vbox_new(FALSE, 10);
	gtk_container_add(GTK_CONTAINER(bottom), data->widgets->combobox);
	gtk_container_add(GTK_CONTAINER(bottom), data->widgets->spin_button);
	gtk_container_add(GTK_CONTAINER(bottom), data->widgets->offset);
	gtk_container_add(GTK_CONTAINER(box), table);	
	gtk_container_add(GTK_CONTAINER(box), songlist);	
	gtk_container_add(GTK_CONTAINER(box), bottom);
	gtk_container_add(GTK_CONTAINER(box), data->widgets->progress);
	gtk_container_add(GTK_CONTAINER(box), separator);
	gtk_container_add(GTK_CONTAINER(box), data->widgets->controls);
	gtk_container_add(GTK_CONTAINER(window), box);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_widget_grab_focus(data->widgets->button);
	
	// show
	
	gtk_widget_show_all(window);
	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();
	
	return 0;
}
