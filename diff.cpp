#include "diff.h"

char* s = NULL;

const int N_OPER = sizeof (OP) / sizeof (OP[0]);

const int OPER_N_SYMB = 10;

const int MAX_N_VARS = 10;

const int FOR_ANSW = 10;

const int YES_NO_LEN = 4;

const int N_OPERS = sizeof (OP) / sizeof (OP[0]);

static FILE* log_file = stderr;

enum DifError dif_set_log_file (FILE* file)
{
    if (file == NULL)
        return DIF_NULL_PTR_LOG;
    log_file = file;
    return DIF_NO_ERROR;
}

enum DifError check_argc (const int argc, int necessary_n_arg)
{
    if (argc != necessary_n_arg)
        return DIF_ERROR_ARGC;

    return DIF_NO_ERROR;
}

enum DifError read_file (const char* NAME, int* size)
{
    struct stat statbuf = {};

    FILE* file = fopen (NAME, "r");

    if (stat (NAME, &statbuf))
        return DIF_ERROR_STAT;

    s = (char*) calloc ((long unsigned int) statbuf.st_size + sizeof (char), sizeof (char));

    if (s == NULL)
        return DIF_ERROR_CALLOC;

    *size = (int) fread ((char*) s, sizeof (char), (size_t) statbuf.st_size, file);

    if (*size != statbuf.st_size)
        return DIF_ERROR_FREAD;

    fclose (file);

    return DIF_NO_ERROR;
}

int search_oper (const char* str, long len)
{
    int n_oper = 0;
    while (n_oper < N_OPERS)
    {
        if (strncmp (str, OP[n_oper].name, (size_t) len) == 0)
        {
            return n_oper;
        }
        n_oper += 1;
    }
    return -1;
}

int space_counter (char* line)
{
    int n_space = 0;
    while (isspace (*(line + n_space)))
    {
        n_space++;
    }
    return n_space;
}

void printf_str (FILE* file, Node* node, int n_space)
{
    if (node->type == OPER)
    {
        fprintf (file, "%*c(\"%s\"\n", n_space, ' ', OP[node->value.n_oper].name);
    }
    else if (node->type == VAR)
    {
        fprintf (file, "%*c(\"<%d>\"\n", n_space, ' ', node->value.n_var);
    }
    else
    {
        fprintf (file, "%*c(\"%.3lf\"\n", n_space, ' ', node->value.number);
    }
}

void printing_branches (Node* node, FILE* file, int* n_space)
{
    if (node != NULL)
    {
        dif_tree_print_txt (node, file, n_space);
    }
    else
    {
        fprintf (file, "_");
        (*n_space) -= 4;
    }
}

void dif_tree_print_txt (Node* node, FILE* file, int* n_space)
{
    printf_str (file, node, *n_space);
    (*n_space) += 4;

    printing_branches (node->left, file, n_space);
    printing_branches (node->right, file, n_space);

    fprintf (file, ")\n");
}

enum DifError new_node (Node** node)
{
    Node* temp = (Node*) calloc (1, sizeof (Node));
    if (temp == NULL)
        return DIF_ERROR_CALLOC;
    *node = temp;
    return DIF_NO_ERROR;
}

void tree_dtor (Node* root)
{
    assert (root != NULL);

    if (root->left != NULL)
        tree_dtor (root->left);

    if (root->right != NULL)
        tree_dtor (root->right);

    free (root);
}

enum DifError graphviz (Node* node, FILE* file, struct Vars* VARS)
{
    assert (node != NULL);
    assert (file != NULL);

    print_start (file);

    print_filling (node, file, VARS);

    print_end (file);
    fclose (file);
    system ("dot graphviz.txt -Tsvg -otree.svg");
    return DIF_NO_ERROR;
}

void print_start (FILE* file)
{
    fprintf (file, "graph G {\n");
}

void print_end (FILE* file)
{
    fprintf (file, "}");
}

void print_filling (Node* node, FILE* file, struct Vars* VARS)
{
    draw_left (node, file, VARS);
    draw_right (node, file, VARS);
}

void draw_right (Node* node, FILE* file, struct Vars* VARS)
{
    char* color = NULL;
    assert (file != NULL);
    if (node->right != NULL)
    {
        if (node->right->left == NULL && node->right->right == NULL)
            color = strdup ("red"); // sprintf and snprintf
        else
            color = strdup ("green");
        if (node->right->type == OPER)
        {
            fprintf (file, "{\"%s\n%p\"--\"%s\n%p\"[color = \"%s\"]};\n",
            OP[node->value.n_oper].name, node, OP[node->right->value.n_oper].name, node->right, color);
        }
        else if (node->right->type == NUM)
        {
            fprintf (file, "{\"%s\n%p\"--\"%.3lf\n%p\"[color = \"%s\"]};\n",
            OP[node->value.n_oper].name, node, node->right->value.number, node->right, color);
        }
        else if (node->right->type == VAR)
        {
            fprintf (file, "{\"%s\n%p\"--\"%.*s\n%p\"[color = \"%s\"]};\n",
            OP[node->value.n_oper].name, node, VARS[node->right->value.n_var].len, VARS[node->right->value.n_var].name, node->right, color);
        }

        print_filling (node->right, file, VARS);
    }
    free (color);
}

void draw_left (Node* node, FILE* file, struct Vars* VARS)
{
    char* color = NULL;
    if (node->left != NULL)
    {
        if (node->left->left == NULL && node->left->right == NULL)
            color = strdup ("red"); // sprintf and snprintf
        else
            color = strdup ("green");
        if (node->left->type == OPER)
        {
            fprintf (file, "{\"%s\n%p\"--\"%s\n%p\"[color = \"%s\"]};\n",
            OP[node->value.n_oper].name, node, OP[node->left->value.n_oper].name, node->left, color);
        }
        else if (node->left->type == NUM)
        {
            fprintf (file, "{\"%s\n%p\"--\"%.3lf\n%p\"[color = \"%s\"]};\n",
            OP[node->value.n_oper].name, node, node->left->value.number, node->left, color);
        }
        else if (node->left->type == VAR)
        {
            fprintf (file, "{\"%s\n%p\"--\"%.*s\n%p\"[color = \"%s\"]};\n",
            OP[node->value.n_oper].name, node, VARS[node->left->value.n_var].len, VARS[node->left->value.n_var].name, node->left, color);
        }
        print_filling (node->left, file, VARS);
    }
    free (color);
}

Node* create_node (enum Type type, double value, Node* left, Node* right)
{
    Node* new_node = (Node*) calloc (1, sizeof (Node));
    new_node->type = type;

    if (type == OPER)
        new_node->value.n_oper = (int) value;

    else if (type == NUM)
        new_node->value.number = value;

    else if (type == VAR)
        new_node->value.n_var = (int) value;

    new_node->left = left;
    new_node->right = right;

    return new_node;
}

Node* copy (const Node* node)
{
    Node* copy_node = (Node*) calloc (1, sizeof (Node));

    copy_node->type = node->type;

    if (node->type == NUM)
        copy_node->value.number = node->value.number;

    else if (node->type == VAR)
        copy_node->value.n_var = node->value.n_var;

    else if (node->type == OPER)
        copy_node->value.n_oper = node->value.n_oper;

    if (node->left != NULL)
        copy_node->left = copy (node->left);

    else
        copy_node->left = NULL;

    if (node->right != NULL)
        copy_node->right = copy (node->right);

    else
        copy_node->right = NULL;

    return copy_node;
}

void dif_print_error (enum DifError error)
{
    fprintf (log_file, "%s\n", dif_get_error (error));
}

const char* dif_get_error (enum DifError error)
{
    switch (error)
    {
        case DIF_NO_ERROR:
            return "Dif: Ошибок в работе функций не выявлено.";
        case DIF_ERROR_CALLOC:
            return "Dif: Ошибка работы функции calloc.";
        case DIF_ERROR_STAT:
            return "Dif: Ошибка в работе функции stat.";
        case DIF_ERROR_READ:
            return "Dif: Ошибка чтения выражения с файла.";
        case DIF_ERROR_FREAD:
            return "Dif: Ошибка в работе функции fread.";
        case DIF_NULL_PTR_LOG:
            return "Dif: Передан нулевой указатель на log_file.";
        case DIF_ERROR_STRTOD:
            return "Dif: Ошибка работы функции strtod.";
        case DIF_ERROR_ARGC:
            return "Dif: Введено некорректное число аргументов.";
        case DIF_ERROR_FOPEN:
            return "Dif: Ошибка открытия файла (fopen).";
        case DIF_FUNC_ERROR:
            return "Dif: Ошибка определения типа функции.";
        case DIF_SYNTAX_ERROR:
            return "Dif: Синтаксическая ошибка.";
        case DIF_DIV_NUL:
            return "Dif: Куда, блин, на нуль делить!";
        default:
            return "Dif: Куда делся мой enum ошибок?";
    }
}

Node* diff (const Node* node)
{
    switch (node->type)
    {
        case NUM:
            return create_node (NUM, 0, NULL, NULL);
        case VAR:
            return create_node (NUM, 1, NULL, NULL);
        case OPER:
            switch (OP[node->value.n_oper].op_enum)
            {
                case ADD:
                    return create_node (OPER, ADD, diff (node->left), diff (node->right));
                case SUB:
                    return create_node (OPER, SUB, diff (node->left), diff (node->right));
                case MUL:
                {
                    Node* du = diff (node->left); // define cl cr dl dr
                    Node* cu = copy (node->left);
                    Node* dv = diff (node->right);
                    Node* cv = copy (node->right);
                    Node* res = create_node (OPER, ADD,
                    create_node (OPER, MUL, du, cv),
                    create_node (OPER, MUL, cu, dv));
                    return res;
                }
                case SIN:
                {
                    Node* cu = copy (node->right);
                    Node* du = diff (node->right);
                    Node* res = create_node (OPER, MUL,
                    create_node (OPER, COS, NULL, cu), du);
                    return res;
                }
                case COS:
                {
                    Node* cu = copy (node->right);
                    Node* du = diff (node->right);
                    Node* res = create_node (OPER, MUL,
                                    create_node (OPER, MUL,
                                        create_node (OPER, SIN, NULL, cu),
                                        create_node (NUM, -1, NULL, NULL)),
                                    du);
                    return res;
                }
                case DIV:
                {
                    Node* cu = copy (node->left);
                    Node* cv = copy (node->right);
                    Node* du = diff (node->left);
                    Node* dv = diff (node->right);
                    Node* res = create_node (OPER, DIV,
                    create_node (OPER, SUB,
                    create_node (OPER, MUL, du, cv),
                    create_node (OPER, MUL, dv, cu)),
                    create_node (OPER, MUL, cv, cv));
                    return res;
                }
                case LN:
                {
                    Node* cu = copy (node->right);
                    Node* du = diff (node->right);
                    Node* res = create_node (OPER, DIV, du, cu);
                    return res;
                }
                case SQRT:
                {
                    Node* cu = copy (node);
                    Node* v = create_node (OPER, MUL, create_node (NUM, 2, NULL, NULL), cu);
                    Node* h = create_node (OPER, DIV, create_node (NUM, 1, NULL, NULL), v);
                    Node* dv = diff (node->right);
                    Node* res = create_node (OPER, MUL, dv, h);
                    return res;
                }
                case DEG:
                {
                    if (node->right->type == NUM && node->left->type == NUM)
                    {
                        return create_node (NUM, 0, NULL, NULL);
                    }
                    else if (node->right->type == NUM)
                    {
                        Node* cu = copy (node->left);
                        Node* du = diff (node->left);
                        Node* pre_res = create_node (OPER, MUL, create_node (NUM, node->right->value.number, NULL, NULL),
                        create_node (OPER, DEG, cu, create_node (NUM, node->right->value.number - 1, NULL, NULL)));
                        return create_node (OPER, MUL, pre_res, du);
                    }
                    else if (node->left->type == NUM)
                    {
                        Node* cu = copy (node);
                        Node* du = diff (node->right);
                        Node* ln_num = create_node (OPER, LN, NULL, node->left);
                        return create_node (OPER, MUL, cu,
                        create_node (OPER, MUL, du, ln_num));
                    }
                    else
                    {
                        Node* cu = copy (node->left);
                        Node* cv = copy (node->right);
                        Node* du = diff (node->left);
                        Node* dv = diff (node->right);
                        Node* res1 = create_node (OPER, MUL, dv, create_node (OPER, LN, NULL, cu));
                        Node* res2 = create_node (OPER, MUL, du, cv);
                        Node* res3 = create_node (OPER, DIV, res2, copy (cu));
                        Node* res4 = create_node (OPER, ADD, res1, res3);
                        Node* res5 = create_node (OPER, DEG, copy (cu), copy (cv));
                        return create_node (OPER, MUL, res5, res4);
                    }
                }
                case EXP:
                {
                    Node* du = diff (node->right);
                    Node* cu = copy (node->right);
                    return create_node (OPER, MUL, create_node (OPER, EXP, NULL, cu), du);
                }
            }
    }
}

int search_var (void)
{
    switch ((int) *s)
    {
        case ((int) 'x'):
            return 0;
        case ((int) 'y'):
            return 1;
        case ((int) 'z'):
            return 2;
        default:
            return -1;
    }
}

/*enum DifError token (struct Tokens* TOK, struct Vars* VARS)
{
    int n_tok = 0;
    double num = 0;
    while (*s != '$')
    {
        skip_space (&s);
        char* start_pos = s;
        int n_var = search_var ();
        if (n_var != -1)
        {
            TOK[n_tok].type = VAR;
            TOK[n_tok].elem.n_var = n_var;
            s++;
            n_tok++;
            continue;
        }
        bool is_al_num = false;
        while (isalnum (*s) || *s == '.')
        {
            is_al_num = true;
            s++;
        }

        if (is_al_num == false)
            s++;

        int n_oper = dif_search_func (start_pos, (size_t) (s - start_pos));

        if (is_al_num == false && n_oper == -1)
        {
            TOK[n_tok].type = TXT;
            TOK[n_tok].elem.symb = *start_pos;
            n_tok++;
            continue;
        }
        else if (sscanf (start_pos, "%lf", &num) != 0)
        {
            TOK[n_tok].type = NUM;
            TOK[n_tok].elem.num = num;
            n_tok++;
            continue;
        }
        else
        {
            TOK[n_tok].type = OPER;
            TOK[n_tok].elem.n_oper = n_oper;
            n_tok++;
            continue;
        }
        return DIF_SYNTAX_ERROR;
    }
    TOK[n_tok].type = TXT;
    TOK[n_tok].elem.symb = '$';
    tokin_dump (TOK, n_tok);
    return DIF_NO_ERROR;
}*/

enum DifError token (struct Tokens* TOK, struct Vars* VARS)
{
    int n_tok = 0;
    double num = 0;
    int n_var = 0;
    while (*s != '$')
    {
        skip_space (&s);
        char* start_pos = s;
        bool is_alpha = false;
        while (isalpha (*s))
        {
            is_alpha = true;
            s++;
        }

        if (is_alpha == false && (*s == '+' ||
                                  *s == '-' ||
                                  *s == '*' ||
                                  *s == '/' ||
                                  *s == '^'))
        {
            is_alpha = true;
            s++;
        }

        if (is_alpha == true)
        {
            int n_oper = dif_search_func (start_pos, (size_t) (s - start_pos));
            if (n_oper == -1)
            {
                assert (n_var < MAX_N_VARS);

                TOK[n_tok].type = VAR;
                TOK[n_tok].elem.n_var = n_var;

                VARS[n_var].num = n_var;
                VARS[n_var].len = s - start_pos;
                VARS[n_var].name = start_pos;

                n_var++;
                n_tok++;
                continue;
            }
            else
            {
                TOK[n_tok].type = OPER;
                TOK[n_tok].elem.n_oper = n_oper;
                n_tok++;
                continue;
            }
        }

        bool was_point = false;

        while (isdigit (*s))
        {
            s++;
            if (*s == '.' && was_point == false)
                s++;
        }

        if (sscanf (start_pos, "%lf", &num) != 0)
        {
            TOK[n_tok].type = NUM;
            TOK[n_tok].elem.num = num;
            n_tok++;
            continue;
        }

        if (*s == '(' || *s == ')')
        {
            s++;
            TOK[n_tok].type = TXT;
            TOK[n_tok].elem.symb = *start_pos;
            n_tok++;
            continue;
        }
        return DIF_SYNTAX_ERROR;
    }
    TOK[n_tok].type = TXT;
    TOK[n_tok].elem.symb = '$';
    tokin_dump (TOK, n_tok, VARS);
    return DIF_NO_ERROR;
}

void tokin_dump (struct Tokens* TOK, int n_tok, struct Vars* VARS)
{
    for (int pass = 0; pass < n_tok + 1; pass++)
    {
        switch (TOK[pass].type)
        {
            case OPER:
                fprintf (log_file, "OPER: %d --> %s\n", pass, OP[TOK[pass].elem.n_oper].name);
                break;
            case NUM:
                fprintf (log_file, "NUM: %d --> %lf\n", pass, TOK[pass].elem.num);
                break;
            case VAR:
                fprintf (log_file, "VAR: %d --> %.*s\n", pass, VARS[TOK[pass].elem.n_var].len, VARS[TOK[pass].elem.n_var].name);
                break;
            case TXT:
                fprintf (log_file, "TXT: %d --> %c\n", pass, TOK[pass].elem.symb);
                break;
        }
    }
}

int dif_search_func (const char* name, size_t len)
{
    for (int i = 0; i < N_FUNC; i++)
    {
        if (strncmp (name, OP[i].name, len) == 0)
            return i;
    }
    return -1;
}


void skip_space (char** str)
{
    while (isspace(**str))
        (*str)++;
}

//========================================

Node* simplification (Node* node, enum DifError* error)
{
    bool change_this_time = false;
    Node* c_node = copy (node);
    do
    {
        change_this_time = false;
        c_node = swertka_const (c_node, &change_this_time);
        c_node = nul_and_one (c_node, &change_this_time, error);
    }
    while (change_this_time);

    return c_node;
}

Node* nul_and_one (Node* node, bool* change, enum DifError* error)
{
    if (node->left != NULL)
        node->left = nul_and_one (node->left, change, error);

    if (node->right != NULL)
        node->right = nul_and_one (node->right, change, error);

    if (node->left != NULL && node->left->type == NUM && node->left->value.number == 1)
    {
        Node* res = NULL;
        switch (OP[node->value.n_oper].op_enum)
        {
            case MUL:
                *change = true;
                res = copy (node->right);
                tree_dtor (node);
                return res;
            case DEG:
                *change = true;
                tree_dtor (node);
                return create_node (NUM, 1, NULL, NULL);
        }
    }
    if (node->right != NULL && node->right->type == NUM && node->right->value.number == 1)
    {
        switch (OP[node->value.n_oper].op_enum)
        {
            case MUL:
            case DIV:
            case DEG:
                *change = true;
                Node* res = copy (node->left);
                tree_dtor (node);
                return res;
        }
    }
    if (node->left != NULL && node->left->type == NUM && node->left->value.number == 0)
    {
        switch (OP[node->value.n_oper].op_enum)
        {
            case MUL:
            case DIV:
            case DEG:
                *change = true;
                tree_dtor (node);
                return create_node (NUM, 0, NULL, NULL);
        }
    }
    if (node->right != NULL && node->right->type == NUM && node->right->value.number == 0)
    {
        switch (OP[node->value.n_oper].op_enum)
        {
            case MUL:
                *change = true;
                tree_dtor (node);
                return create_node (NUM, 0, NULL, NULL);
            case DIV:
                *error = DIF_DIV_NUL;
                return NULL;
            case DEG:
                *change = true;
                tree_dtor (node);
                return create_node (NUM, 1, NULL, NULL);
        }
    }
    return node;

}

Node* swertka_const (Node* node, bool* change)
{
    if (node->left != NULL)
        node->left = swertka_const (node->left, change);

    if (node->right != NULL)
        node->right = swertka_const (node->right, change);

    if (node->left == NULL && node->right != NULL && node->right->type == NUM)
    {
        double val = NAN;
        Node* res = NULL;
        switch (OP[node->value.n_oper].op_enum)
        {
            case SIN:
                *change = true;
                res = create_node (NUM, sin (node->right->value.number), NULL, NULL);
                tree_dtor (node);
                return res;
            case COS:
                *change = true;
                res = create_node (NUM, cos (node->right->value.number), NULL, NULL);
                tree_dtor (node);
                return res;
            case LN:
                *change = true;
                res = create_node (NUM, log (node->right->value.number), NULL, NULL);
                tree_dtor (node);
                return res;
            case SQRT:
                *change = true;
                res = create_node (NUM, sqrt (node->right->value.number), NULL, NULL);
                tree_dtor (node);
                return res;
            case EXP:
                *change = true;
                res = create_node (NUM, exp (node->right->value.number), NULL, NULL);
                tree_dtor (node);
                return res;
        }
    }
    else if (node->left != NULL && node->left->type == NUM && node->right->type == NUM)
    {
        double val1 = node->left->value.number;
        double val2 = node->right->value.number;
        Node* res = NULL;
        switch (OP[node->value.n_oper].op_enum)
        {
            case ADD:
                *change = true;
                res = create_node (NUM, val1 + val2, NULL, NULL);
                tree_dtor (node);
                return res;
            case SUB:
                *change = true;
                res = create_node (NUM, val1 - val2, NULL, NULL);
                tree_dtor (node);
                return res;
            case MUL:
                *change = true;
                res = create_node (NUM, val1 * val2, NULL, NULL);
                tree_dtor (node);
                return res;
            case DIV:
                *change = true;
                res = create_node (NUM, val1 / val2, NULL, NULL);
                tree_dtor (node);
                return res;
            case DEG:
                *change = true;
                res = create_node (NUM, pow (val1, val2), NULL, NULL);
                tree_dtor (node);
                return res;
        }
    }
    return node;
}

void taylor (Node* node, enum DifError* error)
{
    printf ("Hello! Would you like to decompose the expression? ('Yes' or 'No')\n");
    char answer[FOR_ANSW] = {};
    if (fgets (answer, FOR_ANSW, stdin) == NULL)
        *error = DIF_FGETS_ERROR;
    if (strlen (answer) > YES_NO_LEN)
        printf ("Incorrect response. Some of the information is lost.\n");
    if (strncmp (answer, "No\n", YES_NO_LEN) == 0)
    {
        printf ("Sadly :(\n");
        return;
    }
    if (strncmp (answer, "Yes\n", YES_NO_LEN) == 0)
    {
        double x0 = NAN;
        int n = 0;
        printf ("In the vicinity of which x? (enter a number)\n");

        if (scanf ("%lf", &x0) == 0)
            *error = DIF_ERROR_x0;

        printf ("To what extent? (enter a number)\n");

        if (scanf ("%d", &n) == 0)
            *error = DIF_ERROR_N;

        printf ("Processing...\n");

        printf ("y = ");

        Node* change_node = NULL;
        Node* prev_node = NULL;

        for (int i = 0; i <= n; i++)
        {
            prev_node = node;
            if (i > 0)
            {
                node = diff (node);
                tree_dtor (prev_node);
            }

            change_node = copy (node);
            change_node = change_x0 (change_node, x0);
            prev_node = change_node;
            change_node = simplification (change_node, error);
            tree_dtor (prev_node);
            printf ("%lf * (x - %lf)^%d / %d + ", change_node->value.number, x0, i, fact (i));
            tree_dtor (change_node);
        }
        tree_dtor (node);

        printf ("o (x - %lf)^%d\n", x0, n);
        printf ("It was difficult.\n");
    }

    else
        printf ("Incorrect response. Goodbye!\n");
}

int fact (int n)
{
    int res = 1;
    for (int i = 1; i <= n; i++)
    {
        res *= i;
    }
    return res;
}

Node* n_diff (Node* node, int i)
{
    for (int pass = 0; pass < i; pass++)
    {
        node = diff (node);
    }
    return node;
}

Node* change_x0 (Node* node, double x0)
{
    if (node->left != NULL)
        node->left = change_x0 (node->left, x0);

    if (node->right != NULL)
        node->right = change_x0 (node->right, x0);

    if (node->type == VAR)
    {
        node->type = NUM;
        node->value.number = x0;
    }
    return node;
}

Node* get_g (enum DifError* error, struct Tokens* TOK, int* n_tok)
{
    Node* val = get_e (error, TOK, n_tok);

    if (TOK[*n_tok].type == TXT && TOK[*n_tok].elem.symb != '$')
    {
        printf ("Не смогли обработать выражение :(\n");
        *error = DIF_SYNTAX_ERROR;
    }

    return val;
}

Node* get_e (enum DifError* error, struct Tokens* TOK, int* n_tok)
{
    Node* val = NULL;
    if (TOK[*n_tok].type == OPER && TOK[*n_tok].elem.n_oper == SUB)
    {
        (*n_tok)++;

        if (couple_mis_op (*n_tok, TOK) == true)
        {
            *error = DIF_SYNTAX_ERROR;
            printf ("Введено некорректное выражение: оператор \"%s\" сразу после оператора \"%s\"."
                    "Работа программы завершена досрочно :(\n", OP[TOK[*n_tok].elem.n_oper].name,
                                                                OP[TOK[*n_tok - 1].elem.n_oper].name);
            return create_node (OPER, ADD, val, NULL); // не разбираюсь, ADD или SUB,
                                                       // т.к. в любом случае надо удалять дерево
        }

        val = create_node (OPER, MUL, create_node (NUM, -1, NULL, NULL), get_t (error, TOK, n_tok));
    }
    else
    {
        val = get_t (error, TOK, n_tok);
    }
    while (TOK[*n_tok].type == OPER && (OP[TOK[*n_tok].elem.n_oper].op_enum == ADD ||
    OP[TOK[*n_tok].elem.n_oper].op_enum == SUB))
    {
        int old_n_tok = (*n_tok);
        (*n_tok)++;

        if (couple_mis_op (*n_tok, TOK) == true)
        {
            *error = DIF_SYNTAX_ERROR;
            printf ("Введено некорректное выражение: оператор \"%s\" сразу после оператора \"%s\"."
                    "Работа программы завершена досрочно :(\n", OP[TOK[*n_tok].elem.n_oper].name,
                                                                OP[TOK[*n_tok - 1].elem.n_oper].name);
            return create_node (OPER, ADD, val, NULL); // не разбираюсь, ADD или SUB,
                                                       // т.к. в любом случае надо удалять дерево
        }

        Node* val2 = get_t (error, TOK, n_tok);
        if (OP[TOK[old_n_tok].elem.n_oper].op_enum == ADD)
            val = create_node (OPER, ADD, val, val2);
        else
            val = create_node (OPER, SUB, val, val2);
    }
    return val;
}

bool couple_mis_op (int n_tok, struct Tokens* TOK)
{
    if (TOK[n_tok].type == OPER)
    {
        switch (OP[TOK[n_tok].elem.n_oper].op_enum)
        {
            case ADD:
            case SUB:
            case MUL:
            case DIV:
            case DEG:
                return true;
        }
    }
    return false;
}

Node* get_k (enum DifError* error, struct Tokens* TOK, int* n_tok)
{
    Node* val = get_p (error, TOK, n_tok);
    if (TOK[*n_tok].type == OPER && OP[TOK[*n_tok].elem.n_oper].op_enum == DEG)
    {
        (*n_tok)++;

        if (couple_mis_op (*n_tok, TOK) == true)
        {
            *error = DIF_SYNTAX_ERROR;
            printf ("Введено некорректное выражение: оператор \"%s\" сразу после оператора \"%s\"."
                    "Работа программы завершена досрочно :(\n", OP[TOK[*n_tok].elem.n_oper].name,
                                                                OP[TOK[*n_tok - 1].elem.n_oper].name);
            return create_node (OPER, ADD, val, NULL); // не разбираюсь, ADD или SUB,
                                                       // т.к. в любом случае надо удалять дерево
        }

        Node* val2 = get_p (error, TOK, n_tok);
        val = create_node (OPER, DEG, val, val2);

        if (TOK[*n_tok].type == OPER && OP[TOK[*n_tok].elem.n_oper].op_enum == DEG)
        {
            *error = DIF_SYNTAX_ERROR;
            printf ("Введено некорректное выражение: "
                    "используйте скобки для записи.\n");
        }
    }
    return val;
}

Node* get_s (enum DifError* error, struct Tokens* TOK, int* n_tok)
{
    if (TOK[*n_tok].type == OPER)
    {
        int n_oper = TOK[*n_tok].elem.n_oper;
        (*n_tok)++;

        if (TOK[*n_tok].type == TXT && TOK[*n_tok].elem.symb != '(')
            *error = DIF_SYNTAX_ERROR;

        (*n_tok)++;
        Node* val = get_e (error, TOK, n_tok);

        if (TOK[*n_tok].type == TXT && TOK[*n_tok].elem.symb != ')')
            *error = DIF_SYNTAX_ERROR;

        (*n_tok)++;

        switch (OP[n_oper].op_enum)
        {
            case SIN:
                val = create_node (OPER, SIN, NULL, val);
                return val;
            case COS:
                val = create_node (OPER, COS, NULL, val);
                return val;
            case LN:
                val = create_node (OPER, LN, NULL, val);
                return val;
            case SQRT:
                val = create_node (OPER, SQRT, NULL, val);
                return val;
            case EXP:
                val = create_node (OPER, EXP, NULL, val);
                return val;
            default:
                *error = DIF_FUNC_ERROR;
        }
    }
    return get_k (error, TOK, n_tok);
}

Node* get_p (enum DifError* error, struct Tokens* TOK, int* n_tok)
{
    if (TOK[*n_tok].type == TXT && TOK[*n_tok].elem.symb == '(')
    {
        (*n_tok)++;
        Node* val = get_e (error, TOK, n_tok);

        if (TOK[*n_tok].type == TXT && TOK[*n_tok].elem.symb != ')')
            *error = DIF_SYNTAX_ERROR;

        (*n_tok)++;

        return val;
    }
    Node* val = get_n (error, TOK, n_tok);
    return val;
}

Node* get_t (enum DifError* error, struct Tokens* TOK, int* n_tok)
{
    Node* val = get_s (error, TOK, n_tok);
    while (TOK[*n_tok].type == OPER && (OP[TOK[*n_tok].elem.n_oper].op_enum == MUL
    || OP[TOK[*n_tok].elem.n_oper].op_enum == DIV))
    {
        int old_n_tok = *n_tok;
        (*n_tok)++;

        if (couple_mis_op (*n_tok, TOK) == true)
        {
            *error = DIF_SYNTAX_ERROR;
            printf ("Введено некорректное выражение: оператор \"%s\" сразу после оператора \"%s\"."
                    "Работа программы завершена досрочно :(\n", OP[TOK[*n_tok].elem.n_oper].name,
                                                                OP[TOK[*n_tok - 1].elem.n_oper].name);
            return create_node (OPER, ADD, val, NULL); // не разбираюсь, ADD или SUB,
                                                       // т.к. в любом случае надо удалять дерево
        }

        Node* val2 = get_s (error, TOK, n_tok);

        if (OP[TOK[old_n_tok].elem.n_oper].op_enum == MUL)
            val = create_node (OPER, MUL, val, val2);
        else
            val = create_node (OPER, DIV, val, val2);
    }
    return val;
}

Node* get_n (enum DifError* error, struct Tokens* TOK, int* n_tok)
{
    double val = 0;
    int n_var = -1;

    if (TOK[*n_tok].type == NUM)
    {
        val = TOK[*n_tok].elem.num;
        (*n_tok)++;
        return create_node (NUM, val, NULL, NULL);
    }
    else if (TOK[*n_tok].type == VAR)
    {
        n_var = TOK[*n_tok].elem.n_var;
        (*n_tok)++;
        return create_node (VAR, n_var, NULL, NULL);
    }
    else
    {
        *error = DIF_SYNTAX_ERROR;
        return NULL;
    }
}
