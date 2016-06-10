#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "database.h"
#include "global_definitions.h"
#include "imgproc.h"
#include "hyst_handler.h"

#define D_BASE_CAPACITY 10

struct _stored_image_info {
  char      *name;
  GdkPixbuf *image;
  Hystogram *hyst;
};

void
database_init (DataBase **d_base)
{
  *d_base = (DataBase*) malloc (sizeof (DataBase));
  (*d_base)->data = (StoredImageInfo*) malloc (sizeof (StoredImageInfo) * D_BASE_CAPACITY);
  (*d_base)->n_of_images = 0;
  (*d_base)->capacity = D_BASE_CAPACITY;
}

void
database_release (DataBase **d_base)
{
  int n_of_images;
  StoredImageInfo *image_info;

  if ((n_of_images = (*d_base)->n_of_images) > 0)
    {
      for (int i = 0; i < n_of_images; ++i)
        {
          image_info = (*d_base)->data + i;
          free (image_info->name);
          g_object_unref (image_info->image);
          release_hystogram (&image_info->hyst);
        }
    }
  free ((*d_base)->data);
  free (*d_base);
}

void
database_add (DataBase *d_base, GdkPixbuf *image, const char *name)
{
  int n_of_images;
  StoredImageInfo *new_image_info, *next_free_position;
  int name_len;

  new_image_info = (StoredImageInfo*) malloc (sizeof (StoredImageInfo));

  if ((n_of_images = d_base->n_of_images) < d_base->capacity)
    {
      /* setup name */
      if (name != NULL)
        {
          name_len = strlen (name);
          new_image_info->name = (char*) malloc (name_len + 1);
          memcpy (new_image_info->name, name, name_len + 1);
        }
      else
        {
          new_image_info->name = NULL;
        }

      /* setup image */
      new_image_info->image = gdk_pixbuf_copy (image);

      /* setup hystogram */
      init_hystogram (&new_image_info->hyst);
      build_hystogram_from_image (new_image_info->hyst, image, HYST_TYPE_ORIENT);

      /* add element to array */
      next_free_position = d_base->data + n_of_images;
      memcpy (next_free_position, new_image_info, sizeof (StoredImageInfo));

      /* increase number of elements */
      d_base->n_of_images ++;
    }

  free (new_image_info);
}

void
database_clear (DataBase *d_base)
{
  int n_of_images;
  StoredImageInfo *image_info;

  if ((n_of_images = d_base->n_of_images) > 0)
    {
      for (int i = 0; i < n_of_images; ++i)
        {
          image_info = d_base->data + i;
          free (image_info->name);
          g_object_unref (image_info->image);
          release_hystogram (&image_info->hyst);
        }
      d_base->n_of_images = 0;
    }
}

char*
database_get_image_name_by_index (DataBase *d_base,
                                  int       index)
{
  int n_of_images;

  g_assert ((n_of_images = d_base->n_of_images) > 0 && index < n_of_images);

  return d_base->data[index].name;
}

GdkPixbuf*
database_get_image_pbuf_by_index (DataBase  *d_base,
                                  int        index)
{
  int n_of_images;

  g_assert ((n_of_images = d_base->n_of_images) > 0 && index < n_of_images);

  return d_base->data[index].image;
}

Hystogram*
database_get_image_hyst_by_index (DataBase  *d_base,
                                  int        index)
{
  int n_of_images;

  g_assert ((n_of_images = d_base->n_of_images) > 0 && index < n_of_images);

  return d_base->data[index].hyst;
}

int
database_create_from_folder (DataBase *d_base,
                             char     *path)
{
  GDir *img_folder;
  const gchar *next_file;
  gchar full_name[MAX_PATH_LENGTH];

  g_assert (d_base != NULL && path != NULL);

  if (d_base->n_of_images > 0)
    database_clear (d_base);

  img_folder = g_dir_open (path, 0, NULL);

  if (img_folder == NULL)
    return -1;

  while ((next_file = g_dir_read_name (img_folder)) != NULL)
    {
      GdkPixbuf *image;
      GError *error;

      error = NULL;
      sprintf (full_name, "%s/%s", path, next_file);

      if (g_file_test (full_name, G_FILE_TEST_IS_REGULAR))
        {

          image = gdk_pixbuf_new_from_file (full_name, &error);

          if (error != NULL)
            {
              database_clear (d_base);
              return -1;
            }

          database_add (d_base, image, next_file);

          g_object_unref (image);
        }
    }

  g_dir_close (img_folder);

  return 0;
}

GSList*
database_find_similar (DataBase    *d_base,
                       GdkPixbuf   *image,
                       DistanceFunc func)
{
  Hystogram *input;
  int n_of_images;
  GSList *indeces_list;

  g_assert ((n_of_images = d_base->n_of_images) > 0);

  init_hystogram (&input);
  build_hystogram_from_image (input, image, HYST_TYPE_ORIENT);

  indeces_list = NULL;

  for (int i = 0; i < n_of_images; ++i)
    {
      Hystogram *current_image_hyst;
      double distance;

      current_image_hyst = database_get_image_hyst_by_index (d_base, i);

      distance =  hystogram_get_distance_between_2_hysts (input,
                                                          current_image_hyst);

      if (distance < DISTANCE_THRESHOLD)
        indeces_list = g_slist_prepend (indeces_list, GUINT_TO_POINTER (i));
    }

  return indeces_list;
}
