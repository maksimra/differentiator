#include <math.h>
#include <stdbool.h>

static const double ACCURACY = 1e-5;

bool compare_doubles (double a, double b);

bool compare_doubles (double a, double b)
{
    if (fabs (a - b) < ACCURACY)
        return true;

    return false;
}
