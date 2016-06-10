#ifndef HYST_HANDLER_H
#define HYST_HANDLER_H

#include "hyst_builder.h"

void   hystogram_normalize                    (Hystogram *hyst_in,
                                               Hystogram *hyst_out);
double hystogram_get_distance_between_2_hysts (Hystogram *hyst1,
                                               Hystogram *hyst2);

#endif // HYST_HANDLER_H
