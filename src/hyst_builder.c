#include "hyst_builder.h"
#include <stdlib.h>
#include "imgproc.h"
#include "global_definitions.h"
#include "hyst_handler.h"

enum ORIENT {NORTH = 0, NORTH_EAST, EAST, SOUTH_EAST, SOUTH, SOUTH_WEST, WEST, NORTH_WEST, NONE};

#define ORIENT_HYST_SIZE 8

static void
build_orient_hystogram (Hystogram *hyst,
                        GdkPixbuf *image);


void
init_hystogram (Hystogram **hyst)
{
  (*hyst) = (Hystogram*) malloc (sizeof (Hystogram));
  (*hyst)->hyst = NULL;
  (*hyst)->size = 0;
}

void
release_hystogram (Hystogram **hyst)
{
  if ((*hyst)->hyst != NULL)
    free ((*hyst)->hyst);
  free (*hyst);
}

void
build_hystogram_from_image (Hystogram *hyst,
                            GdkPixbuf *image,
                            int        type)
{
  build_orient_hystogram (hyst, image);
}

static int
check_orientation_clockwise (struct point p1,
                             struct point p2)
{
  int dx, dy;

  dx = p2.x - p1.x;
  dy = p2.y - p1.y;

  if (dx == 0 && dy == -1)
    return NORTH;
  else if (dx == 1 && dy == -1)
    return NORTH_EAST;
  else if (dx == 1 && dy == 0)
    return EAST;
  else if (dx == 1 && dy == 1)
    return SOUTH_EAST;
  else if (dx == 0 && dy == 1)
    return SOUTH;
  else if (dx == -1 && dy == 1)
    return SOUTH_WEST;
  else if (dx == -1 && dy == 0)
    return WEST;
  else if (dx == -1 && dy == -1)
    return NORTH_WEST;
  else
    return NONE;
}

static void
build_orient_hystogram (Hystogram *hyst,
                        GdkPixbuf *image)
{
  struct point *contour;
  int contour_size;
  int orientation;

  hyst->hyst = (double*) calloc (ORIENT_HYST_SIZE, sizeof (double));
  hyst->size = ORIENT_HYST_SIZE;

  contour = get_contour_points_from_image_with_size (image, &contour_size);

  for (int i = 1; i < contour_size; ++i)
    {
      orientation = check_orientation_clockwise (contour[i - 1],
                                                 contour[i]);
      hyst->hyst[orientation]++;
    }

  orientation = check_orientation_clockwise (contour[0],
                                             contour[contour_size - 1]);
  hyst->hyst[orientation]++;

  hystogram_normalize (hyst, hyst);

  free (contour);
}
