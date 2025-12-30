#ifndef EX_HEADER
#define EX_HEADER

#include "AR.hpp"
#include "LX.hpp"
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace EX
{

constexpr size_t PARSER_FAILED = std::numeric_limits<size_t>::max ();

enum class Type
{
  Unknown = 0,
  Int,
  Minus,
  Add,
  Sub,
  Mult,
  Div,
  Modulus,
};

struct T
{
  Type m_type;
  T *m_left;
  T *m_right;
  int m_int;
};

using Tokens = std::vector<LX::T>;

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
  case EX::Type::Int    : return "EX::Type::Int";
  case EX::Type::Minus  : return "EX::Type::Minus";
  case EX::Type::Sub    : return "EX::Type::Sub";
  case EX::Type::Add    : return "EX::Type::Add";
  case EX::Type::Mult   : return "EX::Type::Mult";
  case EX::Type::Div    : return "EX::Type::Div";
  case EX::Type::Modulus: return "EX::Type::Modulus";
  case EX::Type::Unknown: return "EX::Type::Unknown";
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
  case EX::Type::Add:
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
    s += "-(";
    s += to_string (expr->m_left);
    s += ")";
  }
  break;
  case EX::Type::Sub:
  {
    s += "(";
    s += to_string (expr->m_left);
    s += " - ";
    s += to_string (expr->m_right);
    s += ")";
  }
  break;
  case EX::Type::Mult:
  {
    s += "(";
    s += to_string (expr->m_left);
    s += " * ";
    s += to_string (expr->m_right);
    s += ")";
  }
  break;
  case EX::Type::Div:
  {
    s += "(";
    s += to_string (expr->m_left);
    s += " / ";
    s += to_string (expr->m_right);
    s += ")";
  }
  break;
  case EX::Type::Modulus:
  {
    s += "(";
    s += to_string (expr->m_left);
    s += " % ";
    s += to_string (expr->m_right);
    s += ")";
  }
  break;
  case EX::Type::Unknown:
  {
    s += "EX::T::Unknown";
  }
  break;
  }

  return s;
}
}

#endif // EX_HEADER
