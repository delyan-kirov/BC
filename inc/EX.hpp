#ifndef EX_HEADER
#define EX_HEADER

#include "LX.hpp"
#include "UT.hpp"
#include <string>
#include <type_traits>

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

struct Expr;
using Exprs = UT::Vec<Expr>;
struct Expr
{
  Exprs exprs;
  ssize_t m_int;
  Type m_type;
};

class Parser
{
public:
  AR::Arena &m_arena;
  ER::Events m_events;
  const char *m_input;
  const LX::Tokens m_tokens;
  size_t m_begin;
  size_t m_end;
  Exprs m_exprs;

  Parser (LX::Lexer l)
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

  Parser (EX::Parser &parent_parser, LX::Tokens &t)
      : m_arena{ parent_parser.m_arena },  //
        m_events{ parent_parser.m_arena }, //
        m_input{ parent_parser.m_input },  //
        m_tokens{ t },                     //
        m_begin{ 0 },                      //
        m_end{ t.m_len },                  //
        m_exprs{ parent_parser.m_arena }   //
  {};

  E run ();

  EX::Expr alloc_subexpr (size_t n);

  E parse_binop (EX::Type type, size_t start, size_t end);

  E parse_max_precedence_arithmetic_op (EX::Type, size_t &idx);
  E parse_min_precedence_arithmetic_op (EX::Type, size_t &idx);

  bool
  match_token_type (size_t start, const LX::Type type)
  {
    // NOTE: here, we NEED to check that start index is in bounds
    LX::Type m_type = this->m_tokens[start].m_type;
    if (this->m_tokens.m_len <= start)
    {
      return false; //
    }
    else
    {
      UT_FAIL_IF (LX::Type::Max <= m_type || LX::Type::Min >= m_type);
      return type == m_type;
    }
  }

  template <typename... Args>
  bool
  match_token_type (size_t start, Args &&...args)
  {
    static_assert ((std::is_same_v<std::decay_t<Args>, LX::Type> && ...),
                   "[TYPE-ERROR] All extra arguments must be LX::Type");
    return (... || this->match_token_type (start, args));
  }
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

inline string to_string (EX::Expr *expr);

inline string
to_string (EX::Expr expr)
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
