#include "diff.h"

#define DL diff (node->left, error)
#define DR diff (node->right, error)
#define CL copy (node->left)
#define CR copy (node->right)
#define LEFT_NUM node->left->value.number
#define RIGHT_NUM node->right->value.number

Node* dif_add (const Node* node, enum DifError* error)
{
    return create_node (OPER, ADD, DL, DR, error);
}
Node* dif_sub  (const Node* node, enum DifError* error)
{
    return create_node (OPER, SUB, DL, DR, error);
}
Node* dif_mul  (const Node* node, enum DifError* error)
{
    Node* du = DL;
    Node* cu = CL;
    Node* dv = DR;
    Node* cv = CR;
    Node* res = create_node (OPER, ADD,
    create_node (OPER, MUL, du, cv, error),
    create_node (OPER, MUL, cu, dv, error), error);
    return res;
}
Node* dif_div  (const Node* node, enum DifError* error)
{
    Node* cu = CL;
    Node* cv1 = CR;
    Node* cv2 = CR;
    Node* du = DL;
    Node* dv = DR;
    Node* res = create_node (OPER, DIV,
    create_node (OPER, SUB,
    create_node (OPER, MUL, du, cv1, error),
    create_node (OPER, MUL, dv, cu, error), error),
    create_node (OPER, POW, cv2, create_node (NUM, 0, NULL, NULL, error), error), error);
    return res;
}
Node* dif_sin  (const Node* node, enum DifError* error)
{
    Node* cu = CR;
    Node* du = DR;
    Node* res = create_node (OPER, MUL,
    create_node (OPER, COS, NULL, cu, error), du, error);
    return res;
}
Node* dif_cos  (const Node* node, enum DifError* error)
{
    Node* cu = CR;
    Node* du = DR;
    Node* res = create_node (OPER, MUL,
                    create_node (OPER, MUL,
                        create_node (OPER, SIN, NULL, cu, error),
                        create_node (NUM, -1, NULL, NULL, error), error),
                    du, error);
    return res;
}
Node* dif_ln   (const Node* node, enum DifError* error)
{
    Node* cu = CR;
    Node* du = DR;
    Node* res = create_node (OPER, DIV, du, cu, error);
    return res;
}
Node* dif_sqrt (const Node* node, enum DifError* error)
{
    Node* cu = copy (node);
    Node* v = create_node (OPER, MUL, create_node (NUM, 2, NULL, NULL, error), cu, error);
    Node* h = create_node (OPER, DIV, create_node (NUM, 1, NULL, NULL, error), v, error);
    Node* dv = DR;
    Node* res = create_node (OPER, MUL, dv, h, error);
    return res;
}
Node* dif_pow  (const Node* node, enum DifError* error)
{
    if (node->right->type == NUM && node->left->type == NUM)
    {
        return create_node (NUM, 0, NULL, NULL, error);
    }
    else if (node->right->type == NUM)
    {
        Node* cu = CL;
        Node* du = DL;
        Node* pre_res = create_node (OPER, MUL, create_node (NUM, RIGHT_NUM, NULL, NULL, error),
        create_node (OPER, POW, cu, create_node (NUM, RIGHT_NUM - 1, NULL, NULL, error), error), error);
        return create_node (OPER, MUL, pre_res, du, error);
    }
    else if (node->left->type == NUM)
    {
        Node* cu = copy (node);
        Node* du = DR;
        Node* ln_num = create_node (OPER, LN, NULL, node->left, error);
        return create_node (OPER, MUL, cu,
        create_node (OPER, MUL, du, ln_num, error), error);
    }
    else
    {
        Node* cu = CL;
        Node* cv = CR;
        Node* du = DL;
        Node* dv = DR;
        Node* res1 = create_node (OPER, MUL, dv, create_node (OPER, LN, NULL, cu, error), error);
        Node* res2 = create_node (OPER, MUL, du, cv, error);
        Node* res3 = create_node (OPER, DIV, res2, copy (cu), error);
        Node* res4 = create_node (OPER, ADD, res1, res3, error);
        Node* res5 = create_node (OPER, POW, copy (cu), copy (cv), error);
        return create_node (OPER, MUL, res5, res4, error);
    }
}
Node* dif_exp (const Node* node, enum DifError* error)
{
    Node* du = DR;
    Node* cu = CR;
    return create_node (OPER, MUL, create_node (OPER, EXP, NULL, cu, error), du, error);
}
Node* eval_add  (Node* node, enum DifError* error)
{
    Node* res = create_node (NUM, LEFT_NUM + RIGHT_NUM, NULL, NULL, error);
    tree_dtor (node);
    return res;

    /*node->type = NUM;
    node->value.number = LEFT_NUM + RIGHT_NUM;
    tree_dtor (node->left);
    tree_dtor(node->right);
    return node;*/
}
Node* eval_sub  (Node* node, enum DifError* error)
{
    Node* res = create_node (NUM, LEFT_NUM - RIGHT_NUM, NULL, NULL, error);
    tree_dtor (node);
    return res;
}
Node* eval_mul  (Node* node, enum DifError* error)
{
    Node* res = create_node (NUM, LEFT_NUM * RIGHT_NUM, NULL, NULL, error);
    tree_dtor (node);
    return res;
}
Node* eval_div  (Node* node, enum DifError* error)
{
    if (compare_doubles (RIGHT_NUM, 0))
    {
        *error = DIF_DIV_NUL;
        return node;
    }

    Node* res = create_node (NUM, LEFT_NUM / RIGHT_NUM, NULL, NULL, error);
    tree_dtor (node);
    return res;
}
Node* eval_pow  (Node* node, enum DifError* error)
{
    Node* res = create_node (NUM, pow (LEFT_NUM, RIGHT_NUM), NULL, NULL, error);
    tree_dtor (node);
    return res;
}
Node* eval_sin  (Node* node, enum DifError* error)
{
    Node* res = create_node (NUM, sin (RIGHT_NUM), NULL, NULL, error);
    tree_dtor (node);
    return res;
}
Node* eval_cos  (Node* node, enum DifError* error)
{
    Node* res = create_node (NUM, cos (RIGHT_NUM), NULL, NULL, error);
    tree_dtor (node);
    return res;
}
Node* eval_ln (Node* node, enum DifError* error)
{
    if (compare_doubles (RIGHT_NUM, 0))
    {
        *error = DIF_DIV_NUL;
        return node;
    }

    Node* res = create_node (NUM, log (RIGHT_NUM), NULL, NULL, error);
    tree_dtor (node);
    return res;
}
Node* eval_sqrt (Node* node, enum DifError* error)
{
    Node* res = create_node (NUM, sqrt (RIGHT_NUM), NULL, NULL, error);
    tree_dtor (node);
    return res;
}
Node* eval_exp  (Node* node, enum DifError* error)
{
    Node* res = create_node (NUM, exp (RIGHT_NUM), NULL, NULL, error);
    tree_dtor (node);
    return res;
}
/*
Node* smp_add  (Node* node);
;
; Node* smp_sub  (Node* node); ;
; Node* smp_mul  (Node* node); ;
; Node* smp_div  (Node* node); ;
; Node* smp_sin  (Node* node); ;
; Node* smp_cos  (Node* node); ;
; Node* smp_ln   (Node* node); ;
; Node* smp_sqrt (Node* node); ;
; Node* smp_pow  (Node* node); ;
; Node* smp_exp  (Node* node); ;*/
