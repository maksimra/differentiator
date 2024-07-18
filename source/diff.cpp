#include "../include/diff.h"

static FILE* log_file = stderr;

#define PRINT(...) if (log_file != NULL) fprintf (log_file, __VA_ARGS__)

char* s = NULL;

static const int OPER_N_SYMB = 10;

static const int FOR_ANSW = 10; //

static const int YES_NO_LEN = 4;

static const double ACCURACY = 1e-5;

static const int N_OPERS = sizeof (OP) / sizeof (OP[0]);


bool compare_doubles (double a, double b)
{
    if (fabs (a - b) < ACCURACY)
        return true;

    return false;
}

enum DifError dif_set_log_file (FILE* file)
{
    log_file = file;
    return DIF_NO_ERROR;
}

enum DifError read_file (const char* NAME, size_t* size)
{
    struct stat statbuf = {};

    FILE* file = fopen (NAME, "r");

    if (stat (NAME, &statbuf))
        return DIF_ERROR_STAT;

    s = (char*) calloc ((size_t) statbuf.st_size + sizeof (char), sizeof (char));

    if (s == NULL)
        return DIF_ERROR_CALLOC;

    *size = fread (s, sizeof (char), (size_t) statbuf.st_size, file);

    if (*size != (size_t) statbuf.st_size)
    {
        free (s);
        return DIF_ERROR_FREAD;
    }

    fclose (file);

    return DIF_NO_ERROR;
}

int search_oper (const char* str, size_t len)
{
    for (int n_oper = 0; n_oper < N_OPERS; n_oper ++)
    {
        if (strncmp (str, OP[n_oper].name, len) == 0)
        {
            return n_oper;
        }
    }
    return -1;
}

size_t count_space (const char* line)
{
    size_t n_space = 0;
    while (isspace (*(line + n_space)))
    {
        n_space++;
    }
    return n_space;
}

void printf_str (FILE* file, const Node* node, int n_space)
{
    if (node->type == OPER)
    {
        fprintf (file, "%*c(\"%s\"\n", n_space, ' ', get_oper_name (node->value.oper));
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

enum DifError allocate_node (Node** node)
{
    Node* temp = (Node*) calloc (1, sizeof (Node));
    if (temp == NULL)
        return DIF_ERROR_CALLOC;
    *node = temp;
    return DIF_NO_ERROR;
}

void tree_dtor (Node* root)
{
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

void print_end (FILE* file)
{
    fprintf (file, "}");
}

void print_connections (Node* node, FILE* file, struct Vars* VARS)
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
    free (color);
}

Node* create_node (enum Type type, double value, Node* left, Node* right, enum DifError* error)
{
    Node* new_node = (Node*) calloc (1, sizeof (Node));

    if (new_node == NULL)
    {
        *error = DIF_ERROR_CALLOC;
        return NULL;
    }

    new_node->type = type;

    if (type == OPER)
        new_node->value.oper = (enum OPER) value;

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
    // printf ("node.type == %d\n", node->type);
    Node* copy_node = (Node*) calloc (1, sizeof (Node));

    copy_node->type = node->type;

    if (node->type == NUM)
        copy_node->value.number = node->value.number;

    else if (node->type == VAR)
        copy_node->value.n_var = node->value.n_var;

    else if (node->type == OPER)
        copy_node->value.oper = node->value.oper;

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
    PRINT ("%s\n", dif_get_error (error));
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
            return "Dif: Введено некорректное число аргументов. "
                   "Введите \"./output name_file.txt\".";
        case DIF_ERROR_ARGV:
            return "Dif: Введено некорректное имя файла.";
        case DIF_ERROR_FOPEN:
            return "Dif: Ошибка открытия файла (fopen).";
        case DIF_FUNC_ERROR:
            return "Dif: Ошибка определения типа функции.";
        case DIF_SYNTAX_ERROR:
            return "Dif: Синтаксическая ошибка.";
        case DIF_DIV_NUL:
            return "Dif: Деление на нуль!";
        case DIF_LN_NUL:
            return "Dif: Логарифм от нуля - опасно.";
        case DIF_FGETS_ERROR:
            return "Dif: Ошибка в работе функции fgets.";
        case DIF_ERROR_x0:
            return "Dif: Ошибка считывания x0.";
        case DIF_ERROR_N:
            return "Dif: Ошибка считывания степени разложения.";
        default:
            return "Dif: Куда делся мой enum ошибок?";
    }
}

Node* diff (const Node* node, enum DifError* error)
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

enum DifError token (struct Tokens* TOK, struct Vars* VARS, int MAX_N_VARS)
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
                VARS[n_var].len = (size_t) (s - start_pos);
                VARS[n_var].name = start_pos;

                n_var++;
                n_tok++;
                continue;
            }
            else
            {
                TOK[n_tok].type = OPER;
                TOK[n_tok].elem.oper = (enum OPER) n_oper;
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

const char* get_oper_name (enum OPER oper)
{
    return OP[(int) oper].name;
}

void tokin_dump (struct Tokens* TOK, int n_tok, struct Vars* VARS)
{
    for (int pass = 0; pass < n_tok + 1; pass++)
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
                PRINT ("TXT: %d --> %c\n", pass, TOK[pass].elem.symb);
                break;
            default:
                assert (0);
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
        c_node = swertka_const (c_node, &change_this_time, error);

        if (*error != DIF_NO_ERROR)
        {
            tree_dtor (c_node);
            return NULL;
        }

        c_node = nul_and_one (c_node, &change_this_time, error);

        if (*error != DIF_NO_ERROR)
        {
            tree_dtor (c_node);
            return NULL;
        }
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

    if (node->type == OPER)
        return OP[(int) node->value.oper].smp(node, change, error);

    return node;
}

Node* swertka_const (Node* node, bool* change, enum DifError* error)
{
    if (node->left != NULL)
        node->left = swertka_const (node->left, change, error);

    if (node->right != NULL)
        node->right = swertka_const (node->right, change, error);

    if (node->left == NULL && node->right != NULL && node->right->type == NUM ||
        node->left != NULL && node->left->type == NUM && node->right->type == NUM)
    {
        *change = true;
        return OP[(int) node->value.oper].eval(node, error);
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
                node = diff (node, error);
                tree_dtor (prev_node);
            }

            change_node = copy (node);
            change_node = change_x0 (change_node, x0);
            prev_node = change_node;
            change_node = simplification (change_node, error);

            if (*error != DIF_NO_ERROR)
            {
                printf ("\n%s\n", dif_get_error (*error));
                tree_dtor (prev_node);
                tree_dtor (node);
                return;
            }

            tree_dtor (prev_node);
            printf ("%lf * (x - %lf)^%d / %d + ", change_node->value.number, x0, i, fact (i));
            tree_dtor (change_node);
        }
        tree_dtor (node);

        printf ("o (x - %lf)^%d\n", x0, n);
        printf ("It was difficult.\n");
    }
    else
    {
        printf ("Goodbye!\n");
        tree_dtor (node);
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

Node* get_g (enum DifError* error, struct Tokens* TOK, int* n_tok)
{
    Node* val = get_e (error, TOK, n_tok);

    if ((TOK[*n_tok].type != TXT) ||
        (TOK[*n_tok].type == TXT && TOK[*n_tok].elem.symb != '$'))
    {
        printf ("Не смогли обработать выражение :(\n");
        *error = DIF_SYNTAX_ERROR;
    }

    return val;
}

Node* get_e (enum DifError* error, struct Tokens* TOK, int* n_tok)
{
    Node* val = NULL;
    if (TOK[*n_tok].type == OPER && TOK[*n_tok].elem.oper == SUB)
    {
        (*n_tok)++;

        if (couple_mis_op (*n_tok, TOK) == true)
        {
            *error = DIF_SYNTAX_ERROR;
            printf ("Введено некорректное выражение: оператор \"%s\" сразу после оператора \"%s\"."
                    "Работа программы завершена досрочно :(\n", get_oper_name (TOK[*n_tok].elem.oper),
                                                                get_oper_name (TOK[*n_tok - 1].elem.oper));
            return NULL;
        }

        val = create_node (OPER, MUL, create_node (NUM, -1, NULL, NULL, error), get_t (error, TOK, n_tok), error);
    }
    else
    {
        val = get_t (error, TOK, n_tok);
    }
    while (TOK[*n_tok].type == OPER && (TOK[*n_tok].elem.oper == ADD ||
    TOK[*n_tok].elem.oper == SUB))
    {
        int old_n_tok = (*n_tok);
        (*n_tok)++;

        if (couple_mis_op (*n_tok, TOK) == true)
        {
            *error = DIF_SYNTAX_ERROR;
            printf ("Введено некорректное выражение: оператор \"%s\" сразу после оператора \"%s\"."
                    "Работа программы завершена досрочно :(\n", get_oper_name (TOK[*n_tok].elem.oper),
                                                                get_oper_name (TOK[*n_tok - 1].elem.oper));
            tree_dtor (val);
            return NULL;
        }

        Node* val2 = get_t (error, TOK, n_tok);
        if (TOK[old_n_tok].elem.oper == ADD)
            val = create_node (OPER, ADD, val, val2, error);
        else
            val = create_node (OPER, SUB, val, val2, error);
    }
    return val;
}

bool couple_mis_op (int n_tok, struct Tokens* TOK)
{
    if (TOK[n_tok].type == OPER)
    {
        switch (TOK[n_tok].elem.oper)
        {
            case ADD:
            case SUB:
            case MUL:
            case DIV:
            case POW:
                return true;
        }
    }
    return false;
}

Node* get_k (enum DifError* error, struct Tokens* TOK, int* n_tok)
{
    Node* val = get_p (error, TOK, n_tok);
    if (TOK[*n_tok].type == OPER && TOK[*n_tok].elem.oper == POW)
    {
        (*n_tok)++;

        if (couple_mis_op (*n_tok, TOK) == true)
        {
            *error = DIF_SYNTAX_ERROR;
            printf ("Введено некорректное выражение: оператор \"%s\" сразу после оператора \"%s\"."
                    "Работа программы завершена досрочно :(\n", get_oper_name (TOK[*n_tok].elem.oper),
                                                                get_oper_name (TOK[*n_tok - 1].elem.oper));
            tree_dtor (val);
            return NULL;
        }

        Node* val2 = get_p (error, TOK, n_tok);
        val = create_node (OPER, POW, val, val2, error);

        if (TOK[*n_tok].type == OPER && TOK[*n_tok].elem.oper == POW)
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
        int n_oper = TOK[*n_tok].elem.oper;
        (*n_tok)++;

        if (TOK[*n_tok].type == OPER)
        {
            *error = DIF_SYNTAX_ERROR;
            printf ("Введено некорректное выражение: оператор \"%s\" сразу после оператора \"%s\"."
                    "Работа программы завершена досрочно :(\n", get_oper_name (TOK[*n_tok].elem.oper),
                                                                get_oper_name (TOK[*n_tok - 1].elem.oper));
            return NULL;
        }

        if (TOK[*n_tok].type == TXT && TOK[*n_tok].elem.symb != '(')
        {
            *error = DIF_SYNTAX_ERROR;
            printf ("Введено некорректное выражение:"
                    "после оператора %s ожидается '(' "
                    "(пробелы допускаются).\n", get_oper_name (TOK[*n_tok - 1].elem.oper));
            return NULL;
        }

        (*n_tok)++;
        Node* val = get_e (error, TOK, n_tok);

        if (TOK[*n_tok].type == TXT && TOK[*n_tok].elem.symb != ')')
            *error = DIF_SYNTAX_ERROR;

        (*n_tok)++;

        switch (OP[n_oper].op_enum)
        {
            case SIN:
                val = create_node (OPER, SIN, NULL, val, error);
                return val;
            case COS:
                val = create_node (OPER, COS, NULL, val, error);
                return val;
            case LN:
                val = create_node (OPER, LN, NULL, val, error);
                return val;
            case SQRT:
                val = create_node (OPER, SQRT, NULL, val, error);
                return val;
            case EXP:
                val = create_node (OPER, EXP, NULL, val, error);
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
    while (TOK[*n_tok].type == OPER && (TOK[*n_tok].elem.oper == MUL
    || TOK[*n_tok].elem.oper == DIV))
    {
        int old_n_tok = *n_tok;
        (*n_tok)++;

        if (couple_mis_op (*n_tok, TOK) == true)
        {
            *error = DIF_SYNTAX_ERROR;
            printf ("Введено некорректное выражение: оператор \"%s\" сразу после оператора \"%s\"."
                    "Работа программы завершена досрочно :(\n", get_oper_name (TOK[*n_tok].elem.oper),
                                                                get_oper_name (TOK[*n_tok - 1].elem.oper));
            tree_dtor (val);
            return NULL;
        }

        Node* val2 = get_s (error, TOK, n_tok);

        if (TOK[old_n_tok].elem.oper == MUL)
            val = create_node (OPER, MUL, val, val2, error);
        else
            val = create_node (OPER, DIV, val, val2, error);
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
        return create_node (NUM, val, NULL, NULL, error);
    }
    else if (TOK[*n_tok].type == VAR)
    {
        n_var = TOK[*n_tok].elem.n_var;
        (*n_tok)++;
        return create_node (VAR, n_var, NULL, NULL, error);
    }
    else
    {
        *error = DIF_SYNTAX_ERROR;
        return NULL;
    }
}
