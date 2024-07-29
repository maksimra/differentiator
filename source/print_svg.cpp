#include "../include/diff.hpp"

DifError graphviz (const Node* node, FILE* file, const Vars* VARS)
{
    assert (node != NULL);
    assert (file != NULL);

    print_start (file);

    print_connections (node, file, VARS);

    print_end (file);
    fclose (file);

    const char* cmd = "dot graphviz.txt -Tsvg -otree.svg";
    system (cmd);
    return DIF_NO_ERROR;
}

void print_start (FILE* file)
{
    fprintf (file, "graph G {\n");
}

void print_connections (const Node* node, FILE* file, const Vars* VARS)
{
    draw_left (node, file, VARS);
    draw_right (node, file, VARS);
}

void print_end (FILE* file)
{
    fprintf (file, "}");
}

void draw_right (const Node* node, FILE* file, const Vars* VARS)
{
    const char* color = NULL;
    assert (file != NULL);
    if (node->right != NULL)
    {
        if (node->right->left == NULL && node->right->right == NULL)
            color = "red";
        else
            color = "green";
        if (node->right->type == OPER)
        {
            fprintf (file, "{\"%s\n%p\"--\"%s\n%p\"[color = \"%s\"]};\n",
            get_oper_name (node->value.oper), node, get_oper_name (node->right->value.oper), node->right, color);
        }
        else if (node->right->type == NUM)
        {
            fprintf (file, "{\"%s\n%p\"--\"%.3lf\n%p\"[color = \"%s\"]};\n",
            get_oper_name (node->value.oper), node, node->right->value.number, node->right, color);
        }
        else if (node->right->type == VAR)
        {
            fprintf (file, "{\"%s\n%p\"--\"%.*s\n%p\"[color = \"%s\"]};\n",
            get_oper_name (node->value.oper), node, (int) VARS[node->right->value.n_var].len, VARS[node->right->value.n_var].name, node->right, color);
        }

        print_connections (node->right, file, VARS);
    }
}

void draw_left (const Node* node, FILE* file, const Vars* VARS)
{
    const char* color = NULL;
    if (node->left != NULL)
    {
        if (node->left->left == NULL && node->left->right == NULL)
            color = "red"; // sprintf and snprintf
        else
            color = "green";
        if (node->left->type == OPER)
        {
            fprintf (file, "{\"%s\n%p\"--\"%s\n%p\"[color = \"%s\"]};\n",
            get_oper_name (node->value.oper), node, get_oper_name (node->left->value.oper), node->left, color);
        }
        else if (node->left->type == NUM)
        {
            fprintf (file, "{\"%s\n%p\"--\"%.3lf\n%p\"[color = \"%s\"]};\n",
            get_oper_name (node->value.oper), node, node->left->value.number, node->left, color);
        }
        else if (node->left->type == VAR)
        {
            fprintf (file, "{\"%s\n%p\"--\"%.*s\n%p\"[color = \"%s\"]};\n",
            get_oper_name (node->value.oper), node, (int) VARS[node->left->value.n_var].len, VARS[node->left->value.n_var].name, node->left, color);
        }
        print_connections (node->left, file, VARS);
    }
}
