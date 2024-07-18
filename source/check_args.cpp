#include "diff.h"

const int necessary_n_args = 2;

enum DifError check_args (const int argc, const char* argv[])
{
    if (argc != necessary_n_args)
        return DIF_ERROR_ARGC;

    FILE* file = fopen (argv[1], "r");

    if (file == NULL)
        return DIF_ERROR_ARGV;

    fclose (file);
    return DIF_NO_ERROR;
}
