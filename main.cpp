#include "diff.h"

int main (const int argc, const char* argv[])
{
    const char* NAME = argv[1];
    enum DifError error = check_argc (argc, 2);
    dif_print_error (error);
    FILE* source = fopen (NAME, "r");
    FILE* output = fopen ("output.txt", "w");
    FILE* g_viz = fopen ("graphviz.txt", "w");
    int n_space = 0;

    if (source == NULL || output == NULL || g_viz == NULL)
        error = DIF_ERROR_FOPEN;

    dif_print_error (error);
    Node* root = NULL;
    int size = 0;

    error = read_file (NAME, &size);
    struct Tokens* TOK = (struct Tokens*) calloc (size, sizeof (struct Tokens));

    if (TOK == NULL)
        error = DIF_ERROR_CALLOC;

    dif_print_error (error);
    error = token (TOK); // доделать токинизацию
    dif_print_error (error);

    int n_tok = 0;
    root = get_g (&error, TOK, &n_tok);
    dif_print_error (error);
    dif_tree_print (root, output, &n_space);
    Node* new_root = simplification (root, &error);
    dif_print_error (error);

    //root = diff (root);
    error = graphviz (new_root, g_viz);
    dif_print_error (error);
    tree_dtor (root);
    tree_dtor (new_root);
    free (TOK);
    fclose (output);
    fclose (source);
    fclose (g_viz);
    return 0;
}
