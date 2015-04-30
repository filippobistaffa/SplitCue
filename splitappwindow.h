#ifndef __SPLITAPPWIN_H
#define __SPLITAPPWIN_H

#include <gtk/gtk.h>
#include "splitapp.h"

#define SPLIT_APP_WINDOW_TYPE (split_app_window_get_type())
#define SPLIT_APP_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SPLIT_APP_WINDOW_TYPE, SplitAppWindow))

typedef struct _SplitAppWindow SplitAppWindow;
typedef struct _SplitAppWindowClass SplitAppWindowClass;

GType split_app_window_get_type(void);
SplitAppWindow *split_app_window_new(SplitApp *app);
void split_app_window_open(SplitAppWindow *win, GFile *file);

#endif /* __SPLITAPPWIN_H */
