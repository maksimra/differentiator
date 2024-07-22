#include "../include/parser.h"

#define CUR_TYPE TOK[*n_tok].type
#define CUR_OPER TOK[*n_tok].elem.oper
#define PREV_OPER TOK[*n_tok - 1].elem.oper

Node* parse (enum DifError* error, struct Tokens* TOK, int* n_tok)
{
    Node* val = get_e (error, TOK, n_tok);

    if ((CUR_TYPE != TXT) ||
        (CUR_TYPE == TXT && TOK[*n_tok].elem.symb != '$'))
    {
        printf ("Не смогли обработать выражение :(\n");
        *error = DIF_SYNTAX_ERROR;
    }

    return val;
}

Node* get_e (enum DifError* error, struct Tokens* TOK, int* n_tok)
{
    Node* val = NULL;
    if (CUR_TYPE == OPER && CUR_OPER == SUB)
    {
        (*n_tok)++;

        if (couple_oper (*n_tok, TOK) == true)
        {
            *error = DIF_SYNTAX_ERROR;
            printf ("Введено некорректное выражение: оператор \"%s\" сразу после оператора \"%s\"."
                    "Работа программы завершена досрочно :(\n", get_oper_name (CUR_OPER),
                                                                get_oper_name (PREV_OPER));
            return NULL;
        }

        val = create_node (OPER, MUL, create_node (NUM, -1, NULL, NULL, error), get_t (error, TOK, n_tok), error);
    }
    else
    {
        val = get_t (error, TOK, n_tok);
    }
    while (CUR_TYPE == OPER && (CUR_OPER == ADD ||
    CUR_OPER == SUB))
    {
        int old_n_tok = (*n_tok);
        (*n_tok)++;

        if (couple_oper (*n_tok, TOK) == true)
        {
            *error = DIF_SYNTAX_ERROR;
            printf ("Введено некорректное выражение: оператор \"%s\" сразу после оператора \"%s\"."
                    "Работа программы завершена досрочно :(\n", get_oper_name (CUR_OPER),
                                                                get_oper_name (PREV_OPER));
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

Node* get_k (enum DifError* error, struct Tokens* TOK, int* n_tok)
{
    Node* val = get_p (error, TOK, n_tok);
    if (CUR_TYPE == OPER && CUR_OPER == POW)
    {
        (*n_tok)++;

        if (couple_oper (*n_tok, TOK) == true)
        {
            *error = DIF_SYNTAX_ERROR;
            printf ("Введено некорректное выражение: оператор \"%s\" сразу после оператора \"%s\"."
                    "Работа программы завершена досрочно :(\n", get_oper_name (CUR_OPER),
                                                                get_oper_name (PREV_OPER));
            tree_dtor (val);
            return NULL;
        }

        Node* val2 = get_p (error, TOK, n_tok);
        val = create_node (OPER, POW, val, val2, error);

        if (CUR_TYPE == OPER && CUR_OPER == POW)
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
    if (CUR_TYPE == OPER)
    {
        int n_oper = CUR_OPER;
        (*n_tok)++;

        if (CUR_TYPE == OPER)
        {
            *error = DIF_SYNTAX_ERROR;
            printf ("Введено некорректное выражение: оператор \"%s\" сразу после оператора \"%s\"."
                    "Работа программы завершена досрочно :(\n", get_oper_name (CUR_OPER),
                                                                get_oper_name (PREV_OPER));
            return NULL;
        }

        if (CUR_TYPE == TXT && TOK[*n_tok].elem.symb != '(')
        {
            *error = DIF_SYNTAX_ERROR;
            printf ("Введено некорректное выражение:"
                    "после оператора %s ожидается '(' "
                    "(пробелы допускаются).\n", get_oper_name (PREV_OPER));
            return NULL;
        }

        (*n_tok)++;
        Node* val = get_e (error, TOK, n_tok);

        if (CUR_TYPE == TXT && TOK[*n_tok].elem.symb != ')')
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
            case ADD:
            case SUB:
            case MUL:
            case DIV:
            case POW:
            default:
                *error = DIF_FUNC_ERROR;
        }
    }
    return get_k (error, TOK, n_tok);
}

Node* get_p (enum DifError* error, struct Tokens* TOK, int* n_tok)
{
    if (CUR_TYPE == TXT && TOK[*n_tok].elem.symb == '(')
    {
        (*n_tok)++;
        Node* val = get_e (error, TOK, n_tok);

        if (CUR_TYPE == TXT && TOK[*n_tok].elem.symb != ')')
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
    while (CUR_TYPE == OPER && (CUR_OPER == MUL
    || CUR_OPER == DIV))
    {
        int old_n_tok = *n_tok;
        (*n_tok)++;

        if (couple_oper (*n_tok, TOK) == true)
        {
            *error = DIF_SYNTAX_ERROR;
            printf ("Введено некорректное выражение: оператор \"%s\" сразу после оператора \"%s\"."
                    "Работа программы завершена досрочно :(\n", get_oper_name (CUR_OPER),
                                                                get_oper_name (PREV_OPER));
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

    if (CUR_TYPE == NUM)
    {
        val = TOK[*n_tok].elem.num;
        (*n_tok)++;
        return create_node (NUM, val, NULL, NULL, error);
    }
    else if (CUR_TYPE == VAR)
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

bool couple_oper (int n_tok, struct Tokens* TOK)
{
    if (TOK[n_tok].type == OPER)
        if (!OP[(int) TOK[n_tok].elem.oper].is_func)
            return true;

    return false;
}
