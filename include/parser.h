#include "diff.h"

Node*         get_e                    (enum DifError* error, struct Tokens* TOK, int* n_tok);
Node*         get_k                    (enum DifError* error, struct Tokens* TOK, int* n_tok);
Node*         get_s                    (enum DifError* error, struct Tokens* TOK, int* n_tok);
Node*         get_p                    (enum DifError* error, struct Tokens* TOK, int* n_tok);
Node*         get_t                    (enum DifError* error, struct Tokens* TOK, int* n_tok);
Node*         get_n                    (enum DifError* error, struct Tokens* TOK, int* n_tok);
bool          couple_oper            (int n_tok, struct Tokens* TOK);
Node*         parse                    (enum DifError* error, struct Tokens* TOK, int* n_tok);
