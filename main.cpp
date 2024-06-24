#include "diff.h"

int main (const int argc, const char* argv[])
{

    const char* NAME = argv[1];
    struct Vars VARS[10] = {};
    FILE* log_file = fopen ("log_file.txt", "w");
    FILE* source = fopen (NAME, "r");
    FILE* output = fopen ("output.txt", "w");
    FILE* g_viz = fopen ("graphviz.txt", "w");
    FILE* taylor = fopen ("taylor.txt", "w");

    dif_set_log_file (log_file);

    enum DifError error = DIF_NO_ERROR;

    if (source == NULL || output == NULL || g_viz == NULL || log_file == NULL)
        error = DIF_ERROR_FOPEN;

    dif_print_error (error);


    error = check_argc (argc, 2);
    int n_space = 0;


    dif_print_error (error);
    Node* root = NULL;
    int size = 0;

    error = read_file (NAME, &size);
    struct Tokens* TOK = (struct Tokens*) calloc ((size_t) size, sizeof (struct Tokens));

    if (TOK == NULL)
        error = DIF_ERROR_CALLOC;

    dif_print_error (error);
    error = token (TOK, VARS);
    dif_print_error (error);

    int n_tok = 0;
    root = get_g (&error, TOK, &n_tok);

    if (error != DIF_NO_ERROR)
    {
        if (root != NULL)
            tree_dtor (root);

        return 0;
    }

    error = graphviz (root, g_viz, VARS);
    //Node* simpl_root = simplification (root, &error);
    dif_tree_print (root, output, &n_space);
    dif_print_error (error);

    //taylr (simpl_root, taylor, &error);

    /*Node* dif_root = diff (simpl_root);
    error = graphviz (simpl_root, g_viz);
    dif_print_error (error);
    tree_dtor (dif_root);*/
    tree_dtor (root);
    free (TOK);
    fclose (output);
    fclose (source);
    return 0;
}
