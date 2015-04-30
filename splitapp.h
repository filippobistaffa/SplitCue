#ifndef __SPLITAPP_H
#define __SPLITAPP_H

#include <gtk/gtk.h>

#define SPLIT_APP_TYPE (split_app_get_type())
#define SPLIT_APP(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SPLIT_APP_TYPE, SplitApp))

typedef struct _SplitApp SplitApp;
typedef struct _SplitAppClass SplitAppClass;

GType split_app_get_type(void);
SplitApp *split_app_new(void);

#endif /* __SPLITAPP_H */

