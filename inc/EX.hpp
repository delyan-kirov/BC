#ifndef EX_HEADER
#define EX_HEADER

#include "LX.hpp"
#include "UT.hpp"
#include <cstdint>
#include <cstdio>
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
  FnDef,
  FnApp,
  VarApp,
  Var,
  If,
};

struct Expr;
using Exprs = UT::Vec<Expr>;

enum class FnFlags : std::uint64_t
{
  MIN            = 0,
  NONE           = 0,
  FN_MUST_INLINE = 1 << 0,
  MAX            = FN_MUST_INLINE,
};

inline std::uint64_t
operator&(
  FnFlags l, FnFlags r)
{
  return (std::uint64_t)l & (std::uint64_t)r;
}

inline std::uint64_t
operator|(
  FnFlags l, FnFlags r)
{
  return (std::uint64_t)l | (std::uint64_t)r;
}

inline std::uint64_t
operator^(
  FnFlags l, FnFlags r)
{
  return (std::uint64_t)l ^ (std::uint64_t)r;
}

struct FnDef
{
  FnFlags    m_flags;
  UT::String m_param;
  Exprs      m_body;

  FnDef() = default;
  FnDef(
    FnFlags flags, UT::String param, AR::Arena &arena)
      : m_flags{ flags },
        m_param{ param }
  {
    this->m_body = { arena };
  }
};

struct If
{
  Exprs m_condition;
  Exprs m_true_branch;
  Exprs m_else_branch;
};

struct FnApp
{
  FnDef m_body;
  Exprs m_param;
};

struct VarApp
{
  UT::String m_fn_name;
  Exprs      m_param;
};

struct Expr
{
  Type m_type;
  union
  {
    FnDef      m_fn;
    FnApp      m_fnapp;
    VarApp     m_varapp;
    UT::String m_var;
    Exprs      exprs;
    ssize_t    m_int = 0;
    If         m_if;
  } as;

  Expr() = default;
  Expr(Type type)
      : m_type{ type } {};
  Expr(
    Type type, AR::Arena &arena)
      : m_type{ type }
  {
    switch (type)
    {
    case Type::FnDef : this->as.m_fn.m_body = { arena }; break;
    case Type::FnApp : this->as.m_fnapp.m_param = { arena }; break;
    case Type::VarApp: this->as.m_varapp.m_param = { arena }; break;
    case Type::If:
    {
      this->as.m_if.m_condition   = { arena };
      this->as.m_if.m_else_branch = { arena };
      this->as.m_if.m_true_branch = { arena };
    }
    break;
    case Type::Div:
    case Type::Sub:
    case Type::Modulus:
    case Type::Mult:
    case Type::Add    : this->as.exprs = { arena, 2 }; break;
    case Type::Minus  : this->as.exprs = { arena, 1 }; break;
    default           : UT_FAIL_IF("Invalid type for this constructor");
    }
  };
};

class Parser
{
public:
  AR::Arena       &m_arena;
  ER::Events       m_events;
  const char      *m_input;
  const LX::Tokens m_tokens;
  size_t           m_begin;
  size_t           m_end;
  Exprs            m_exprs;

  Parser(
    LX::Lexer l)
      : m_arena{ l.m_arena },
        m_events{ std::move(l.m_events) },
        m_input{ l.m_input },
        m_tokens{ std::move(l.m_tokens) },
        m_begin{ 0 },
        m_end{ 0 },
        m_exprs{ l.m_arena }
  {
    this->m_end = this->m_tokens.m_len;
  };

  Parser(EX::Parser old, size_t begin, size_t end)
      : m_arena{ old.m_arena },   //
        m_events{ old.m_arena },  //
        m_input{ old.m_input },   //
        m_tokens{ old.m_tokens }, //
        m_begin{ begin },         //
        m_end{ end },             //
        m_exprs{ old.m_arena }    //
  {};

  Parser(EX::Parser &parent_parser, LX::Tokens &t)
      : m_arena{ parent_parser.m_arena },  //
        m_events{ parent_parser.m_arena }, //
        m_input{ parent_parser.m_input },  //
        m_tokens{ t },                     //
        m_begin{ 0 },                      //
        m_end{ t.m_len },                  //
        m_exprs{ parent_parser.m_arena }   //
  {};

  E run();

  E parse_binop(EX::Type type, size_t start, size_t end);

  E parse_max_precedence_arithmetic_op(EX::Type, size_t &idx);
  E parse_min_precedence_arithmetic_op(EX::Type, size_t &idx);

  bool
  match_token_type(
    size_t start, const LX::Type type)
  {
    // NOTE: here, we NEED to check that start index is in bounds
    LX::Type m_type = this->m_tokens[start].m_type;
    if (this->m_tokens.m_len <= start)
    {
      return false; //
    }
    else
    {
      UT_FAIL_IF(LX::Type::Max <= m_type || LX::Type::Min >= m_type);
      return type == m_type;
    }
  }

  template <typename... Args>
  bool
  match_token_type(
    size_t start, Args &&...args)
  {
    static_assert((std::is_same_v<std::decay_t<Args>, LX::Type> && ...),
                  "[TYPE-ERROR] All extra arguments must be LX::Type");
    return (... || this->match_token_type(start, args));
  }
};

} // namespace EX

namespace std
{
inline string
to_string(
  EX::Type expr_type)
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
  case EX::Type::FnDef  : return "EX::Type::FnDef";
  case EX::Type::Var    : return "EX::Type::Var";
  case EX::Type::FnApp  : return "EX::Type::FnApp";
  case EX::Type::VarApp : return "EX::Type::VarApp";
  case EX::Type::If     : return "EX::Type::If";
  case EX::Type::Unknown: return "EX::Type::Unknown";
  }

  UT_FAIL_IF("UNREACHABLE");
  return "";
}

inline string to_string(EX::FnDef fndef);

inline string
to_string(
  EX::Expr expr)
{
  string s{ "" };

  switch (expr.m_type)
  {
  case EX::Type::Int:
  {
    s = std::to_string(expr.as.m_int);
  }
  break;
  case EX::Type::Add:
  {
    s += "(";
    s += to_string(expr.as.exprs[0]);
    s += " + ";
    s += to_string(expr.as.exprs[1]);
    s += ")";
  }
  break;
  case EX::Type::Minus:
  {
    s += "-(";
    s += to_string(expr.as.exprs[0]);
    s += ")";
  }
  break;
  case EX::Type::Sub:
  {
    s += "(";
    s += to_string(expr.as.exprs[0]);
    s += " - ";
    s += to_string(expr.as.exprs[1]);
    s += ")";
  }
  break;
  case EX::Type::Mult:
  {
    s += "(";
    s += to_string(expr.as.exprs[0]);
    s += " * ";
    s += to_string(expr.as.exprs[1]);
    s += ")";
  }
  break;
  case EX::Type::Div:
  {
    s += "(";
    s += to_string(expr.as.exprs[0]);
    s += " / ";
    s += to_string(expr.as.exprs[1]);
    s += ")";
  }
  break;
  case EX::Type::Modulus:
  {
    s += "(";
    s += to_string(expr.as.exprs[0]);
    s += " % ";
    s += to_string(expr.as.exprs[1]);
    s += ")";
  }
  break;
  case EX::Type::FnDef:
  {
    s += "( \\" + to_string(expr.as.m_fn.m_param) + " = "
         + to_string(*expr.as.m_fn.m_body.last()) + " )";
  }
  break;
  case EX::Type::FnApp:
  {
    s += to_string(expr.as.m_fnapp.m_body) + " (" + " "
         + to_string(*expr.as.m_fnapp.m_param.last()) + " )";
  }
  break;
  case EX::Type::VarApp:
  {
    s += to_string(expr.as.m_varapp.m_fn_name) + " (" + " "
         + to_string(*expr.as.m_varapp.m_param.last()) + " )";
  }
  break;
  case EX::Type::Var:
  {
    s += "Var (" + std::to_string(expr.as.m_var) + ")";
  }
  break;
  case EX::Type::If:
  {
    s += "if " + std::to_string(*expr.as.m_if.m_condition.last()) +    //
         " => " + std::to_string(*expr.as.m_if.m_true_branch.last()) + //
         " else " + std::to_string(*expr.as.m_if.m_else_branch.last());
  }
  break;
  case EX::Type::Unknown:
  {
    s += "EX::T::Unknown";
  }
  break;
  default:
  {
    // TODO: Don't use default case here, fail under switch
    UT_FAIL_IF("UNREACHABLE");
  }
  break;
  }

  return s;
}
inline string
to_string(
  EX::FnDef fndef)
{
  return "(\\" + to_string(fndef.m_param) + " = "
         + to_string(*fndef.m_body.last()) + ")";
}
} // namespace std

#endif // EX_HEADER
