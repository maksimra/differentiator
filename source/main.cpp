#include "diff.h"

const int MAX_N_VARS = 10;

int main (const int argc, const char* argv[])
{
    enum DifError error = check_args (argc, argv);

    if (error != DIF_NO_ERROR)
    {
        dif_print_error (error);
        return 0;
    }

    const char* NAME = argv[1];
    struct Vars VARS[MAX_N_VARS] = {};

    FILE* source = fopen (NAME, "r");
    FILE* log_file = fopen ("log_file.txt", "w");
    FILE* text_tree = fopen ("text_tree.txt", "w");
    FILE* g_viz = fopen ("graphviz.txt", "w");

    if (log_file == NULL)
        printf ("Error opening the log file.\n");

    dif_set_log_file (log_file);

    if (text_tree == NULL || g_viz == NULL || log_file == NULL)
        error = DIF_ERROR_FOPEN;

    dif_print_error (error);

    size_t size = 0;
    error = read_file (NAME, &size);
    dif_print_error (error);

    struct Tokens* TOK = (struct Tokens*) calloc (size, sizeof (struct Tokens));

    if (TOK == NULL)
        error = DIF_ERROR_CALLOC;

    dif_print_error (error);

    error = token (TOK, VARS, MAX_N_VARS);
    dif_print_error (error);

    int n_tok = 0;
    Node* root = get_g (&error, TOK, &n_tok);

    if (error != DIF_NO_ERROR)
    {
        if (root != NULL)
            tree_dtor (root);
        free (TOK);
        return 0;
    }

    error = graphviz (root, g_viz, VARS);

    int n_space = 0;
    print_tree_txt_incr_tabs (root, text_tree, &n_space);

    taylor (root, &error);

    free (TOK);
    fclose (text_tree);
    fclose (source);
    return 0;
}
