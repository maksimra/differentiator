#include "../include/parser.h"

Node* parse (enum DifError* error, struct Tokens* TOK, int* n_tok)
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

bool couple_mis_op (int n_tok, struct Tokens* TOK)
{
    if (TOK[n_tok].type == OPER)
        if (!OP[(int) TOK[n_tok].elem.oper].is_func)
            return true;

    return false;
}
