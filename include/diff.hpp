#ifndef DIFF_H
#define DIFF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <unistd.h>

enum Operator
{
    OPER_NONE = 0,
    OPER_ADD  = 1,
    OPER_SUB  = 2,
    OPER_MUL  = 3,
    OPER_DIV  = 4,
    OPER_POW  = 5,
    OPER_SIN  = 6,
    OPER_COS  = 7,
    OPER_LN   = 8,
    OPER_SQRT = 9,
    OPER_EXP  = 10
};

union ExprElem
{
    double num;
    char symbol;
    Operator oper;
    int n_var;
};


struct Vars
{
    const char* name;
    size_t len;
    int num;
};

union Value
{
    Operator oper;
    double number;
    int n_var;
};

enum Type
{
    TYPE_OPER = 1,
    TYPE_NUM  = 2,
    TYPE_VAR  = 3,
    TYPE_TXT  = 4
};

struct Tokens
{
    Type type;
    union ExprElem elem;
};

struct Node
{
    Type type;
    Value value;

    Node* left;
    Node* right;
};


enum DifError
{
    DIF_NO_ERROR           = 0,
    DIF_ERROR_CALLOC       = 1,
    DIF_ERROR_STAT         = 2,
    DIF_ERROR_READ         = 3,
    DIF_ERROR_FREAD        = 4,
    DIF_ERROR_NULL_PTR_LOG = 5,
    DIF_ERROR_STRTOD       = 6,
    DIF_ERROR_ARGC         = 7,
    DIF_ERROR_ARGV         = 8,
    DIF_ERROR_FOPEN        = 9,
    DIF_ERROR_FUNC         = 10,
    DIF_ERROR_SYNTAX       = 11,
    DIF_ERROR_DIV_NUL      = 12,
    DIF_ERROR_LN_NUL       = 13,
    DIF_ERROR_FGETS        = 14,
    DIF_ERROR_x0           = 15,
    DIF_ERROR_N            = 16,
    DIF_ERROR_N_VARS       = 17
};

struct DifOps
{
    Operator op_enum;
    const char* name;
    size_t len;
    Node* (*dif)  (const Node* node, DifError* error);
    Node* (*eval) (Node* node, DifError* error);
    Node* (*smp)  (Node* node, bool* change, DifError* error);
    bool is_func;
};

Node* dif_add   (const Node* node, DifError* error);
Node* dif_sub   (const Node* node, DifError* error);
Node* dif_mul   (const Node* node, DifError* error);
Node* dif_div   (const Node* node, DifError* error);
Node* dif_sin   (const Node* node, DifError* error);
Node* dif_cos   (const Node* node, DifError* error);
Node* dif_ln    (const Node* node, DifError* error);
Node* dif_sqrt  (const Node* node, DifError* error);;
Node* dif_pow   (const Node* node, DifError* error);
Node* dif_exp   (const Node* node, DifError* error);
Node* eval_add  (Node* node, DifError* error);
Node* eval_sub  (Node* node, DifError* error);
Node* eval_mul  (Node* node, DifError* error);
Node* eval_div  (Node* node, DifError* error);
Node* eval_pow  (Node* node, DifError* error);
Node* eval_sin  (Node* node, DifError* error);
Node* eval_cos  (Node* node, DifError* error);
Node* eval_ln   (Node* node, DifError* error);
Node* eval_sqrt (Node* node, DifError* error);
Node* eval_exp  (Node* node, DifError* error);
Node* smp_add   (Node* node, bool* change, DifError* error);
Node* smp_sub   (Node* node, bool* change, DifError* error);
Node* smp_mul   (Node* node, bool* change, DifError* error);
Node* smp_div   (Node* node, bool* change, DifError* error);
Node* smp_sin   (Node* node, bool* change, DifError* error);
Node* smp_cos   (Node* node, bool* change, DifError* error);
Node* smp_ln    (Node* node, bool* change, DifError* error);
Node* smp_sqrt  (Node* node, bool* change, DifError* error);
Node* smp_exp   (Node* node, bool* change, DifError* error);
Node* smp_pow   (Node* node, bool* change, DifError* error);

const DifOps OPER_ARRAY[] =
{
    {OPER_NONE},
    {OPER_ADD,  "+",    1, dif_add,  eval_add,  smp_add,  false},
    {OPER_SUB,  "-",    1, dif_sub,  eval_sub,  smp_sub,  false},
    {OPER_MUL,  "*",    1, dif_mul,  eval_mul,  smp_mul,  false},
    {OPER_DIV,  "/",    1, dif_div,  eval_div,  smp_div,  false},
    {OPER_POW,  "^",    1, dif_pow,  eval_pow,  smp_pow,  false},
    {OPER_SIN,  "sin",  3, dif_sin,  eval_sin,  smp_sin,  true},
    {OPER_COS,  "cos",  3, dif_cos,  eval_cos,  smp_cos,  true},
    {OPER_LN,   "ln",   2, dif_ln,   eval_ln,   smp_ln,   true},
    {OPER_SQRT, "sqrt", 4, dif_sqrt, eval_sqrt, smp_sqrt, true},
    {OPER_EXP,  "exp",  3, dif_exp,  eval_exp,  smp_exp,  true}
};

Operator      search_oper              (const char* str, size_t len);
void          print_tree_txt_incr_tabs (Node* node, FILE* file);
void          print_node_or_decr_tabs  (Node* node, FILE* file, int* n_space);
void          printf_str               (FILE* file, const Node* node, int n_space);
DifError      dif_set_log_file         (FILE* file);
const char*   dif_get_error            (DifError error);
void          dif_print_error          (DifError error);
void          tree_dtor                (Node* root);
Node*         copy_tree                (const Node* node, DifError* error);
Node*         create_node              (Type type, double value, Node* left, Node* right, DifError* error);
void          graphviz                 (const Node* node, FILE* file, const Vars* vars);
void          print_start              (FILE* file);
void          print_end                (FILE* file);
void          print_connections        (const Node* node, FILE* file, const Vars* vars);
void          draw_right               (const Node* node, FILE* file, const Vars* vars);
void          draw_left                (const Node* node, FILE* file, const Vars* vars);
Node*         diff                     (const Node* node, DifError* error);
DifError      read_file                (FILE* file, const char* file_name, char** buffer, size_t* size);
DifError      token                    (Tokens* tok, Vars* vars, const char* buffer, int MAX_N_VARS);
void          skip_space               (const char** str);
void          token_dump               (const Tokens* tok, int n_tok, const Vars* vars);
Node*         simplification           (Node* node, DifError* error);
Node*         zeros_and_ones           (Node* node, bool* change, DifError* error);
Node*         count_const              (Node* node, bool* change, DifError* error);
DifError      taylor                   (const Node* node);
int           fact                     (int n);
Node*         change_x0                (Node* node, double x0);
const char*   get_oper_name            (Operator oper);
int           search_var               (Vars* vars, int n_vars, const char* begin, size_t len);
bool          try_char_operation       (Tokens* tok, int n_tok, const char** buffer);
bool          try_digit                (Tokens* tok, int n_tok, const char** buffer);
bool          try_parenthesis          (Tokens* tok, int n_tok, const char** buffer);
bool          try_function             (Tokens* tok, int n_tok, const char** buffer);
bool          try_var                  (Tokens* tok, int n_tok, Vars* vars, int* n_vars, int max_n_vars, const char** buffer, DifError* error);
void          fill_token_oper          (Tokens* tok, int n_tok, Operator oper);
size_t        fill_token_double        (Tokens* tok, int n_tok, const char* buffer);
Operator      search_char_operation    (const char* buffer);
DifError      print_decompose          (const Node* node, double x0, int accuracy);

#endif // DIFF_H
