#include "diff.h"

char* s = NULL;

const int N_OPER = sizeof (OP) / sizeof (OP[0]);

const int OPER_N_SYMB = 10;

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

    printf ("size == %d\n", *size);

    if (*size != statbuf.st_size)
        return DIF_ERROR_FREAD;

    fclose (file);

    return DIF_NO_ERROR;
}


/*enum DifError dif_read_tree (FILE* file, const char* NAME, Node** root)
{
    enum DifError error = DIF_NO_ERROR;
    struct stat statbuf = {};

    assert (file != NULL);

    if (stat (NAME, &statbuf))
        return DIF_ERROR_STAT;

    char* buffer = (char*) calloc ((long unsigned int) statbuf.st_size + sizeof (char), sizeof (char));

    size_t size = 0;

    size = fread (buffer, sizeof (char), (size_t) statbuf.st_size, file);

    buffer[size] = '\0';

    if (size != statbuf.st_size)
        return DIF_ERROR_FREAD;


    if (error != DIF_NO_ERROR)
        return error;

    int pos = 0;
    Node* cur_node = NULL;
    error = create_tree (buffer, &cur_node, &pos);

    if (error != DIF_NO_ERROR)
        return error;

    *root = cur_node;
    free (buffer);
    return DIF_NO_ERROR;
}*/

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
        if (node->value.n_oper == 4)
        {
            printf ("%s\n", OP[node->value.n_oper].name);
            printf ("Левая часть синуса: %p\n", node->right);
        }
        fprintf (file, "%*c(\"%s\"\n", n_space, ' ', OP[node->value.n_oper].name);
    }
    else if (node->type == VAR)
    {
        fprintf (file, "%*c(\"<%d>\"\n", n_space, ' ', node->value.n_var); // допилить потом
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
        dif_tree_print (node, file, n_space);
    }
    else
    {
        fprintf (file, "_");
        (*n_space) -= 4;
    }
}

void dif_tree_print (Node* node, FILE* file, int* n_space)
{
    printf ("Зашёл в печать дерева.\n");
    printf_str (file, node, *n_space);
    (*n_space) += 4;

    printing_branches (node->left, file, n_space);
    printing_branches (node->right, file, n_space);

    fprintf (file, ")\n");
}

enum DifError create_tree (char* buffer, Node** cur_node, int* pos)
{
    assert (buffer != NULL);
    enum DifError error = new_node (cur_node);

    if (error != DIF_NO_ERROR)
        return error;

    *pos += space_counter (buffer + *pos);

    if (buffer[*pos] == '{')
    {
        (*pos)++;
        error = create_tree (buffer, &((*cur_node)->left), pos);

        if (error != DIF_NO_ERROR)
            return error;

    }

    *pos += space_counter (buffer + *pos);

    if (buffer[*pos] == '#')
    {
        char* remainder = NULL;
        (*pos)++;
        if (buffer[*pos] == '#')
        {
            (*pos)++;
            free (*cur_node);
            (*cur_node) = NULL;
            return DIF_NO_ERROR;
        }

        double num = strtod (buffer + *pos, &remainder);

        if (num == 0)
            return DIF_ERROR_STRTOD;

        (*cur_node)->value.number = num;
        (*cur_node)->type = NUM;
        char* pos_hash = strchr (buffer + *pos, '#');
        (*pos) += pos_hash - (buffer + *pos) + sizeof ('#');
        *pos += space_counter (buffer + *pos);

        // Check that '}' exist so you can throw errors
        // if (buffer[*pos] != '}') return error;
        return DIF_NO_ERROR;
    }

    *pos += space_counter (buffer + *pos);

    char oper[OPER_N_SYMB] = {};
    sscanf (buffer + *pos, "%s", oper); // unsafe
    long len = strchr (buffer + *pos, ' ') - (buffer + *pos);
    int n_oper = search_oper (oper, len);

    if (n_oper >= 0)
    {
        (*cur_node)->type = OPER;
        (*cur_node)->value.n_oper = n_oper;
        (*pos) += len;
        *pos += space_counter (buffer + *pos);

        error = create_tree (buffer, &((*cur_node)->right), pos);

        if (error != DIF_NO_ERROR)
            return error;

        if  (buffer[*pos] != '}')
        {
            return DIF_ERROR_READ;
        }
        (*pos)++;

        return DIF_NO_ERROR;
    }

    return DIF_NO_ERROR;
}
//
// call label <=> push current address
//                jump label
//
// ret <=> pop
//         jump to it
//

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
    printf ("root type - %d\n", root->type);

    if (root->left != NULL)
        tree_dtor (root->left);

    if (root->right != NULL)
        tree_dtor (root->right);

    free (root);

}

enum DifError graphviz (Node* node, FILE* file)
{
    assert (node != NULL);
    assert (file != NULL);

    print_start (file);

    print_filling (node, file);

    print_end (file);

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

void print_filling (Node* node, FILE* file)
{
    draw_left (node, file);
    draw_right (node, file);
}

void draw_right (Node* node, FILE* file)
{
    char* color = NULL;
    assert (file != NULL);
    printf ("file == %p\n", file);
    if (node->right != NULL)
    {
        if (node->right->left == NULL && node->right->right == NULL)
            color = strdup ("red"); // sprintf and snprintf
        else
            color = strdup ("green");
        if (node->right->type == OPER)
        {
            printf ("OPER\n");
            fprintf (file, "{\"%s\n%p\"--\"%s\n%p\"[color = \"%s\"]};\n",
            OP[node->value.n_oper].name, node, OP[node->right->value.n_oper].name, node->right, color);
        }
        else if (node->right->type == NUM)
        {
            printf ("NUM\n");
            fprintf (file, "{\"%s\n%p\"--\"%.3lf\n%p\"[color = \"%s\"]};\n",
            OP[node->value.n_oper].name, node, node->right->value.number, node->right, color);
        }
        else if (node->right->type == VAR)
        {
            printf ("VAR\n");
            fprintf (file, "{\"%s\n%p\"--\"%c\n%p\"[color = \"%s\"]};\n",
            OP[node->value.n_oper].name, node, VARS[node->right->value.n_var].name, node->right, color);
        }

        print_filling (node->right, file);
    }
    free (color);
}

void draw_left (Node* node, FILE* file)
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
            printf ("OPER\n");
            fprintf (file, "{\"%s\n%p\"--\"%s\n%p\"[color = \"%s\"]};\n",
            OP[node->value.n_oper].name, node, OP[node->left->value.n_oper].name, node->left, color);
        }
        else if (node->left->type == NUM)
        {
            printf ("NUM\n");
            fprintf (file, "{\"%s\n%p\"--\"%.3lf\n%p\"[color = \"%s\"]};\n",
            OP[node->value.n_oper].name, node, node->left->value.number, node->left, color);
        }
        else if (node->left->type == VAR)
        {
            printf ("VAR\n");
            fprintf (file, "{\"%s\n%p\"--\"%c\n%p\"[color = \"%s\"]};\n",
            OP[node->value.n_oper].name, node, VARS[node->left->value.n_var].name, node->left, color);
        }
        print_filling (node->left, file);
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
                    Node* du = diff (node->left);
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
                    create_node (OPER, COS, NULL, cu),
                    du);
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
                        printf ("HUUUUI\n");
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
                        printf ("I AM HERE.\n");
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

enum DifError token (struct Tokens* TOK)
{
    printf ("Зашёл в функцию token\n");
    int n_tok = 0;
    double num = 0;
    while (*s != '$')
    {
        skip_space (&s);
        printf ("*s == %c\n", *s);
        char* start_pos = s;
        int n_var = search_var ();
        printf ("n_var == %d\n", n_var);
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
            printf ("Нахожусь в isalnum *s == %c\n", *s);
            is_al_num = true;
            s++;
        }
        printf ("Прошёл isalnum: *s == %c\ns - start_pos == %d\n", *s, s - start_pos);

        if (is_al_num == false)
            s++;

        int n_oper = dif_search_func (start_pos, (size_t) (s - start_pos));
        printf ("len == %zu\n", (size_t) (s - start_pos));
        printf ("n_oper == %d\n", n_oper);

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
        printf ("Сейчас выдам ошибку!!!\n");
        return DIF_SYNTAX_ERROR;
    }
    TOK[n_tok].type = TXT;
    TOK[n_tok].elem.symb = '$';
    tokin_dump (TOK, n_tok);
    return DIF_NO_ERROR; // add dump tokin
}

void tokin_dump (struct Tokens* TOK, int n_tok)
{
    for (int pass = 0; pass < n_tok; pass++)
    {
        switch (TOK[pass].type)
        {
            case OPER:
                printf ("OPER: %d --> %s\n", pass, OP[TOK[pass].elem.n_oper].name);
                break;
            case NUM:
                printf ("NUM: %d --> %lf\n", pass, TOK[pass].elem.num);
                break;
            case VAR:
                printf ("VAR: %d --> %c\n", pass, VARS[TOK[pass].elem.n_var].name);
                break;
            case TXT:
                printf ("TXT: %d --> %c\n", pass, TOK[pass].elem.symb);
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
    bool it_change = false;
    Node* c_node = copy (node);
    do
    {
        it_change = false;
        c_node = swertka_const (c_node, &it_change);
        c_node = nul_and_one (c_node, &it_change, error);
    }
    while (it_change);
    return c_node;
}

Node* nul_and_one (Node* node, bool* change, enum DifError* error)
{
    if (node->left != NULL && node->left->type == NUM && node->left->value.number == 1)
    {
        switch (OP[node->value.n_oper].op_enum)
        {
            case MUL:
                *change = true;
                return copy (node->right);
            case DEG:
                *change = true;
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
                return copy (node->left);
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
                return create_node (NUM, 0, NULL, NULL);
        }
    }
    if (node->right != NULL && node->right->type == NUM && node->right->value.number == 0)
    {
        switch (OP[node->value.n_oper].op_enum)
        {
            case MUL:
                *change = true;
                return create_node (NUM, 0, NULL, NULL);
            case DIV:
                *error = DIF_DIV_NUL;
                return NULL;
            case DEG:
                *change = true;
                return create_node (NUM, 1, NULL, NULL);
        }
    }
    if (node->left != NULL)
        node->left = nul_and_one (node->left, change, error);

    if (node->right != NULL)
        node->right = nul_and_one (node->right, change, error);

    return node;

}

Node* swertka_const (Node* node, bool* change)
{
    if (node->left == NULL && node->right != NULL && node->right->type == NUM)
    {
        double val = NAN;
        switch (OP[node->value.n_oper].op_enum)
        {
            case SIN:
                *change = true;
                return create_node (NUM, sin (node->right->value.number), NULL, NULL);
            case COS:
                *change = true;
                return create_node (NUM, cos (node->right->value.number), NULL, NULL);
            case LN:
                *change = true;
                return create_node (NUM, log (node->right->value.number), NULL, NULL);
            case SQRT:
                *change = true;
                return create_node (NUM, sqrt (node->right->value.number), NULL, NULL);
            case EXP:
                *change = true;
                return create_node (NUM, exp (node->right->value.number), NULL, NULL);
        }
    }
    else if (node->left != NULL && node->left->type == NUM && node->right->type == NUM)
    {
        double val1 = node->left->value.number;
        double val2 = node->right->value.number;
        switch (OP[node->value.n_oper].op_enum)
        {
            case ADD:
                *change = true;
                return create_node (NUM, val1 + val2, NULL, NULL);
            case SUB:
                *change = true;
                return create_node (NUM, val1 - val2, NULL, NULL);
            case MUL:
                *change = true;
                return create_node (NUM, val1 * val2, NULL, NULL);
            case DIV:
                *change = true;
                return create_node (NUM, val1 / val2, NULL, NULL);
            case DEG:
                *change = true;
                return create_node (NUM, pow (val1, val2), NULL, NULL);
        }
    }
    if (node->left != NULL)
        node->left = swertka_const (node->left, change);

    if (node->right != NULL)
        node->right = swertka_const (node->right, change);

    return node;
}

Node* get_g (enum DifError* error, struct Tokens* TOK, int* n_tok)
{
    printf ("get_g: n_tok = %d\n", *n_tok);
    Node* val = get_e (error, TOK, n_tok);

    return val;
}

Node* get_e (enum DifError* error, struct Tokens* TOK, int* n_tok)
{
    Node* val = NULL;
    printf ("get_e: n_tok = %d\n", *n_tok);
    if (TOK[*n_tok].type == OPER && TOK[*n_tok].elem.n_oper == SUB)
    {
        (*n_tok)++;
        val = create_node (OPER, MUL, create_node (NUM, -1, NULL, NULL), get_t (error, TOK, n_tok));
    }
    else // Node* create_node (enum Type type, double value, Node* left, Node* right)
    {
        val = get_t (error, TOK, n_tok);
    }
    while (TOK[*n_tok].type == OPER && (OP[TOK[*n_tok].elem.n_oper].op_enum == ADD ||
    OP[TOK[*n_tok].elem.n_oper].op_enum == SUB))
    {
        int old_n_tok = (*n_tok);
        (*n_tok)++;
        Node* val2 = get_t (error, TOK, n_tok);
        if (OP[TOK[old_n_tok].elem.n_oper].op_enum == ADD)
            val = create_node (OPER, ADD, val, val2);
        else
            val = create_node (OPER, SUB, val, val2);
    }
    return val;
}

Node* get_k (enum DifError* error, struct Tokens* TOK, int* n_tok)
{
    Node* val = get_p (error, TOK, n_tok);
    printf ("get_k: n_tok = %d\n", *n_tok);
    if (TOK[*n_tok].type == OPER && OP[TOK[*n_tok].elem.n_oper].op_enum == DEG)
    {
        printf ("Я в степени!\n");
        (*n_tok)++;
        Node* val2 = get_p (error, TOK, n_tok);
        val = create_node (OPER, DEG, val, val2);
    }
    return val;
}

Node* get_s (enum DifError* error, struct Tokens* TOK, int* n_tok)
{
    printf ("get_s: n_tok = %d\n", *n_tok);
    if (TOK[*n_tok].type == OPER)
    {
        int n_oper = TOK[*n_tok].elem.n_oper;
        (*n_tok)++;

        if (TOK[*n_tok].type == TXT && TOK[*n_tok].elem.symb != '(')
            *error = DIF_SYNTAX_ERROR;

        (*n_tok)++;
        printf ("перед get_k n_tok = %d\n", *n_tok);
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
    printf ("get_p: n_tok = %d\n", *n_tok);
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
    printf ("get_t: n_tok = %d\n", *n_tok);
    Node* val = get_s (error, TOK, n_tok);
    while (TOK[*n_tok].type == OPER && (OP[TOK[*n_tok].elem.n_oper].op_enum == MUL
    || OP[TOK[*n_tok].elem.n_oper].op_enum == DIV))
    {
        int old_n_tok = *n_tok;
        (*n_tok)++;
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
    printf ("get_n: n_tok = %d\n", *n_tok);
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
        printf ("In VARRRR\n");
        n_var = TOK[*n_tok].elem.n_var;
        (*n_tok)++;
        return create_node (VAR, n_var, NULL, NULL);
    }
    else
        *error = DIF_SYNTAX_ERROR;
}
