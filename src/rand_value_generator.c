#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "rand_value_generator.h"

void
rand_value_generator_init (RandValueGenerator **generator)
{
  srand (time (NULL));

  *generator = (RandValueGenerator*) malloc (sizeof (RandValueGenerator));
  (*generator)->value.dbl_val = 0;
}

void
rand_value_generator_release (RandValueGenerator **generator)
{
  free (*generator);
}

int
rand_value_generator_get_next_int (RandValueGenerator *generator,
                                   int                 min_bound,
                                   int                 max_bound)
{
  generator->value.dbl_val = 0;
  generator->value.int_val = rand () % max_bound + min_bound;

  return generator->value.int_val;
}

double
rand_value_generator_get_next_double (RandValueGenerator *generator,
                                      double              min_bound,
                                      double              max_bound)
{
  generator->value.dbl_val = fmod ((double)rand (), max_bound) + min_bound;

  return generator->value.dbl_val;
}
