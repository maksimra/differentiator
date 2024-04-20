#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h> // зашить команду систем для графиза (не работает!!!)
#include <stdbool.h>
#include <assert.h> // пофиксить, что в случае одной ноды ничего не рисуется
#include <math.h>

typedef const char* chr;


/*struct
{
    enum,
    union {char*, double, char, enum (для оператора)}
}
*/

union Elem
{
    double num;
    char symb;
    int n_oper;
    int n_var;
};


enum OPER
{
    ADD = 0,
    SUB = 1,
    MUL = 2,
    DIV = 3,
    SIN = 4,
    COS = 5,
    LN = 6,
    SQRT = 7,
    DEG = 8,
    EXP = 9
};

struct Ops
{
    enum OPER op_enum;
    int number;
    const char* name;
    long len;
};

struct Vars
{
    char name;
    int num;
};

union Value
{
    int n_oper;
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
    DIF_ERROR_FOPEN,
    DIF_FUNC_ERROR,
    DIF_SYNTAX_ERROR,
    DIF_DIV_NUL
};

const struct Vars VARS[] =
{
    {'x', 0},
    {'y', 1},
    {'z', 2}
};

const struct Ops OP[] =
{
    {ADD, 0, "+", 1},
    {SUB, 1, "-", 1},
    {MUL, 2, "*", 1},
    {DIV, 3, "/", 1},
    {SIN, 4, "sin", 3},
    {COS, 5, "cos", 3},
    {LN, 6, "ln", 2},
    {SQRT, 7, "sqrt", 4},
    {DEG, 8, "^", 1},
    {EXP, 9, "exp", 3}
};

const int N_FUNC = sizeof (OP) / sizeof (OP[0]);

enum DifError create_tree       (char* buffer, Node** cur_node, int* pos);
enum DifError dif_read_tree         (FILE* file, const char* NAME, Node** root);
int           space_counter     (char* line);
enum DifError new_node          (Node** node);
int           search_oper       (const char* str, long len);
void          dif_tree_print    (Node* node, FILE* file, int* n_space);
void          printing_branches (Node* node, FILE* file, int* n_space);
void          printf_str        (FILE* file, Node* node, int n_space);
enum DifError dif_set_log_file  (FILE* file);
const char*   dif_get_error     (enum DifError error);
void          dif_print_error   (enum DifError error);
enum DifError check_argc        (const int argc, int necessary_n_arg);
void          tree_dtor         (Node* root);
Node*         copy              (const Node* node);
Node*         create_node       (enum Type type, double value, Node* left, Node* right);
enum DifError graphviz          (Node* node, FILE* file);
void          print_start       (FILE* file);
void          print_end         (FILE* file);
void          print_filling     (Node* node, FILE* file);
void          draw_right        (Node* node, FILE* file);
void          draw_left         (Node* node, FILE* file);
Node*         diff              (const Node* node);
enum DifError read_file         (const char* NAME, int* size);
int           search_var        (void);
enum DifError token             (struct Tokens* TOK);
void          skip_space        (char** str);
int           dif_search_func   (const char* name, size_t len);
void          tokin_dump        (struct Tokens* TOK, int n_tok);
Node*         get_g             (enum DifError* error, struct Tokens* TOK, int* n_tok);
Node*         get_e             (enum DifError* error, struct Tokens* TOK, int* n_tok);
Node*         get_k             (enum DifError* error, struct Tokens* TOK, int* n_tok);
Node*         get_s             (enum DifError* error, struct Tokens* TOK, int* n_tok);
Node*         get_p             (enum DifError* error, struct Tokens* TOK, int* n_tok);
Node*         get_t             (enum DifError* error, struct Tokens* TOK, int* n_tok);
Node*         get_n             (enum DifError* error, struct Tokens* TOK, int* n_tok);
Node*         simplification    (Node* node, enum DifError* error);
Node*         nul_and_one       (Node* node, bool* change, enum DifError* error);
Node*         swertka_const     (Node* node, bool* change);
