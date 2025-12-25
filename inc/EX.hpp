#ifndef PARSER_HEADER
#define PARSER_HEADER

#include "LX.hpp"
#include <iostream>
#include <string>

namespace EX
{

enum class Type
{
  Integer,
  Plus,
  Minus,
  Unknown
};

struct T
{
  Type type;
  T *left;
  T *right;
  int integer;
};

void parse (std::vector<LX::T> &tokens, // in
            size_t begin,                      // in
            size_t &end,                       // out
            size_t stop,                       // in
            T *expr                            // out
);
}

namespace std
{
inline string
to_string (EX::Type expr_type)
{
  switch (expr_type)
  {
  case EX::Type::Integer: return "Expr_t::Integer";
  case EX::Type::Minus  : return "Expr_t::Minus";
  case EX::Type::Plus   : return "Expr_t::Plus";
  case EX::Type::Unknown:
  default                 : return "Expr_t::Unknown";
  }
}

inline string to_string (EX::T *expr);

inline string
to_string (EX::T *expr)
{
  string s{ "" };

  if (!expr)
  {
    std::cerr << "ERROR: expression given is null";
    return "";
  }

  switch (expr->type)
  {
  case EX::Type::Integer:
  {
    s = std::to_string (expr->integer);
  }
  break;
  case EX::Type::Plus:
  {
    s += "(";
    s += to_string (expr->left);
    s += " + ";
    s += to_string (expr->right);
    s += ")";
  }
  break;
  case EX::Type::Minus:
  {
    s += "(";
    s += to_string (expr->left);
    s += " - ";
    s += to_string (expr->right);
    s += ")";
  }
  break;
  case EX::Type::Unknown:
  default:
  {
    s += "Expr_t::Unknown";
  }
  break;
  }

  return s;
}
}

#endif // PARSER_HEADER
