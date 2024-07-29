#include "../include/check_args.hpp"

#define PRINT_BEGIN() fprintf (log_file, "begin: %s.\n",      __PRETTY_FUNCTION__)
#define PRINT_END()   fprintf (log_file, "success end: %s\n", __PRETTY_FUNCTION__)
#define PRINT(...) if (log_file != NULL) fprintf (log_file, __VA_ARGS__)

static FILE* log_file = stderr;

void args_set_log_file (FILE* file)
{
    log_file = file;
}

ArgsError args_check (const int argc, const char** argv, int ref_argc)
{
    PRINT_BEGIN();
    if (argc != ref_argc)
        return ARGS_ERROR_ARGC;

    FILE* file = fopen (argv[1], "r"); // TODO: функция должна вместо этого
                                       // смотреть флаги. (не понял, спросить лично потом)
    if (file == NULL)
        return ARGS_ERROR_ARGV;

    fclose (file);
    PRINT_END();
    return ARGS_NO_ERROR;
}

bool args_print_if_error (ArgsError error)
{
    if (error != ARGS_NO_ERROR)
    {
        PRINT ("%s\n", args_get_error (error));
        return true;
    }
    return false;
}

const char* args_get_error (ArgsError error)
{
    switch (error)
    {
        case ARGS_NO_ERROR:
            return "Args: Ошибок в работе функций не выявлено.";
        case ARGS_ERROR_ARGC:
            return "Args: Введено некорректное число аргументов.";
        case ARGS_ERROR_ARGV:
            return "Argv: Файл не открылся (вторым аргументом должно быть имя файла).";
        default:
            return "Args: Нужной ошибки не найдено...";
    }
}
