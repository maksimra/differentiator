#include <stdio.h>

#include "include/parser.hpp"
#include "include/check_args.hpp"

const int MAX_N_VARS = 10;

const int necessary_n_args = 2;

int main (const int argc, const char* argv[])
{
    DifError dif_error = DIF_NO_ERROR;
    ArgsError args_error = ARGS_NO_ERROR;

    FILE* log_file = fopen ("log_file.txt", "w");

    if (log_file == NULL)
        fprintf (stderr, "log_file.txt wasn't open.\n");

    dif_set_log_file  (log_file); // pass NULL if you don't want
    args_set_log_file (log_file); // to write to the log file

    args_error = args_check (argc, argv, necessary_n_args);
    if (args_print_if_error (args_error))
    {
        return EXIT_FAILURE;
    }

    const char* name_of_express_file = argv[1];
    FILE* source = fopen (name_of_express_file, "r");
    if (source == NULL)
    {
        fprintf (stderr, "source file wasn't open.\n");
        return EXIT_FAILURE;
    }

    FILE* text_tree = fopen ("text_tree.txt", "w");
    FILE* g_viz = fopen ("graphviz.txt", "w");
    if (text_tree == NULL || g_viz == NULL)
        dif_error = DIF_ERROR_FOPEN;

    dif_print_error (dif_error);

    size_t size = 0;
    char* expression = NULL;
    dif_error = read_file (source, name_of_express_file, &expression, &size);
    dif_print_error (dif_error);
    fclose (source);

    Tokens* tok = (Tokens*) calloc (size, sizeof (Tokens));
    if (tok == NULL)
    {
        dif_print_error (DIF_ERROR_CALLOC);
        return EXIT_FAILURE;
    }

    Vars vars[MAX_N_VARS] = {};
    dif_error = token (tok, vars, expression, MAX_N_VARS);
    dif_print_error (dif_error);

    Node* root = parse (&dif_error, tok);
    if (dif_error != DIF_NO_ERROR)
    {
        if (root != NULL)
            tree_dtor (root);
        free (tok);
        return 0;
    }

    graphviz (root, g_viz, vars);
    print_tree_txt_incr_tabs (root, text_tree);

    dif_error = taylor (root);
    dif_print_error (dif_error);

    tree_dtor (root);
    free (tok);
    fclose (text_tree);
    return EXIT_SUCCESS;
}
