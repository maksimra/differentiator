#include "../include/diff.hpp"
#include "../include/compare_doubles.hpp"

static FILE* log_file = stderr;

#define PRINT(...) if (log_file != NULL) fprintf (log_file, __VA_ARGS__)

const int NUM_OF_CHAR_TO_ANSW = 10;

const int NOT_VAR = -1;

const int NUM_OF_OPERS = sizeof (OPER_ARRAY) / sizeof (OPER_ARRAY[0]);

DifError dif_set_log_file (FILE* file)
{
    log_file = file;
    return DIF_NO_ERROR;
}

DifError read_file (FILE* file, const char* file_name, char** buffer, size_t* size)
{
    assert (file != NULL);
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

Operator search_oper (const char* str, size_t len)
{
    for (int n_oper = 1; n_oper < NUM_OF_OPERS; n_oper++)
    {
        if (strncmp (str, OPER_ARRAY[n_oper].name, len) == 0)
            return OPER_ARRAY[n_oper].op_enum;
    }

    return OPER_NONE;
}

void printf_str (FILE* file, const Node* node, int n_space)
{
    switch (node->type)
    {
        case TYPE_OPER:
            fprintf (file, "%*c(\"%s\"\n", n_space, ' ', get_oper_name (node->value.oper));
            return;
        case TYPE_VAR:
            fprintf (file, "%*c(\"<%d>\"\n", n_space, ' ', node->value.n_var);
            return;
        case TYPE_NUM:
            fprintf (file, "%*c(\"%.3lf\"\n", n_space, ' ', node->value.number);
            return;
        case TYPE_TXT:
        default:
            assert (0 && "Attempt to print TYPE_TXT to svg file.");
    }
}

void print_node_or_decr_tabs (Node* node, FILE* file, int* n_space)
{
    if (node != NULL)
    {
        print_tree_txt_incr_tabs (node, file);
    }
    else
    {
        fprintf (file, "_");
        (*n_space) -= 4;
    }
}

void print_tree_txt_incr_tabs (Node* node, FILE* file)
{
    static int n_space = 0;
    printf_str (file, node, n_space);
    n_space += 4;

    print_node_or_decr_tabs (node->left, file, &n_space);
    print_node_or_decr_tabs (node->right, file, &n_space);

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

    if (type == TYPE_OPER)
        new_node->value.oper = (Operator) value;

    else if (type == TYPE_NUM)
        new_node->value.number = value;

    else if (type == TYPE_VAR)
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
        case TYPE_NUM:
            return create_node (TYPE_NUM, 0, NULL, NULL, error);
        case TYPE_VAR:
            return create_node (TYPE_NUM, 1, NULL, NULL, error);
        case TYPE_OPER:
            return OPER_ARRAY[(int) node->value.oper].dif(node, error);
        case TYPE_TXT:
        default:
            assert (0);
    }
}

DifError token (Tokens* tok, Vars* vars, const char* buffer, int max_n_vars)
{
    DifError error = DIF_NO_ERROR;
    int n_tok = 0;
    int n_vars = 0;
    while (*buffer != '$')
    {
        skip_space (&buffer);
        if (try_char_operation (tok, n_tok, &buffer) ||
            try_digit          (tok, n_tok, &buffer) ||
            try_parenthesis    (tok, n_tok, &buffer) ||
            try_function       (tok, n_tok, &buffer) ||
            try_var            (tok, n_tok, vars, &n_vars, max_n_vars, &buffer, &error))
        {
            n_tok++;
            continue;
        }
        else
        {
            return DIF_ERROR_SYNTAX;
        }
    }

    tok[n_tok].type = TYPE_TXT;
    tok[n_tok].elem.symbol = '$';
    token_dump (tok, n_tok, vars);
    return DIF_NO_ERROR;
}

bool try_var (Tokens* tok, int n_tok, Vars* vars, int* n_vars, int max_n_vars, const char** buffer, DifError* error)
{
    if (isalpha (**buffer))
    {
        const char* start_position = *buffer;

        (*buffer)++;
        while (isalpha (**buffer))
            (*buffer)++;

        tok[n_tok].type = TYPE_VAR;
        int n_var = search_var (vars, *n_vars, start_position, (size_t) (*buffer - start_position));
        if (n_var != NOT_VAR)
        {
            tok[n_tok].elem.n_var = n_var;
        }
        else
        {
            if (*n_vars >= max_n_vars)
            {
                *error = DIF_ERROR_N_VARS;
                return false;
            }

            vars[*n_vars].num = *n_vars;
            vars[*n_vars].len = (size_t) (*buffer - start_position);
            vars[*n_vars].name = start_position;
            tok[n_tok].elem.n_var = *n_vars;
            (*n_vars)++;
        }
        return true;
    }
    return false;
}

bool try_function (Tokens* tok, int n_tok, const char** buffer)
{
    size_t len_of_function = 0;

    if (isalpha (**buffer))
    {
        len_of_function++;
        while (isalpha ((*buffer)[len_of_function]))
            len_of_function++;

        Operator oper = search_oper (*buffer, len_of_function);
        if (oper != OPER_NONE)
        {
            (*buffer) += len_of_function;
            fill_token_oper (tok, n_tok, oper);
            return true;
        }
    }
    return false;
}

bool try_parenthesis (Tokens* tok, int n_tok, const char** buffer)
{
    if ((**buffer) == '(' || (**buffer) == ')')
    {
        tok[n_tok].type = TYPE_TXT;
        tok[n_tok].elem.symbol = **buffer;
        (*buffer)++;
        return true;
    }
    return false;
}

bool try_digit (Tokens* tok, int n_tok, const char** buffer)
{
    if (isdigit (**buffer))
    {
        size_t len_of_double = fill_token_double (tok, n_tok, *buffer);
        (*buffer) += len_of_double;
        return true;
    }
    return false;

}

bool try_char_operation (Tokens* tok, int n_tok, const char** buffer)
{
    Operator oper = search_char_operation (*buffer);
    if (oper != OPER_NONE)
    {
        (*buffer)++;
        fill_token_oper (tok, n_tok, oper);
        return true;
    }
    return false;
}

Operator search_char_operation (const char* buffer)
{
    for (int num_of_oper = 1; OPER_ARRAY[num_of_oper].is_func == false; num_of_oper++)
    {
        if (strncmp (buffer, OPER_ARRAY[num_of_oper].name, sizeof (char)) == 0)
            return OPER_ARRAY[num_of_oper].op_enum;
    }
    return OPER_NONE;
}

void fill_token_oper (Tokens* tok, int n_tok, Operator oper)
{
    tok[n_tok].type = TYPE_OPER;
    tok[n_tok].elem.oper = oper;
}

size_t fill_token_double (Tokens* tok, int n_tok, const char* buffer)
{
    double number = NAN;
    sscanf (buffer, "%lf", &number);
    tok[n_tok].type = TYPE_NUM;
    tok[n_tok].elem.num = number;

    size_t len_of_double = 0;
    bool point_was = false;
    while (isdigit(buffer[len_of_double]))
    {
        len_of_double++;
        if (buffer[len_of_double] == '.' && point_was == false)
        {
            len_of_double++;
            point_was = true;
        }
    }
    return len_of_double;
}

int search_var (Vars* vars, int n_vars, const char* begin, size_t len)
{
    for (int number_of_var = 0; number_of_var < n_vars; number_of_var++)
        if (strncmp (begin, vars[number_of_var].name, len) == 0)
            return number_of_var;

    return NOT_VAR;
}

const char* get_oper_name (Operator oper)
{
    return OPER_ARRAY[(int) oper].name;
}

void token_dump (const Tokens* tok, int n_tok, const Vars* vars)
{
    for (int pass = 0; pass <= n_tok; pass++)
    {
        switch (tok[pass].type)
        {
            case TYPE_OPER:
                PRINT ("OPER: %d --> %s\n", pass, get_oper_name (tok[pass].elem.oper));
                break;
            case TYPE_NUM:
                PRINT ("NUM: %d --> %lf\n", pass, tok[pass].elem.num);
                break;
            case TYPE_VAR:
                PRINT ("VAR: %d --> %.*s\n", pass, (int) vars[tok[pass].elem.n_var].len, vars[tok[pass].elem.n_var].name);
                break;
            case TYPE_TXT:
                PRINT ("TXT: %d --> %c\n", pass, tok[pass].elem.symbol);
                break;
            default:
                assert (0);
        }
    }
}

void skip_space (const char** str)
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

    if (node->type == TYPE_OPER)
        return OPER_ARRAY[(int) node->value.oper].smp(node, change, error);

    return node;
}

Node* count_const (Node* node, bool* change, DifError* error)
{
    if (node->left != NULL)
        node->left = count_const (node->left, change, error);

    if (node->right != NULL)
        node->right = count_const (node->right, change, error);

    if ((node->left == NULL && node->right != NULL && node->right->type == TYPE_NUM) ||
        (node->left != NULL && node->left->type == TYPE_NUM && node->right->type == TYPE_NUM))
    {
        *change = true;
        return OPER_ARRAY[(int) node->value.oper].eval(node, error);
    }

    return node;
}

DifError taylor (const Node* node)
{
    printf ("Hello! Would you like to decompose the expression? ('yes' or 'no')\n");
    char answer[NUM_OF_CHAR_TO_ANSW] = {};
    if (fgets (answer, NUM_OF_CHAR_TO_ANSW, stdin) == NULL)
        return DIF_ERROR_FGETS;
    if (strncmp (answer, "no\n", NUM_OF_CHAR_TO_ANSW) == 0)
    {
        printf ("Sadly :(\n");
        return DIF_NO_ERROR;
    }
    if (strncmp (answer, "yes\n", NUM_OF_CHAR_TO_ANSW) == 0)
    {
        double x0 = NAN;
        int accuracy = 0;
        printf ("In the vicinity of which x? (enter a number)\n");

        if (scanf ("%lf", &x0) == 0)
            return DIF_ERROR_x0;

        printf ("To what extent? (enter a number)\n");

        if (scanf ("%d", &accuracy) == 0)
            return DIF_ERROR_N;

        return print_decompose (node, x0, accuracy);
    }
    else
    {
        printf ("Goodbye!\n");
        return DIF_NO_ERROR;
    }
}

DifError print_decompose (const Node* node, double x0, int accuracy)
{
    DifError error = DIF_NO_ERROR;
    printf ("Processing...\n");
    printf ("y = ");

    Node* change_node = NULL;
    Node* prev_node = NULL;
    Node* copy_node = copy_tree (node, &error);
    for (int i = 0; i <= accuracy; i++)
    {
        prev_node = copy_node;
        if (i > 0)
        {
            Node* temp_node = diff (copy_node, &error);
            if (error != DIF_NO_ERROR)
            {
                tree_dtor (copy_node);
                return error;
            }
            copy_node = temp_node;
            tree_dtor (prev_node);
        }
        change_node = copy_tree (copy_node, &error);
        change_node = change_x0 (change_node, x0);
        prev_node = change_node;
        change_node = simplification (change_node, &error);
        if (error != DIF_NO_ERROR)
        {
            printf ("\n%s\n", dif_get_error (error));
            tree_dtor (prev_node);
            tree_dtor (copy_node);
            return error;
        }
        tree_dtor (prev_node);
        if (compare_doubles (change_node->value.number, 0) != 0)
            printf ("%g * (x - %g)^%d / %d + ", change_node->value.number, x0, i, fact (i));
        tree_dtor (change_node);
    }
    tree_dtor (copy_node);
    printf ("o (x - %g)^%d\n", x0, accuracy);
    printf ("It was difficult.\n");

    return error;
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

    if (node->type == TYPE_VAR)
    {
        node->type = TYPE_NUM;
        node->value.number = x0;
    }
    return node;
}
