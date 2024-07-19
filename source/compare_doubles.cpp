#include "../include/comp_doub.h"

static const double ACCURACY = 1e-5;

bool compare_doubles (double a, double b)
{
    if (fabs (a - b) < ACCURACY)
        return true;

    return false;
}
