#include "include/parser.hpp"
#include "include/check_args.hpp"

const int MAX_N_VARS = 10;

const int necessary_n_args = 2;

int main (const int argc, const char* argv[])
{
    DifError dif_error = DIF_NO_ERROR;
    ArgsError args_error = args_check (argc, argv, necessary_n_args);

    if (args_error != ARGS_NO_ERROR)
    {
        args_print_error (args_error);
        return 0;
    }

    const char* name_of_express_file = argv[1];
    Vars vars[MAX_N_VARS] = {};

    FILE* source = fopen (name_of_express_file, "r");
    FILE* log_file = fopen ("log_file.txt", "w");
    FILE* text_tree = fopen ("text_tree.txt", "w");
    FILE* g_viz = fopen ("graphviz.txt", "w");

    if (log_file == NULL)
        printf ("Error opening the log file.\n");

    dif_set_log_file (log_file);

    if (text_tree == NULL || g_viz == NULL || source == NULL)
        dif_error = DIF_ERROR_FOPEN;

    dif_print_error (dif_error);

    size_t size = 0;
    char* expression = NULL;
    dif_error = read_file (source, name_of_express_file, &expression, &size);
    dif_print_error (dif_error);
    fclose (source);

    Tokens* tok = (Tokens*) calloc (size, sizeof (Tokens));

    if (tok == NULL)
        dif_error = DIF_ERROR_CALLOC;

    dif_print_error (dif_error);

    dif_error = token (tok, vars, expression, MAX_N_VARS);
    dif_print_error (dif_error);

    int n_tok = 0;
    Node* root = parse (&dif_error, tok, &n_tok);

    if (dif_error != DIF_NO_ERROR)
    {
        if (root != NULL)
            tree_dtor (root);
        free (tok);
        return 0;
    }

    dif_error = graphviz (root, g_viz, vars);

    int n_space = 0;
    print_tree_txt_incr_tabs (root, text_tree, &n_space);

    taylor (root, &dif_error);

    tree_dtor (root);

    free (tok);
    fclose (text_tree);
    return 0;
}
