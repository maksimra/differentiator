#include "diff.h"

int main (const int argc, const char* argv[])
{
    const char* NAME = argv[1];
    struct Vars VARS[10] = {};
    enum DifError error = DIF_NO_ERROR;


    FILE* source = fopen (NAME, "r");
    FILE* log_file = fopen ("log_file.txt", "w");
    FILE* text_tree = fopen ("text_tree.txt", "w");
    FILE* g_viz = fopen ("graphviz.txt", "w");

    if (log_file == NULL)
        printf ("Error opening the log file.\n");

    dif_set_log_file (log_file);


    if (source == NULL || text_tree == NULL || g_viz == NULL || log_file == NULL)
        error = DIF_ERROR_FOPEN;

    dif_print_error (error);


    error = check_argc (argc, 2);
    dif_print_error (error);


    int size = 0;
    error = read_file (NAME, &size);
    dif_print_error (error);


    struct Tokens* TOK = (struct Tokens*) calloc ((size_t) size, sizeof (struct Tokens));

    if (TOK == NULL)
        error = DIF_ERROR_CALLOC;

    dif_print_error (error);


    error = token (TOK, VARS);
    dif_print_error (error);


    int n_tok = 0;
    Node* root = get_g (&error, TOK, &n_tok);

    if (error != DIF_NO_ERROR)
    {
        if (root != NULL)
            tree_dtor (root);

        return 0;
    }


    error = graphviz (root, g_viz, VARS);


    int n_space = 0;
    dif_tree_print_txt (root, text_tree, &n_space);


    taylor (root, &error);
    dif_print_error (error);


    free (TOK);
    fclose (text_tree);
    fclose (source);
    return 0;
}
