#ifndef EX_HEADER
#define EX_HEADER

#include "AR.hpp"
#include "LX.hpp"
#include <iostream>
#include <limits>
#include <string>

namespace EX
{

constexpr size_t PARSER_FAILED = std::numeric_limits<size_t>::max ();

enum class Type
{
  Int,
  Plus,
  Minus,
  Unknown
};

struct T
{
  Type m_type;
  T *m_left;
  T *m_right;
  int m_int;
};

typedef std::vector<LX::T> Tokens;

size_t parse (const Tokens &tokens, // in
              AR::T &arena,         // in
              size_t begin,         // in
              size_t end,           // in
              EX::T *expr           // out
);
}

namespace std
{
inline string
to_string (EX::Type expr_type)
{
  switch (expr_type)
  {
  case EX::Type::Int    : return "Expr_t::Integer";
  case EX::Type::Minus  : return "Expr_t::Minus";
  case EX::Type::Plus   : return "Expr_t::Plus";
  case EX::Type::Unknown:
  default               : return "Expr_t::Unknown";
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

  switch (expr->m_type)
  {
  case EX::Type::Int:
  {
    s = std::to_string (expr->m_int);
  }
  break;
  case EX::Type::Plus:
  {
    s += "(";
    s += to_string (expr->m_left);
    s += " + ";
    s += to_string (expr->m_right);
    s += ")";
  }
  break;
  case EX::Type::Minus:
  {
    s += "(";
    s += to_string (expr->m_left);
    s += " - ";
    s += to_string (expr->m_right);
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

#endif // EX_HEADER
