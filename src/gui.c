#include "gui.h"
#include "imgproc.h"
#include "global_definitions.h"
#include "database.h"
#include <string.h>
#include <stdlib.h>

#define MAX_FOUND 10

#define CLEAR_POINTER_ARRAY(array, size)\
{\
  for (int _ind = 0; _ind < size; ++_ind)\
    array[_ind] = NULL;\
}

static GtkBuilder *builder;
static const char *dbase_view_title = "База изображений";
static const char *found_view_title = "Найденные изображения";
static DataBase *image_database;
static GdkPixbuf **found_images;

static void
show_message_box(GtkBuilder *builder,
                 const gchar *msg,
                 GtkMessageType type)
{
  GtkWidget *parent, *dialog;

  parent = GTK_WIDGET (gtk_builder_get_object (builder,
                                               "mainwindow"));
  dialog = gtk_message_dialog_new (GTK_WINDOW(parent),
                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                   type,
                                   GTK_BUTTONS_CLOSE,
                                   msg, NULL);
  gtk_dialog_run (GTK_DIALOG (dialog));

  gtk_widget_destroy (dialog);
}


static void tree_view_append_record (GtkTreeView *t_view,
                                     char        *name);

static void
tree_view_update_records_from_dbase (GtkTreeView *t_view,
                                     DataBase    *d_base)
{
  int n_of_images;
  char *image_name;

  g_assert (d_base != NULL);

  n_of_images = d_base->n_of_images;
  for (int i = 0; i < n_of_images; ++i)
    {
      image_name = database_get_image_name_by_index (d_base, i);
      tree_view_append_record (t_view, image_name);
    }
}

static void
tree_view_clear_records (GtkTreeView *t_view, const char *msg)
{
  GtkTreeStore *store;
  GtkTreeIter root;

  store = GTK_TREE_STORE(gtk_tree_view_get_model(t_view));
  gtk_tree_store_clear(store);
  gtk_tree_store_append(store, &root, NULL);
  gtk_tree_store_set(store, &root, 0, msg, -1);
}

static void
tree_view_append_record (GtkTreeView *t_view, char *name)
{
  GtkTreeStore *store;
  GtkTreeIter root;
  GtkTreeIter new_elem_iter;

  store = GTK_TREE_STORE(gtk_tree_view_get_model(t_view));
  gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(store), &root, "0");
  gtk_tree_store_append(store, &new_elem_iter, &root);
  gtk_tree_store_set(store, &new_elem_iter, 0, name, -1);
}

static void
get_images_folder_path (GtkBuilder *builder,
                        char       *path)
{
  GtkWidget *parent;
  GtkWidget *dialog;
  gchar *folder;
  int response;

  parent = GTK_WIDGET (gtk_builder_get_object (builder, "mainwindow"));

  dialog = gtk_file_chooser_dialog_new("Выберите папку с изображениями",
                                       GTK_WINDOW(parent),
                                       GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                       "OK", GTK_RESPONSE_ACCEPT,
                                       "Отмена", GTK_RESPONSE_CANCEL,
                                       NULL);

  response = gtk_dialog_run(GTK_DIALOG(dialog));

  if(response == GTK_RESPONSE_ACCEPT)
    {
      folder = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
      strcpy (path, folder);
      g_free (folder);
    }

  gtk_widget_destroy(dialog);
}

static void
on_record_activated (GtkTreeView       *t_view,
                     GtkTreePath       *path,
                     GtkTreeViewColumn *column,
                     gpointer           data)
{
  int path_depth, record_index;
  int *indeces;
  GtkBuilder *builder;
  GtkImage *image;
  GdkPixbuf *record_image;

  indeces = gtk_tree_path_get_indices_with_depth (path, &path_depth);
  if (path_depth == 2)
    {
      record_index = indeces[path_depth - 1];
      builder = GTK_BUILDER(data);
      image = GTK_IMAGE(gtk_builder_get_object(builder, "image"));
      record_image = database_get_image_pbuf_by_index (image_database,
                                                          record_index);
      gtk_image_set_from_pixbuf (image, record_image);
    }
}

static void
on_found_activated (GtkTreeView *t_view,
                    GtkTreePath *path,
                    GtkTreeViewColumn *column,
                    gpointer data)
{
  int path_depth, found_index;
  int *indeces;
  GtkBuilder *builder;
  GtkImage *image;
  GdkPixbuf *record_image;

  indeces = gtk_tree_path_get_indices_with_depth (path, &path_depth);
  if (path_depth == 2)
    {
      found_index = indeces[path_depth - 1];

      g_assert (found_images[found_index] != NULL);

      builder = GTK_BUILDER(data);
      image = GTK_IMAGE(gtk_builder_get_object(builder, "image"));
      record_image = found_images[found_index];
      gtk_image_set_from_pixbuf (image, record_image);
    }
}

static void
on_open_image (GtkFileChooserButton *button,
               gpointer               data)
{
  gchar *file_name;
  GtkImage *image;
  GtkBuilder *builder;

  builder = GTK_BUILDER (data);
  image = GTK_IMAGE (gtk_builder_get_object (builder, "image"));
  file_name = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(button));
  gtk_image_set_from_file (image, file_name);

  g_free (file_name);
}

static void
on_find_image (GtkButton *buttton,
               gpointer   data)
{
  GtkBuilder *builder;
  GtkImage *image;
  GdkPixbuf *p_image;
  GtkTreeView *found_view;
  GSList *found_indeces;

  builder = GTK_BUILDER (data);

  if (image_database->n_of_images == 0)
    {
      show_message_box (builder, "ошибка: база не содержит изображений", GTK_MESSAGE_ERROR);
    }
  else
    {
      GSList *found_index;
      int count;
      char message[MAX_MESSAGE_LENGTH];

      image = GTK_IMAGE (gtk_builder_get_object (builder, "image"));
      p_image = gtk_image_get_pixbuf (image);

      found_view = GTK_TREE_VIEW (gtk_builder_get_object (builder, "treeview2"));
      tree_view_clear_records (found_view, found_view_title);

      CLEAR_POINTER_ARRAY (found_images, MAX_FOUND);

      found_indeces = database_find_similar (image_database, p_image, NULL);
      count = 0;
      for (found_index = found_indeces; found_index; found_index = found_index->next)
        {
          int cur_index;
          char *found_name;

          cur_index = GPOINTER_TO_UINT (found_index->data);

          found_name = database_get_image_name_by_index (image_database, cur_index);
          tree_view_append_record (found_view, found_name);

          found_images[count++] = database_get_image_pbuf_by_index (image_database,
                                                                    cur_index);
        }

      gtk_tree_view_expand_all (found_view);

      sprintf (message, "Найдено %d совпадений в базе", count);
      show_message_box (builder, message, GTK_MESSAGE_INFO);
    }
}

static void
on_load_images (GSimpleAction *action,
                GVariant      *variant,
                gpointer       data)
{
  char images_path[MAX_PATH_LENGTH];
  GtkBuilder *builder;
  GtkTreeView *dbase_view;
  int status;

  builder = GTK_BUILDER (data);
  dbase_view = GTK_TREE_VIEW (gtk_builder_get_object (builder, "treeview1"));

  get_images_folder_path (builder, images_path);

  status = database_create_from_folder (image_database, images_path);
  if (status < 0)
    {
      show_message_box (builder, "ошибка: не удалось обновить базу", GTK_MESSAGE_ERROR);
      return;
    }

  tree_view_clear_records (dbase_view, dbase_view_title);
  tree_view_update_records_from_dbase (dbase_view, image_database);
  gtk_tree_view_expand_all (dbase_view);
}

static void
on_clear_database (GSimpleAction *action,
                   GVariant      *variant,
                   gpointer       data)
{
  GtkBuilder *builder;
  GtkTreeView *dbase_view;
  GtkTreeView *found_view;

  database_clear (image_database);

  builder = GTK_BUILDER (data);
  dbase_view = GTK_TREE_VIEW (gtk_builder_get_object (builder, "treeview1"));
  tree_view_clear_records (dbase_view, dbase_view_title);

  found_view = GTK_TREE_VIEW (gtk_builder_get_object (builder, "treeview2"));
  tree_view_clear_records (found_view, found_view_title);
}

static void
on_put_noise (GSimpleAction *action,
              GVariant      *variant,
              gpointer       data)
{
  GtkBuilder *builder;
  GtkImage *image;
  GdkPixbuf *p_image, *p_image_noised;

  builder = GTK_BUILDER (data);
  image = GTK_IMAGE (gtk_builder_get_object (builder, "image"));

  if (gtk_image_get_storage_type (image) != GTK_IMAGE_EMPTY)
    {
      p_image = gtk_image_get_pixbuf (image);
      p_image_noised = get_noised_image_from_image (p_image);
      gtk_image_set_from_pixbuf (image, p_image_noised);

      g_object_unref (p_image_noised);
    }
}

static GActionEntry builder_entries[] =
{
  {"load_images",  on_load_images,    NULL, NULL, NULL},
  {"clear_dbase",  on_clear_database, NULL, NULL, NULL},
  {"put_noise",    on_put_noise,      NULL, NULL, NULL}
};

static void
add_action_entries (GtkBuilder *builder)
{
  GtkApplicationWindow *window;
  GtkImage *image;

  image = GTK_IMAGE (gtk_builder_get_object (builder, "image"));

  window = GTK_APPLICATION_WINDOW (gtk_builder_get_object (builder,
                                                           "mainwindow"));

  g_action_map_add_action_entries(G_ACTION_MAP(window),
                                  builder_entries,
                                  G_N_ELEMENTS(builder_entries),
                                  builder);
}

static void
setup_menu (GtkBuilder *builder)
{
  GtkWidget *menu_button;
  GMenu *menu;

  menu_button = GTK_WIDGET (gtk_builder_get_object (builder,
                                                    "menubutton"));
  menu = g_menu_new ();
  g_menu_append (menu, "Загрузить изображения в базу", "win.load_images");
  g_menu_append (menu, "Очистить базу", "win.clear_dbase");
  g_menu_append (menu, "Добавить искажение", "win.put_noise");
  gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (menu_button),
                                  G_MENU_MODEL (menu));
}

static void
setup_tree_view (GtkBuilder *builder)
{
  GtkTreeIter iter;
  GtkTreeStore *treestore;
  GtkWidget *treeview;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;


  /* treeview1 */
  treeview = GTK_WIDGET (gtk_builder_get_object (builder, "treeview1"));
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (NULL, renderer, "text", 0, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

  treestore = gtk_tree_store_new (1, G_TYPE_STRING);
  gtk_tree_store_append (treestore, &iter, NULL);
  gtk_tree_store_set (treestore, &iter, 0, dbase_view_title, -1);
  gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (treestore));
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);
  g_signal_connect (treeview, "row-activated", G_CALLBACK(on_record_activated), builder);


  /* treeview2 */
  treeview = GTK_WIDGET (gtk_builder_get_object (builder, "treeview2"));
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (NULL, renderer, "text", 0, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

  treestore = gtk_tree_store_new (1, G_TYPE_STRING);
  gtk_tree_store_append (treestore, &iter, NULL);
  gtk_tree_store_set (treestore, &iter, 0, found_view_title, -1);
  gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (treestore));
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);
  g_signal_connect (treeview, "row-activated", G_CALLBACK(on_found_activated), builder);
}

static void
setup_buttons_signals (GtkBuilder *builder)
{
  GtkWidget *open_button, *find_button;

  open_button = GTK_WIDGET (gtk_builder_get_object (builder, "open"));
  g_signal_connect (GTK_FILE_CHOOSER_BUTTON (open_button),
                    "file-set",
                    G_CALLBACK (on_open_image),
                    builder);

  find_button = GTK_WIDGET (gtk_builder_get_object (builder, "find"));
  g_signal_connect (GTK_BUTTON (find_button),
                    "clicked",
                    G_CALLBACK (on_find_image),
                    builder);
}

void
on_startup (GtkApplication *app,
            gpointer        data)
{
  builder = gtk_builder_new_from_file (ui_path);
  database_init (&image_database);
  found_images = (GdkPixbuf**) malloc (MAX_FOUND *
                                       sizeof (GdkPixbuf*));
  CLEAR_POINTER_ARRAY (found_images, MAX_FOUND);

  add_action_entries (builder);
  setup_menu (builder);
  setup_buttons_signals (builder);
  setup_tree_view (builder);
}

void
on_shutdown (GtkApplication *app,
             gpointer        data)
{
  g_object_unref (builder);
  database_release (&image_database);
  free (found_images);
}

void
on_activate (GtkApplication *app,
             gpointer        data)
{
  GtkWidget *window;

  window = GTK_WIDGET (gtk_builder_get_object (builder, "mainwindow"));

  gtk_application_add_window (app, GTK_WINDOW(window));

  gtk_widget_show_all (window);
}
