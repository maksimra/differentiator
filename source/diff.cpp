#include "../include/diff.hpp"

static FILE* log_file = stderr;

#define PRINT(...) if (log_file != NULL) fprintf (log_file, __VA_ARGS__)

const int NUM_OF_CHAR_TO_ANSW = 10;

const int NOT_VAR = -1;

const int NOT_OPER = -1;

const int NUM_OF_OPERS = sizeof (OP) / sizeof (OP[0]);

DifError dif_set_log_file (FILE* file)
{
    log_file = file;
    return DIF_NO_ERROR;
}

DifError read_file (FILE* file, const char* file_name, char** buffer, size_t* size)
{
    struct stat statbuf = {};

    if (stat (file_name, &statbuf))
        return DIF_ERROR_STAT;

    *buffer = (char*) calloc ((size_t) statbuf.st_size + sizeof (char), sizeof (char));

    if (*buffer == NULL)
        return DIF_ERROR_CALLOC;

    *size = fread (*buffer, sizeof (char), (size_t) statbuf.st_size, file);

    if (*size != (size_t) statbuf.st_size)
    {
        free (*buffer);
        return DIF_ERROR_FREAD;
    }

    return DIF_NO_ERROR;
}

int search_oper (const char* str, size_t len)
{
    for (int n_oper = 0; n_oper < NUM_OF_OPERS; n_oper ++)
        if (strncmp (str, OP[n_oper].name, len) == 0)
            return n_oper;

    return NOT_OPER;
}

void printf_str (FILE* file, const Node* node, int n_space)
{
    switch (node->type)
    {
        case OPER:
            fprintf (file, "%*c(\"%s\"\n", n_space, ' ', get_oper_name (node->value.oper));
            return;
        case VAR:
            fprintf (file, "%*c(\"<%d>\"\n", n_space, ' ', node->value.n_var);
            return;
        case NUM:
            fprintf (file, "%*c(\"%.3lf\"\n", n_space, ' ', node->value.number);
            return;
        case TXT:
        default:
            assert (0);
    }
}

void print_node_or_decr_tabs (Node* node, FILE* file, int* n_space)
{
    if (node != NULL)
    {
        print_tree_txt_incr_tabs (node, file, n_space);
    }
    else
    {
        fprintf (file, "_");
        (*n_space) -= 4;
    }
}

void print_tree_txt_incr_tabs (Node* node, FILE* file, int* n_space)
{
    printf_str (file, node, *n_space);
    (*n_space) += 4;

    print_node_or_decr_tabs (node->left, file, n_space);
    print_node_or_decr_tabs (node->right, file, n_space);

    fprintf (file, ")\n");
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

Node* create_node (Type type, double value, Node* left, Node* right, DifError* error)
{
    Node* new_node = (Node*) calloc (1, sizeof (Node));

    if (new_node == NULL)
    {
        *error = DIF_ERROR_CALLOC;
        return NULL;
    }

    new_node->type = type;

    if (type == OPER)
        new_node->value.oper = (Oper) value;

    else if (type == NUM)
        new_node->value.number = value;

    else if (type == VAR)
        new_node->value.n_var = (int) value;

    new_node->left = left;
    new_node->right = right;

    return new_node;
}

Node* copy_tree (const Node* node, DifError* error)
{
    Node* copy_node = (Node*) calloc (1, sizeof (Node));

    if (copy_node == NULL)
    {
        *error = DIF_ERROR_CALLOC;
        return NULL;
    }

    copy_node->type = node->type;

    copy_node->value = node->value;

    if (node->left != NULL)
        copy_node->left = copy_tree (node->left, error);

    else
        copy_node->left = NULL;

    if (node->right != NULL)
        copy_node->right = copy_tree (node->right, error);

    else
        copy_node->right = NULL;

    return copy_node;
}

void dif_print_error (DifError error)
{
    PRINT ("%s\n", dif_get_error (error));
}

const char* dif_get_error (DifError error)
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
        case DIF_ERROR_NULL_PTR_LOG:
            return "Dif: Передан нулевой указатель на log_file.";
        case DIF_ERROR_STRTOD:
            return "Dif: Ошибка работы функции strtod.";
        case DIF_ERROR_ARGC:
            return "Dif: Введено некорректное число аргументов. "
                   "Введите \"./output name_file.txt\".";
        case DIF_ERROR_ARGV:
            return "Dif: Введено некорректное имя файла.";
        case DIF_ERROR_FOPEN:
            return "Dif: Ошибка открытия файла (fopen).";
        case DIF_ERROR_FUNC:
            return "Dif: Ошибка определения типа функции.";
        case DIF_ERROR_SYNTAX:
            return "Dif: Синтаксическая ошибка.";
        case DIF_ERROR_DIV_NUL:
            return "Dif: Деление на нуль!";
        case DIF_ERROR_LN_NUL:
            return "Dif: Логарифм от нуля - опасно.";
        case DIF_ERROR_FGETS:
            return "Dif: Ошибка в работе функции fgets.";
        case DIF_ERROR_x0:
            return "Dif: Ошибка считывания x0.";
        case DIF_ERROR_N:
            return "Dif: Ошибка считывания степени разложения.";
        case DIF_ERROR_N_VARS:
            return "Dif: Слишком много переменных.";
        default:
            return "Dif: Куда делся мой enum ошибок?";
    }
}

Node* diff (const Node* node, DifError* error)
{
    switch (node->type)
    {
        case NUM:
            return create_node (NUM, 0, NULL, NULL, error);
        case VAR:
            return create_node (NUM, 1, NULL, NULL, error);
        case OPER:
            return OP[(int) node->value.oper].dif(node, error);
        case TXT:
        default:
            assert (0);
    }
}

DifError token (Tokens* tok, Vars* vars, char* buffer, int MAX_N_VARS)
{
    bool point_was = false;
    char* start_pos = NULL;
    int n_tok = 0;
    double num = 0;
    int n_vars = 0;
    int n_var = NOT_VAR;
    int n_oper = NOT_OPER;
    while (*buffer != '$')
    {
        skip_space (&buffer);
        start_pos = buffer;
        switch (*buffer)
        {
            case '+':
            case '-':
            case '*':
            case '/':
            case '^':
                buffer++;
                n_oper = search_oper (start_pos, (size_t) (buffer - start_pos));
                tok[n_tok].type = OPER;
                tok[n_tok].elem.oper = (Oper) n_oper;
                n_tok++;
                continue;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                sscanf (start_pos, "%lf", &num);
                point_was = false;
                while (isdigit(*buffer))
                {
                    buffer++;
                    if (*buffer == '.' && point_was == false)
                    {
                        buffer++;
                        point_was = true;
                    }
                }
                tok[n_tok].type = NUM;
                tok[n_tok].elem.num = num;
                n_tok++;
                continue;
            case '(':
            case ')':
                buffer++;
                tok[n_tok].type = TXT;
                tok[n_tok].elem.symbol = *start_pos;
                n_tok++;
                continue;
            default:
                if (isalpha (*buffer))
                {
                    buffer++;
                    while (isalpha (*buffer))
                        buffer++;

                    n_oper = search_oper (start_pos, (size_t) (buffer - start_pos));
                    if (n_oper != NOT_OPER)
                    {
                        tok[n_tok].type = OPER;
                        tok[n_tok].elem.oper = (Oper) n_oper;
                        n_tok++;
                        continue;
                    }
                    else
                    {
                        tok[n_tok].type = VAR;
                        n_var = search_var (vars, n_vars, start_pos, (size_t) (buffer - start_pos));
                        if (n_var != NOT_VAR)
                        {
                            tok[n_tok].elem.n_var = n_var;
                            n_tok++;
                            continue;
                        }
                        else
                        {
                            if (n_vars > MAX_N_VARS)
                                return DIF_ERROR_N_VARS;

                            vars[n_vars].num = n_vars;
                            vars[n_vars].len = (size_t) (buffer - start_pos);
                            vars[n_vars].name = start_pos;

                            tok[n_tok].elem.n_var = n_vars;
                            n_vars++;
                            n_tok++;
                            continue;
                        }
                    }
                }
                return DIF_ERROR_SYNTAX;


        }
    }
    tok[n_tok].type = TXT;
    tok[n_tok].elem.symbol = '$';
    token_dump (tok, n_tok, vars);
    return DIF_NO_ERROR;
}

int search_var (Vars* vars, int n_vars, const char* begin, size_t len)
{
    for (int number_of_var = 0; number_of_var < n_vars; number_of_var++)
        if (strncmp (begin, vars[number_of_var].name, len) == 0)
            return number_of_var;

    return NOT_VAR;
}

const char* get_oper_name (Oper oper)
{
    return OP[(int) oper].name;
}

void token_dump (Tokens* TOK, int n_tok, Vars* VARS)
{
    for (int pass = 0; pass <= n_tok; pass++)
    {
        switch (TOK[pass].type)
        {
            case OPER:
                PRINT ("OPER: %d --> %s\n", pass, get_oper_name (TOK[pass].elem.oper));
                break;
            case NUM:
                PRINT ("NUM: %d --> %lf\n", pass, TOK[pass].elem.num);
                break;
            case VAR:
                PRINT ("VAR: %d --> %.*s\n", pass, (int) VARS[TOK[pass].elem.n_var].len, VARS[TOK[pass].elem.n_var].name);
                break;
            case TXT:
                PRINT ("TXT: %d --> %c\n", pass, TOK[pass].elem.symbol);
                break;
            default:
                assert (0);
        }
    }
}

void skip_space (char** str)
{
    while (isspace(**str))
        (*str)++;
}


Node* simplification (Node* node, DifError* error)
{
    bool change_this_time = false;
    Node* c_node = copy_tree (node, error);
    do
    {
        change_this_time = false;
        c_node = count_const (c_node, &change_this_time, error);

        if (*error != DIF_NO_ERROR)
        {
            tree_dtor (c_node);
            return NULL;
        }

        c_node = zeros_and_ones (c_node, &change_this_time, error);

        if (*error != DIF_NO_ERROR)
        {
            tree_dtor (c_node);
            return NULL;
        }
    }
    while (change_this_time);

    return c_node;
}

Node* zeros_and_ones (Node* node, bool* change, DifError* error)
{
    if (node->left != NULL)
        node->left = zeros_and_ones (node->left, change, error);

    if (node->right != NULL)
        node->right = zeros_and_ones (node->right, change, error);

    if (node->type == OPER)
        return OP[(int) node->value.oper].smp(node, change, error);

    return node;
}

Node* count_const (Node* node, bool* change, DifError* error)
{
    if (node->left != NULL)
        node->left = count_const (node->left, change, error);

    if (node->right != NULL)
        node->right = count_const (node->right, change, error);

    if ((node->left == NULL && node->right != NULL && node->right->type == NUM) ||
        (node->left != NULL && node->left->type == NUM && node->right->type == NUM))
    {
        *change = true;
        return OP[(int) node->value.oper].eval(node, error);
    }

    return node;
}

void taylor (Node* node, DifError* error)
{
    printf ("Hello! Would you like to decompose the expression? ('yes' or 'no')\n");
    char answer[NUM_OF_CHAR_TO_ANSW] = {};
    if (fgets (answer, NUM_OF_CHAR_TO_ANSW, stdin) == NULL)
        *error = DIF_ERROR_FGETS;
    if (strncmp (answer, "no\n", NUM_OF_CHAR_TO_ANSW) == 0)
    {
        printf ("Sadly :(\n");
        return;
    }
    if (strncmp (answer, "yes\n", NUM_OF_CHAR_TO_ANSW) == 0)
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
        Node* cpy_node = copy_tree (node, error);

        for (int i = 0; i <= n; i++)
        {
            prev_node = cpy_node;
            if (i > 0)
            {
                Node* temp_node = diff (cpy_node, error);
                if (*error != DIF_NO_ERROR)
                {
                    tree_dtor (cpy_node);
                    return;
                }
                cpy_node = temp_node;
                tree_dtor (prev_node);
            }

            change_node = copy_tree (cpy_node, error);
            change_node = change_x0 (change_node, x0);
            prev_node = change_node;
            change_node = simplification (change_node, error);

            if (*error != DIF_NO_ERROR)
            {
                printf ("\n%s\n", dif_get_error (*error));
                tree_dtor (prev_node);
                tree_dtor (cpy_node);
                return;
            }

            tree_dtor (prev_node);
            printf ("%lf * (x - %lf)^%d / %d + ", change_node->value.number, x0, i, fact (i));
            tree_dtor (change_node);
        }
        tree_dtor (cpy_node);

        printf ("o (x - %lf)^%d\n", x0, n);
        printf ("It was difficult.\n");
    }
    else
    {
        printf ("Goodbye!\n");
    }
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
