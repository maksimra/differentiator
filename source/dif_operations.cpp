#include "../include/diff.hpp"
#include "../include/compare_doubles.hpp"

#define DL              diff (node->left, error)
#define DR              diff (node->right, error)
#define CL              copy_tree (node->left, error)
#define CR              copy_tree (node->right, error)
#define LEFT_NUM        node->left->value.number
#define RIGHT_NUM       node->right->value.number
#define ARE_EQUAL(a,b)  compare_doubles (a,b) == 0
#define BOTH_NUMBER     node->right->type == TYPE_NUM && node->left->type == TYPE_NUM
#define RIGHT_IS_NUMBER node->right->type == TYPE_NUM
#define LEFT_IS_NUMBER  node->left->type  == TYPE_NUM

Node* dif_add (const Node* node, DifError* error)
{
    assert (node != NULL);
    return create_node (TYPE_OPER, OPER_ADD, DL, DR, error);
}

Node* dif_sub  (const Node* node, DifError* error)
{
    assert (node != NULL);
    return create_node (TYPE_OPER, OPER_SUB, DL, DR, error);
}

Node* dif_mul  (const Node* node, DifError* error)
{
    assert (node != NULL);
    return create_node (TYPE_OPER, OPER_ADD,
                        create_node (TYPE_OPER, OPER_MUL, DL, CR, error),
                        create_node (TYPE_OPER, OPER_MUL, CL, DR, error),
                        error);
}

Node* dif_div  (const Node* node, DifError* error)
{
    assert (node != NULL);
    return create_node (TYPE_OPER, OPER_DIV,
                        create_node (TYPE_OPER, OPER_SUB,
                                    create_node (TYPE_OPER, OPER_MUL, DL, CR, error),
                                    create_node (TYPE_OPER, OPER_MUL, DR, CL, error),
                                    error),
                        create_node (TYPE_OPER, OPER_POW,
                                    CR,
                                    create_node (TYPE_NUM, 2, NULL, NULL, error),
                                    error),
                        error);
}

Node* dif_sin  (const Node* node, DifError* error)
{
    assert (node != NULL);
    return create_node (TYPE_OPER, OPER_MUL,
                        create_node (TYPE_OPER, OPER_COS, NULL, CR, error),
                        DR,
                        error);
}

Node* dif_cos  (const Node* node, DifError* error)
{
    assert (node != NULL);
    return create_node (TYPE_OPER, OPER_MUL,
                        create_node (TYPE_OPER, OPER_MUL,
                                    create_node (TYPE_OPER, OPER_SIN, NULL, CR, error),
                                    create_node (TYPE_NUM, -1, NULL, NULL, error),
                                    error),
                        DR,
                        error);
}

Node* dif_ln   (const Node* node, DifError* error)
{
    assert (node != NULL);
    return create_node (TYPE_OPER, OPER_DIV, DR, CR, error);
}

Node* dif_sqrt (const Node* node, DifError* error)
{
    assert (node != NULL);
    return create_node (TYPE_OPER, OPER_MUL,
                        DR,
                        create_node (TYPE_OPER, OPER_DIV,
                                    create_node (TYPE_NUM, 1, NULL, NULL, error),
                                    create_node (TYPE_OPER, OPER_MUL,
                                                create_node (TYPE_NUM, 2, NULL, NULL, error),
                                                copy_tree (node, error),
                                                error),
                                    error),
                        error);
}

Node* dif_pow  (const Node* node, DifError* error)
{
    assert (node != NULL);
    if (BOTH_NUMBER)
    {
        return create_node (TYPE_NUM, 0, NULL, NULL, error);
    }
    else if (RIGHT_IS_NUMBER)
    {
        return create_node (TYPE_OPER, OPER_MUL,
                            create_node (TYPE_OPER, OPER_MUL,
                                        create_node (TYPE_NUM, RIGHT_NUM, NULL, NULL, error),
                                        create_node (TYPE_OPER, OPER_POW,
                                                    CL,
                                                    create_node (TYPE_NUM, RIGHT_NUM - 1, NULL, NULL, error),
                                                    error),
                                        error),
                            DL,
                            error);
    }
    else if (LEFT_IS_NUMBER)
    {
        return create_node (TYPE_OPER, OPER_MUL,
                            copy_tree (node, error),
                            create_node (TYPE_OPER, OPER_MUL,
                                        DR,
                                        create_node (TYPE_OPER, OPER_LN, NULL, CL, error),
                                        error),
                            error);
    }
    else
    {
        return create_node (TYPE_OPER, OPER_MUL,
                            create_node (TYPE_OPER, OPER_POW,
                                        copy_tree (CL, error),
                                        copy_tree (CR, error),
                                        error),
                            create_node (TYPE_OPER, OPER_ADD,
                                        create_node (TYPE_OPER, OPER_MUL,
                                                    DR,
                                                    create_node (TYPE_OPER, OPER_LN, NULL, CL, error),
                                                    error),
                                        create_node (TYPE_OPER, OPER_DIV,
                                                    create_node (TYPE_OPER, OPER_MUL, DL, CR, error),
                                                    copy_tree (CL, error),
                                                    error),
                                        error),
                            error);
    }
}

Node* dif_exp (const Node* node, DifError* error)
{
    assert (node != NULL);
    return create_node (TYPE_OPER, OPER_MUL, create_node (TYPE_OPER, OPER_EXP, NULL, CR, error), DR, error);
}

Node* eval_add  (Node* node, DifError* error)
{
    assert (node != NULL);
    Node* result = create_node (TYPE_NUM, LEFT_NUM + RIGHT_NUM, NULL, NULL, error);
    tree_dtor (node);
    return result;
}

Node* eval_sub  (Node* node, DifError* error)
{
    assert (node != NULL);
    Node* result = create_node (TYPE_NUM, LEFT_NUM - RIGHT_NUM, NULL, NULL, error);
    tree_dtor (node);
    return result;
}

Node* eval_mul  (Node* node, DifError* error)
{
    assert (node != NULL);
    Node* result = create_node (TYPE_NUM, LEFT_NUM * RIGHT_NUM, NULL, NULL, error);
    tree_dtor (node);
    return result;
}

Node* eval_div  (Node* node, DifError* error)
{
    assert (node != NULL);
    if (ARE_EQUAL(RIGHT_NUM, 0))
    {
        *error = DIF_ERROR_DIV_NUL;
        return node;
    }

    Node* result = create_node (TYPE_NUM, LEFT_NUM / RIGHT_NUM, NULL, NULL, error);
    tree_dtor (node);
    return result;
}

Node* eval_pow  (Node* node, DifError* error)
{
    assert (node != NULL);
    Node* result = create_node (TYPE_NUM, pow (LEFT_NUM, RIGHT_NUM), NULL, NULL, error);
    tree_dtor (node);
    return result;
}

Node* eval_sin  (Node* node, DifError* error)
{
    assert (node != NULL);
    Node* result = create_node (TYPE_NUM, sin (RIGHT_NUM), NULL, NULL, error);
    tree_dtor (node);
    return result;
}

Node* eval_cos  (Node* node, DifError* error)
{
    assert (node != NULL);
    Node* result = create_node (TYPE_NUM, cos (RIGHT_NUM), NULL, NULL, error);
    tree_dtor (node);
    return result;
}

Node* eval_ln (Node* node, DifError* error)
{
    assert (node != NULL);
    if (ARE_EQUAL(RIGHT_NUM, 0))
    {
        *error = DIF_ERROR_DIV_NUL;
        return node;
    }

    Node* result = create_node (TYPE_NUM, log (RIGHT_NUM), NULL, NULL, error);
    tree_dtor (node);
    return result;
}

Node* eval_sqrt (Node* node, DifError* error)
{
    assert (node != NULL);
    Node* result = create_node (TYPE_NUM, sqrt (RIGHT_NUM), NULL, NULL, error);
    tree_dtor (node);
    return result;
}

Node* eval_exp (Node* node, DifError* error)
{
    assert (node != NULL);
    Node* result = create_node (TYPE_NUM, exp (RIGHT_NUM), NULL, NULL, error);
    tree_dtor (node);
    return result;
}

Node* smp_add (Node* node, bool*, DifError* error)
{
    assert (node != NULL);
    *error = DIF_NO_ERROR;
    return node;
}

Node* smp_sub (Node* node, bool*, DifError* error)
{
    assert (node != NULL);
    *error = DIF_NO_ERROR;
    return node;
}

Node* smp_mul (Node* node, bool* change, DifError* error)
{
    assert (node != NULL);
    if (node->left != NULL && LEFT_IS_NUMBER && ARE_EQUAL (LEFT_NUM, 1))
    {
        *change = true;
        Node* new_node = copy_tree (node->right, error);
        tree_dtor (node);
        return new_node;
    }
    else if (node->right != NULL && RIGHT_IS_NUMBER && ARE_EQUAL(RIGHT_NUM, 1))
    {
        *change = true;
        Node* new_node = copy_tree (node->left, error);
        tree_dtor (node);
        return new_node;
    }
    else if ((node->left != NULL && LEFT_IS_NUMBER && ARE_EQUAL(LEFT_NUM, 0)) ||
            (node->right != NULL && RIGHT_IS_NUMBER && ARE_EQUAL(RIGHT_NUM, 0)))
    {
        *change = true;
        tree_dtor (node);
        return create_node (TYPE_NUM, 0, NULL, NULL, error);
    }
    return node;
}

Node* smp_div (Node* node, bool* change, DifError* error)
{
    assert (node != NULL);
    if (node->right != NULL && RIGHT_IS_NUMBER && ARE_EQUAL(RIGHT_NUM, 1))
    {
        *change = true;
        Node* new_node = copy_tree (node->left, error);
        tree_dtor (node);
        return new_node;
    }
    if (node->left != NULL && LEFT_IS_NUMBER && ARE_EQUAL(LEFT_NUM, 0))
    {
        *change = true;
        tree_dtor (node);
        return create_node (TYPE_NUM, 0, NULL, NULL, error);
    }
    if (node->right != NULL && RIGHT_IS_NUMBER && ARE_EQUAL(RIGHT_NUM, 0))
    {
        *error = DIF_ERROR_DIV_NUL;
        return node;
    }
    return node;
}

Node* smp_sin (Node* node, bool*, DifError* error)
{
    assert (node != NULL);
    *error = DIF_NO_ERROR;
    return node;
}

Node* smp_cos (Node* node, bool*, DifError* error)
{
    assert (node != NULL);
    *error = DIF_NO_ERROR;
    return node;
}

Node* smp_ln (Node* node, bool*, DifError* error)
{
    assert (node != NULL);
    *error = DIF_NO_ERROR;
    return node;
}

Node* smp_sqrt (Node* node, bool*, DifError* error)
{
    assert (node != NULL);
    *error = DIF_NO_ERROR;
    return node;
}

Node* smp_exp (Node* node, bool*, DifError* error)
{
    assert (node != NULL);
    *error = DIF_NO_ERROR;
    return node;
}

Node* smp_pow  (Node* node, bool* change, DifError* error)
{
    assert (node != NULL);
    if (node->left != NULL && LEFT_IS_NUMBER && ARE_EQUAL(LEFT_NUM, 1))
    {
        *change = true;
        tree_dtor (node);
        return create_node (TYPE_NUM, 1, NULL, NULL, error);
    }
    if (node->right != NULL && RIGHT_IS_NUMBER && ARE_EQUAL(RIGHT_NUM, 1))
    {
        *change = true;
        Node* new_node = copy_tree (node->left, error);
        tree_dtor (node);
        return new_node;
    }
    if (node->left != NULL && LEFT_IS_NUMBER && ARE_EQUAL(LEFT_NUM, 0))
    {
        *change = true;
        tree_dtor (node);
        return create_node (TYPE_NUM, 0, NULL, NULL, error);
    }
    if (node->right != NULL && RIGHT_IS_NUMBER && ARE_EQUAL(RIGHT_NUM, 0))
    {
        *change = true;
        tree_dtor (node);
        return create_node (TYPE_NUM, 1, NULL, NULL, error);
    }
    return node;
}
