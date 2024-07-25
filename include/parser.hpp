#ifndef PARSER_H
#define PARSER_H

#include "diff.hpp"

Node* get_e       (DifError* error, Tokens* tok, int* n_tok);
Node* get_k       (DifError* error, Tokens* tok, int* n_tok);
Node* get_s       (DifError* error, Tokens* tok, int* n_tok);
Node* get_p       (DifError* error, Tokens* tok, int* n_tok);
Node* get_t       (DifError* error, Tokens* tok, int* n_tok);
Node* get_n       (DifError* error, Tokens* tok, int* n_tok);
bool  couple_oper (int n_tok, Tokens* tok);
Node* parse       (DifError* error, Tokens* tok, int* n_tok);

#endif // PARSER_H
