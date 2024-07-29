#ifndef PARSER_H
#define PARSER_H

#include "diff.hpp"

Node* get_e       (DifError* error, const Tokens* tok);
Node* get_k       (DifError* error, const Tokens* tok);
Node* get_s       (DifError* error, const Tokens* tok);
Node* get_p       (DifError* error, const Tokens* tok);
Node* get_t       (DifError* error, const Tokens* tok);
Node* get_n       (DifError* error, const Tokens* tok);
bool  couple_oper (const Tokens* tok);
Node* parse       (DifError* error, const Tokens* tok);

#endif // PARSER_H
