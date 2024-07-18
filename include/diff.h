#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <unistd.h>

enum OPER
{
    ADD =  0,
    SUB =  1,
    MUL =  2,
    DIV =  3,
    SIN =  4,
    COS =  5,
    LN =   6,
    SQRT = 7,
    POW =  8,
    EXP =  9
};

union Elem
{
    double num;
    char symb;
    enum OPER oper;
    int n_var;
};


struct Vars
{
    char* name;
    size_t len;
    int num;
};

union Value
{
    enum OPER oper;
    double number;
    int n_var;
};

enum Type
{
    OPER,
    NUM,
    VAR,
    TXT
};

struct Tokens
{
    enum Type type;
    union Elem elem;
};

struct Node
{
    enum Type type;
    Value value;

    Node* left;
    Node* right;
};


enum DifError
{
    DIF_NO_ERROR,
    DIF_ERROR_CALLOC,
    DIF_ERROR_STAT,
    DIF_ERROR_READ,
    DIF_ERROR_FREAD,
    DIF_NULL_PTR_LOG,
    DIF_ERROR_STRTOD,
    DIF_ERROR_ARGC,
    DIF_ERROR_ARGV,
    DIF_ERROR_FOPEN,
    DIF_FUNC_ERROR,
    DIF_SYNTAX_ERROR,
    DIF_DIV_NUL,
    DIF_LN_NUL,
    DIF_FGETS_ERROR,
    DIF_ERROR_x0,
    DIF_ERROR_N,
};

struct Ops
{
    enum OPER op_enum;
    int number;
    const char* name;
    size_t len;
    Node* (*dif) (const Node* node, enum DifError* error);
    Node* (*eval) (Node* node, enum DifError* error);
    Node* (*smp) (Node* node, bool* change, enum DifError* error);
    bool is_func;
};

Node* dif_add (const Node* node, enum DifError* error);
Node* dif_sub (const Node* node, enum DifError* error);
Node* dif_mul (const Node* node, enum DifError* error);
Node* dif_div (const Node* node, enum DifError* error);
Node* dif_sin (const Node* node, enum DifError* error);
Node* dif_cos (const Node* node, enum DifError* error);
Node* dif_ln (const Node* node, enum DifError* error);
Node* dif_sqrt (const Node* node, enum DifError* error);;
Node* dif_pow (const Node* node, enum DifError* error);
Node* dif_exp (const Node* node, enum DifError* error);
Node* eval_add (Node* node, enum DifError* error);
Node* eval_sub (Node* node, enum DifError* error);
Node* eval_mul (Node* node, enum DifError* error);
Node* eval_div (Node* node, enum DifError* error);
Node* eval_pow (Node* node, enum DifError* error);
Node* eval_sin  (Node* node, enum DifError* error);
Node* eval_cos  (Node* node, enum DifError* error);
Node* eval_ln (Node* node, enum DifError* error);
Node* eval_sqrt (Node* node, enum DifError* error);
Node* eval_exp  (Node* node, enum DifError* error);
Node* smp_add (Node* node, bool* change, enum DifError* error);
Node* smp_sub (Node* node, bool* change, enum DifError* error);
Node* smp_mul (Node* node, bool* change, enum DifError* error);
Node* smp_div (Node* node, bool* change, enum DifError* error);
Node* smp_sin (Node* node, bool* change, enum DifError* error);
Node* smp_cos (Node* node, bool* change, enum DifError* error);
Node* smp_ln (Node* node, bool* change, enum DifError* error);
Node* smp_sqrt (Node* node, bool* change, enum DifError* error);
Node* smp_exp (Node* node, bool* change, enum DifError* error);
Node* smp_pow  (Node* node, bool* change, enum DifError* error);



const struct Ops OP[] =
{
    {ADD,  0, "+",    1, dif_add, eval_add, smp_add, false},
    {SUB,  1, "-",    1, dif_sub, eval_sub, smp_sub, false},
    {MUL,  2, "*",    1, dif_mul, eval_mul, smp_mul, false},
    {DIV,  3, "/",    1, dif_div, eval_div, smp_div, false},
    {SIN,  4, "sin",  3, dif_sin, eval_sin, smp_sin, true},
    {COS,  5, "cos",  3, dif_cos, eval_cos, smp_cos, true},
    {LN,   6, "ln",   2, dif_ln, eval_ln, smp_ln, true},
    {SQRT, 7, "sqrt", 4, dif_sqrt, eval_sqrt, smp_sqrt, true},
    {POW,  8, "^",    1, dif_pow, eval_pow, smp_pow, false},
    {EXP,  9, "exp",  3, dif_exp, eval_exp, smp_exp, true}
};

const int N_FUNC = sizeof (OP) / sizeof (OP[0]);

enum DifError create_tree              (char* buffer, Node** cur_node, int* pos);
enum DifError dif_read_tree            (FILE* file, const char* NAME, Node** root);
size_t        count_space              (const char* line);
enum DifError allocate_node            (Node** node);
int           search_oper              (const char* str, size_t len);
void          print_tree_txt_incr_tabs (Node* node, FILE* file, int* n_space);
void          print_node_or_decr_tabs  (Node* node, FILE* file, int* n_space);
void          printf_str               (FILE* file, const Node* node, int n_space);
enum DifError dif_set_log_file         (FILE* file);
const char*   dif_get_error            (enum DifError error);
void          dif_print_error          (enum DifError error);
enum DifError check_argc               (const int argc, int necessary_n_arg);
void          tree_dtor                (Node* root);
Node*         copy                     (const Node* node);
Node*         create_node              (enum Type type, double value, Node* left, Node* right, enum DifError* error);
enum DifError graphviz                 (Node* node, FILE* file, struct Vars* VARS);
void          print_start              (FILE* file);
void          print_end                (FILE* file);
void          print_connections            (Node* node, FILE* file, struct Vars* VARS);
void          draw_right               (Node* node, FILE* file, struct Vars* VARS);
void          draw_left                (Node* node, FILE* file, struct Vars* VARS);
Node*         diff                     (const Node* node, enum DifError* error);
enum DifError read_file                (const char* NAME, size_t* size);
int           search_var               (void);
enum DifError token                    (struct Tokens* TOK, struct Vars* VARS, int MAX_N_VARS);
void          skip_space               (char** str);
int           dif_search_func          (const char* name, size_t len);
void          tokin_dump               (struct Tokens* TOK, int n_tok, struct Vars* VARS);
Node*         get_g                    (enum DifError* error, struct Tokens* TOK, int* n_tok);
Node*         get_e                    (enum DifError* error, struct Tokens* TOK, int* n_tok);
Node*         get_k                    (enum DifError* error, struct Tokens* TOK, int* n_tok);
Node*         get_s                    (enum DifError* error, struct Tokens* TOK, int* n_tok);
Node*         get_p                    (enum DifError* error, struct Tokens* TOK, int* n_tok);
Node*         get_t                    (enum DifError* error, struct Tokens* TOK, int* n_tok);
Node*         get_n                    (enum DifError* error, struct Tokens* TOK, int* n_tok);
Node*         simplification           (Node* node, enum DifError* error);
Node*         nul_and_one              (Node* node, bool* change, enum DifError* error);
Node*         swertka_const            (Node* node, bool* change, enum DifError* error);
void          taylor                   (Node* node, enum DifError* error);
int           fact                     (int n);
Node*         n_diff                   (Node* node, int i);
Node*         change_x0                (Node* node, double x0);
bool          couple_mis_op            (int n_tok, struct Tokens* TOK);
const char*   get_oper_name            (enum OPER oper);
enum DifError check_args               (const int argc, const char* argv[]);
bool          compare_doubles          (double a, double b);
