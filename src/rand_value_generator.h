#ifndef RAND_VALUE_GENERATOR_H
#define RAND_VALUE_GENERATOR_H

typedef struct rand_value_generator
{
  union rand_value
  {
    int int_val;
    double dbl_val;
  } value;
} RandValueGenerator;

void   rand_value_generator_init            (RandValueGenerator **generator);
void   rand_value_generator_release         (RandValueGenerator **generator);
int    rand_value_generator_get_next_int    (RandValueGenerator  *generator,
                                             int                  min_bound,
                                             int                  max_bound);
double rand_value_generator_get_next_double (RandValueGenerator  *generator,
                                             double               min_bound,
                                             double               max_bound);

#endif // RAND_VALUE_GENERATOR_H
