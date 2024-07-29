#include "../include/parser.hpp"

#define CUR_TYPE tok[n_tok].type
#define CUR_OPER tok[n_tok].elem.oper
#define PREV_OPER tok[n_tok - 1].elem.oper

static int n_tok = 0;

Node* parse (DifError* error, const Tokens* tok)
{
    Node* val = get_e (error, tok);

    if ((CUR_TYPE != TXT) ||
        (CUR_TYPE == TXT && tok[n_tok].elem.symbol != '$'))
    {
        fprintf (stderr, "Не смогли обработать выражение :(\n");
        *error = DIF_ERROR_SYNTAX;
    }

    return val;
}

Node* get_e (DifError* error, const Tokens* tok)
{
    Node* val = NULL;
    if (CUR_TYPE == OPER && CUR_OPER == SUB)
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

        val = create_node (OPER, MUL, create_node (NUM, -1, NULL, NULL, error), get_t (error, tok), error);
    }
    else
    {
        val = get_t (error, tok);
    }
    while (CUR_TYPE == OPER && (CUR_OPER == ADD ||
    CUR_OPER == SUB))
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
        if (tok[old_n_tok].elem.oper == ADD)
            val = create_node (OPER, ADD, val, val2, error);
        else
            val = create_node (OPER, SUB, val, val2, error);
    }
    return val;
}

Node* get_k (DifError* error, const Tokens* tok)
{
    Node* val = get_p (error, tok);
    if (CUR_TYPE == OPER && CUR_OPER == POW)
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
        val = create_node (OPER, POW, val, val2, error);

        if (CUR_TYPE == OPER && CUR_OPER == POW)
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
    if (CUR_TYPE == OPER)
    {
        int n_oper = CUR_OPER;
        n_tok++;

        if (CUR_TYPE == OPER)
        {
            *error = DIF_ERROR_SYNTAX;
            fprintf (stderr, "Введено некорректное выражение: оператор \"%s\" сразу после оператора \"%s\"."
                             "Работа программы завершена досрочно :(\n", get_oper_name (CUR_OPER),
                                                                         get_oper_name (PREV_OPER));
            return NULL;
        }

        if (CUR_TYPE == TXT && tok[n_tok].elem.symbol != '(')
        {
            *error = DIF_ERROR_SYNTAX;
            fprintf (stderr, "Введено некорректное выражение:"
                             "после оператора %s ожидается '(' "
                             "(пробелы допускаются).\n", get_oper_name (PREV_OPER));
            return NULL;
        }

        n_tok++;
        Node* val = get_e (error, tok);

        if (CUR_TYPE == TXT && tok[n_tok].elem.symbol != ')')
            *error = DIF_ERROR_SYNTAX;

        n_tok++;

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
            case ADD:
            case SUB:
            case MUL:
            case DIV:
            case POW:
            default:
                *error = DIF_ERROR_FUNC;
        }
    }
    return get_k (error, tok);
}

Node* get_p (DifError* error, const Tokens* tok)
{
    if (CUR_TYPE == TXT && tok[n_tok].elem.symbol == '(')
    {
        n_tok++;
        Node* val = get_e (error, tok);

        if (CUR_TYPE == TXT && tok[n_tok].elem.symbol != ')')
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
    while (CUR_TYPE == OPER && (CUR_OPER == MUL
    || CUR_OPER == DIV))
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

        if (tok[old_n_tok].elem.oper == MUL)
            val = create_node (OPER, MUL, val, val2, error);
        else
            val = create_node (OPER, DIV, val, val2, error);
    }
    return val;
}

Node* get_n (DifError* error, const Tokens* tok)
{
    double val = 0;
    int n_var = -1;

    if (CUR_TYPE == NUM)
    {
        val = tok[n_tok].elem.num;
        n_tok++;
        return create_node (NUM, val, NULL, NULL, error);
    }
    else if (CUR_TYPE == VAR)
    {
        n_var = tok[n_tok].elem.n_var;
        n_tok++;
        return create_node (VAR, n_var, NULL, NULL, error);
    }
    else
    {
        *error = DIF_ERROR_SYNTAX;
        return NULL;
    }
}

bool couple_oper (const Tokens* tok)
{
    if (tok[n_tok].type == OPER)
        if (!OP[(int) tok[n_tok].elem.oper].is_func)
            return true;

    return false;
}
