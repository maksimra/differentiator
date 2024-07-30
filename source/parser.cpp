#include "../include/parser.hpp"

#define CUR_TYPE tok[n_tok].type
#define CUR_OPER tok[n_tok].elem.oper
#define PREV_OPER tok[n_tok - 1].elem.oper

static int n_tok = 0;

Node* parse (DifError* error, const Tokens* tok)
{
    Node* val = get_e (error, tok);

    if ((CUR_TYPE != TYPE_TXT) ||
        (CUR_TYPE == TYPE_TXT && tok[n_tok].elem.symbol != '$'))
    {
        fprintf (stderr, "Не смогли обработать выражение :(\n");
        *error = DIF_ERROR_SYNTAX;
    }

    return val;
}

Node* get_e (DifError* error, const Tokens* tok)
{
    Node* val = NULL;
    if (CUR_TYPE == TYPE_OPER && CUR_OPER == OPER_SUB)
    {
        n_tok++;

        if (couple_oper (tok) == true)
        {
            *error = DIF_ERROR_SYNTAX;
            fprintf (stderr, "Введено некорректное выражение: оператор \"%s\" сразу после оператора \"%s\"."
                             "Работа программы завершена досрочно :(\n", get_oper_name (CUR_OPER),
                                                                         get_oper_name (PREV_OPER));
            return NULL;
        }

        val = create_node (TYPE_OPER, OPER_MUL, create_node (TYPE_NUM, -1, NULL, NULL, error), get_t (error, tok), error);
    }
    else
    {
        val = get_t (error, tok);
    }
    while (CUR_TYPE == TYPE_OPER && (CUR_OPER == OPER_ADD ||
    CUR_OPER == OPER_SUB))
    {
        int old_n_tok = n_tok;
        n_tok++;

        if (couple_oper (tok) == true)
        {
            *error = DIF_ERROR_SYNTAX;
            fprintf (stderr, "Введено некорректное выражение: оператор \"%s\" сразу после оператора \"%s\"."
                             "Работа программы завершена досрочно :(\n", get_oper_name (CUR_OPER),
                                                                         get_oper_name (PREV_OPER));
            tree_dtor (val);
            return NULL;
        }

        Node* val2 = get_t (error, tok);
        if (tok[old_n_tok].elem.oper == OPER_ADD)
            val = create_node (TYPE_OPER, OPER_ADD, val, val2, error);
        else
            val = create_node (TYPE_OPER, OPER_SUB, val, val2, error);
    }
    return val;
}

Node* get_k (DifError* error, const Tokens* tok)
{
    Node* val = get_p (error, tok);
    if (CUR_TYPE == TYPE_OPER && CUR_OPER == OPER_POW)
    {
        n_tok++;

        if (couple_oper (tok) == true)
        {
            *error = DIF_ERROR_SYNTAX;
            fprintf (stderr, "Введено некорректное выражение: оператор \"%s\" сразу после оператора \"%s\"."
                             "Работа программы завершена досрочно :(\n", get_oper_name (CUR_OPER),
                                                                         get_oper_name (PREV_OPER));
            tree_dtor (val);
            return NULL;
        }

        Node* val2 = get_p (error, tok);
        val = create_node (TYPE_OPER, OPER_POW, val, val2, error);

        if (CUR_TYPE == TYPE_OPER && CUR_OPER == OPER_POW)
        {
            *error = DIF_ERROR_SYNTAX;
            fprintf (stderr, "Введено некорректное выражение: "
                             "используйте скобки для записи.\n");
        }
    }
    return val;
}

Node* get_s (DifError* error, const Tokens* tok)
{
    if (CUR_TYPE == TYPE_OPER)
    {
        int n_oper = CUR_OPER;
        n_tok++;

        if (CUR_TYPE == TYPE_OPER)
        {
            *error = DIF_ERROR_SYNTAX;
            fprintf (stderr, "Введено некорректное выражение: оператор \"%s\" сразу после оператора \"%s\"."
                             "Работа программы завершена досрочно :(\n", get_oper_name (CUR_OPER),
                                                                         get_oper_name (PREV_OPER));
            return NULL;
        }

        if (CUR_TYPE == TYPE_TXT && tok[n_tok].elem.symbol != '(')
        {
            *error = DIF_ERROR_SYNTAX;
            fprintf (stderr, "Введено некорректное выражение:"
                             "после оператора %s ожидается '(' "
                             "(пробелы допускаются).\n", get_oper_name (PREV_OPER));
            return NULL;
        }

        n_tok++;
        Node* val = get_e (error, tok);

        if (CUR_TYPE == TYPE_TXT && tok[n_tok].elem.symbol != ')')
            *error = DIF_ERROR_SYNTAX;

        n_tok++;

        switch (OPER_ARRAY[n_oper].op_enum)
        {
            case OPER_SIN:
                val = create_node (TYPE_OPER, OPER_SIN, NULL, val, error);
                return val;
            case OPER_COS:
                val = create_node (TYPE_OPER, OPER_COS, NULL, val, error);
                return val;
            case OPER_LN:
                val = create_node (TYPE_OPER, OPER_LN, NULL, val, error);
                return val;
            case OPER_SQRT:
                val = create_node (TYPE_OPER, OPER_SQRT, NULL, val, error);
                return val;
            case OPER_EXP:
                val = create_node (TYPE_OPER, OPER_EXP, NULL, val, error);
                return val;
            case OPER_ADD:
            case OPER_SUB:
            case OPER_MUL:
            case OPER_DIV:
            case OPER_POW:
            case OPER_NONE:
            default:
                *error = DIF_ERROR_FUNC;
        }
    }
    return get_k (error, tok);
}

Node* get_p (DifError* error, const Tokens* tok)
{
    if (CUR_TYPE == TYPE_TXT && tok[n_tok].elem.symbol == '(')
    {
        n_tok++;
        Node* val = get_e (error, tok);

        if (CUR_TYPE == TYPE_TXT && tok[n_tok].elem.symbol != ')')
            *error = DIF_ERROR_SYNTAX;

        n_tok++;

        return val;
    }
    Node* val = get_n (error, tok);
    return val;
}

Node* get_t (DifError* error, const Tokens* tok)
{
    Node* val = get_s (error, tok);
    while (CUR_TYPE == TYPE_OPER && (CUR_OPER == OPER_MUL
    || CUR_OPER == OPER_DIV))
    {
        int old_n_tok = n_tok;
        n_tok++;

        if (couple_oper (tok) == true)
        {
            *error = DIF_ERROR_SYNTAX;
            fprintf (stderr, "Введено некорректное выражение: оператор \"%s\" сразу после оператора \"%s\"."
                             "Работа программы завершена досрочно :(\n", get_oper_name (CUR_OPER),
                                                                         get_oper_name (PREV_OPER));
            tree_dtor (val);
            return NULL;
        }

        Node* val2 = get_s (error, tok);

        if (tok[old_n_tok].elem.oper == OPER_MUL)
            val = create_node (TYPE_OPER, OPER_MUL, val, val2, error);
        else
            val = create_node (TYPE_OPER, OPER_DIV, val, val2, error);
    }
    return val;
}

Node* get_n (DifError* error, const Tokens* tok)
{
    double val = 0;
    int n_var = -1;

    if (CUR_TYPE == TYPE_NUM)
    {
        val = tok[n_tok].elem.num;
        n_tok++;
        return create_node (TYPE_NUM, val, NULL, NULL, error);
    }
    else if (CUR_TYPE == TYPE_VAR)
    {
        n_var = tok[n_tok].elem.n_var;
        n_tok++;
        return create_node (TYPE_VAR, n_var, NULL, NULL, error);
    }
    else
    {
        *error = DIF_ERROR_SYNTAX;
        return NULL;
    }
}

bool couple_oper (const Tokens* tok)
{
    if (tok[n_tok].type == TYPE_OPER)
        if (!OPER_ARRAY[(int) tok[n_tok].elem.oper].is_func)
            return true;

    return false;
}
