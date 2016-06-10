#ifndef DATABASE_H
#define DATABASE_H

#include <gtk/gtk.h>
#include "hyst_builder.h"

typedef double (*DistanceFunc) (void*, void*);

typedef struct _stored_image_info StoredImageInfo;

typedef struct _image_database
{
  StoredImageInfo *data;
  int              n_of_images;
  int              capacity;
} DataBase;

void       database_init                    (DataBase       **d_base);
void       database_release                 (DataBase       **d_base);
void       database_add                     (DataBase        *d_base,
                                             GdkPixbuf       *image,
                                             const char      *name);
void       database_clear                   (DataBase        *d_base);
char*      database_get_image_name_by_index (DataBase        *d_base,
                                             int              index);
GdkPixbuf* database_get_image_pbuf_by_index (DataBase        *d_base,
                                             int              index);
Hystogram* database_get_image_hyst_by_index (DataBase        *d_base,
                                             int              index);
int        database_create_from_folder      (DataBase        *d_base,
                                             char            *path);
GSList*    database_find_similar            (DataBase        *d_base,
                                             GdkPixbuf       *image,
                                             DistanceFunc     func);

#endif // DATABASE_H
