#include "hyst_handler.h"
#include <math.h>

void
hystogram_normalize (Hystogram *hyst_in,
                     Hystogram *hyst_out)
{
  double sum;

  g_assert (hyst_in->size == hyst_out->size);

  sum = 0;

  for (int i = 0; i < hyst_in->size; ++i)
    sum += hyst_in->hyst[i];

  for (int i = 0; i < hyst_out->size; ++i)
    hyst_out->hyst[i] = hyst_in->hyst[i] / sum;
}

double
hystogram_get_distance_between_2_hysts (Hystogram *hyst1,
                                        Hystogram *hyst2)
{
  double distance;

  g_assert (hyst1->size == hyst2->size);

  distance = 0;

  for (int i = 0; i < hyst1->size; ++i)
    distance += fabs (hyst1->hyst[i] - hyst2->hyst[i]);

  return distance;
}
