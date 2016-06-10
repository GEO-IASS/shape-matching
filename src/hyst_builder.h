#ifndef HYST_BUILDER_H
#define HYST_BUILDER_H

#include <gtk/gtk.h>

enum {HYST_TYPE_ORIENT};

typedef struct hystogram
{
  double *hyst;
  int size;
} Hystogram;

void init_hystogram             (Hystogram **hyst);
void release_hystogram          (Hystogram **hyst);
void build_hystogram_from_image (Hystogram  *hyst,
                                 GdkPixbuf  *image,
                                 int         type);

#endif // HYST_BUILDER_H
