#ifndef PARSER_HEADER
#define PARSER_HEADER

#include <iostream>
#include <string>
#include "tokenizer.hpp"

enum class Expr_t
{
  Integer,
  Plus,
  Minus,
  Unknown
};

struct Expr
{
  Expr_t type;
  Expr *left;
  Expr *right;
  int integer;
};

namespace std
{
inline string
to_string (Expr_t expr_type)
{
  switch (expr_type)
  {
  case Expr_t::Integer: return "Expr_t::Integer";
  case Expr_t::Minus  : return "Expr_t::Minus";
  case Expr_t::Plus   : return "Expr_t::Plus";
  case Expr_t::Unknown:
  default             : return "Expr_t::Unknown";
  }
}

inline string to_string (Expr *expr);

inline string
to_string (Expr *expr)
{
  string s{ "" };

  if (!expr)
  {
    std::cerr << "ERROR: expression given is null";
    return "";
  }

  switch (expr->type)
  {
  case Expr_t::Integer:
  {
    s = std::to_string (expr->integer);
  }
  break;
  case Expr_t::Plus:
  {
    s += "(";
    s += to_string (expr->left);
    s += " + ";
    s += to_string (expr->right);
    s += ")";
  }
  break;
  case Expr_t::Minus:
  {
    s += "(";
    s += to_string (expr->left);
    s += " - ";
    s += to_string (expr->right);
    s += ")";
  }
  break;
  case Expr_t::Unknown:
  default:
  {
    s += "Expr_t::Unknown";
  }
  break;
  }

  return s;
}
}

void
parse (std::vector<Token> &tokens, // in
       size_t begin,               // in
       size_t &end,                // out
       size_t stop,                // in
       Expr *expr                  // out
);

#endif // PARSER_HEADER
