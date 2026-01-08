#ifndef EX_HEADER
#define EX_HEADER

#include "AR.hpp"
#include "LX.hpp"
#include <string>

namespace EX
{

enum class E
{
  MIN = -1,
  OK,
  MAX
};

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

struct T;
using Exprs = UT::V<T>;
struct T
{
  Exprs exprs;
  ssize_t m_int;
  Type m_type;
};

class Parser
{
public:
  AR::T &m_arena;
  ER::T m_events;
  const char *m_input;
  const LX::Tokens m_tokens;
  size_t m_begin;
  size_t m_end;
  Exprs m_exprs;

  Parser (LX::L l)
      : m_arena{ l.m_arena },               //
        m_events{ std::move (l.m_events) }, //
        m_input{ l.m_input },               //
        m_tokens{ std::move (l.m_tokens) }, //
        m_begin{ 0 },                       //
        m_end{ 0 },                         //
        m_exprs{ l.m_arena }                //
  {
    this->m_end = this->m_tokens.m_len;
  };

  Parser (EX::Parser old, size_t begin, size_t end)
      : m_arena{ old.m_arena },   //
        m_events{ old.m_arena },  //
        m_input{ old.m_input },   //
        m_tokens{ old.m_tokens }, //
        m_begin{ begin },         //
        m_end{ end },             //
        m_exprs{ old.m_arena }    //
  {};

  E run ();

  EX::T alloc_subexpr (size_t n);
};

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
to_string (EX::T expr)
{
  string s{ "" };

  switch (expr.m_type)
  {
  case EX::Type::Int:
  {
    s = std::to_string (expr.m_int);
  }
  break;
  case EX::Type::Add:
  {
    s += "(";
    s += to_string (expr.exprs[0]);
    s += " + ";
    s += to_string (expr.exprs[1]);
    s += ")";
  }
  break;
  case EX::Type::Minus:
  {
    s += "-(";
    s += to_string (expr.exprs[0]);
    s += ")";
  }
  break;
  case EX::Type::Sub:
  {
    s += "(";
    s += to_string (expr.exprs[0]);
    s += " - ";
    s += to_string (expr.exprs[1]);
    s += ")";
  }
  break;
  case EX::Type::Mult:
  {
    s += "(";
    s += to_string (expr.exprs[0]);
    s += " * ";
    s += to_string (expr.exprs[1]);
    s += ")";
  }
  break;
  case EX::Type::Div:
  {
    s += "(";
    s += to_string (expr.exprs[0]);
    s += " / ";
    s += to_string (expr.exprs[1]);
    s += ")";
  }
  break;
  case EX::Type::Modulus:
  {
    s += "(";
    s += to_string (expr.exprs[0]);
    s += " % ";
    s += to_string (expr.exprs[1]);
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
