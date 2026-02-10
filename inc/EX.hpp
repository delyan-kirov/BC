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

#define EX_Type_EnumVariants                                                   \
  X(Unknown)                                                                   \
  X(Int)                                                                       \
  X(Minus)                                                                     \
  X(Add)                                                                       \
  X(Sub)                                                                       \
  X(Mult)                                                                      \
  X(Div)                                                                       \
  X(Modulus)                                                                   \
  X(Let)                                                                       \
  X(IsEq)                                                                      \
  X(FnDef)                                                                     \
  X(FnApp)                                                                     \
  X(VarApp)                                                                    \
  X(Var)                                                                       \
  X(If)

enum class Type
{
#define X(X_enum) X_enum,
  EX_Type_EnumVariants
#undef X
};

struct Expr;
using Exprs = UT::Vec<Expr>;

enum FnFlags : std::uint64_t
{
  MIN            = 0,
  NONE           = 0,
  FN_MUST_INLINE = 1 << 0,
  MAX            = FN_MUST_INLINE,
};

// TODO: use Expr* not the vector Exprs
struct FnDef
{
  FnFlags    m_flags;
  UT::String m_param;
  Expr      *m_body;

  FnDef() = default;
  FnDef(
    FnFlags flags, UT::String param, AR::Arena &arena)
      : m_flags{ flags },
        m_param{ param }
  {
    this->m_body = (EX::Expr *)arena.alloc<EX::Expr>(1);
  }
};

struct If
{
  Expr *m_condition;
  Expr *m_true_branch;
  Expr *m_else_branch;
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

struct Let
{
  UT::String m_var_name;
  Expr      *m_value;
  Expr      *m_continuation;
};

struct Expr
{
  Type m_type;
  union
  {
    FnDef          m_fn;
    FnApp          m_fnapp;
    VarApp         m_varapp;
    UT::String     m_var;
    UT::Pair<Expr> m_pair;
    Expr          *m_expr;
    ssize_t        m_int = 0;
    If             m_if;
    Let            m_let;
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
    case Type::FnApp : this->as.m_fnapp.m_param = { arena }; break;
    case Type::VarApp: this->as.m_varapp.m_param = { arena }; break;
    case Type::FnDef:
      this->as.m_fn.m_body = (EX::Expr *)arena.alloc<EX::Expr>(1);
      break;
    case Type::If:
    {
      this->as.m_if.m_condition   = (Expr *)arena.alloc<Expr>(1);
      this->as.m_if.m_else_branch = (Expr *)arena.alloc<Expr>(1);
      this->as.m_if.m_true_branch = (Expr *)arena.alloc<Expr>(1);
    }
    break;
    case Type::Div:
    case Type::Sub:
    case Type::Modulus:
    case Type::Mult:
    case Type::IsEq:
    case Type::Add    : this->as.m_pair = { arena }; break;
    case Type::Minus  : this->as.m_expr = (Expr *)arena.alloc<Expr>(1); break;
    default           : UT_FAIL_IF("Invalid type for this constructor");
    }
  };
};

class Parser
{
  // TODO: use UT::String, not const char*
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

  Parser(LX::Tokens tokens, AR::Arena &arena, const char *input)
      : m_arena{ arena },
        m_events{ arena },
        m_input{ input },
        m_tokens{ tokens },
        m_begin{ 0 },
        m_end{ tokens.m_len },
        m_exprs{ arena } {};

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
#define X(X_enum)                                                              \
  case EX::Type::X_enum: return #X_enum;
    EX_Type_EnumVariants
  }
#undef X

  UT_FAIL_IF("UNREACHABLE");
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
    s += to_string(expr.as.m_pair.first());
    s += " + ";
    s += to_string(expr.as.m_pair.second());
    s += ")";
  }
  break;
  case EX::Type::Minus:
  {
    s += "-(";
    s += to_string(*expr.as.m_expr);
    s += ")";
  }
  break;
  case EX::Type::Sub:
  {
    s += "(";
    s += to_string(expr.as.m_pair.first());
    s += " - ";
    s += to_string(expr.as.m_pair.second());
    s += ")";
  }
  break;
  case EX::Type::Mult:
  {
    s += "(";
    s += to_string(expr.as.m_pair.first());
    s += " * ";
    s += to_string(expr.as.m_pair.second());
    s += ")";
  }
  break;
  case EX::Type::Div:
  {
    s += "(";
    s += to_string(expr.as.m_pair.first());
    s += " / ";
    s += to_string(expr.as.m_pair.second());
    s += ")";
  }
  break;
  case EX::Type::Modulus:
  {
    s += "(";
    s += to_string(expr.as.m_pair.first());
    s += " % ";
    s += to_string(expr.as.m_pair.second());
    s += ")";
  }
  break;
  case EX::Type::IsEq:
  {
    s += "(";
    s += to_string(expr.as.m_pair.first());
    s += " ?= ";
    s += to_string(expr.as.m_pair.second());
    s += ")";
  }
  break;
  case EX::Type::FnDef:
  {
    s += "( \\" + to_string(expr.as.m_fn.m_param) + " = "
         + to_string(*expr.as.m_fn.m_body) + " )";
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
    s += "if " + std::to_string(*expr.as.m_if.m_condition) +    //
         " => " + std::to_string(*expr.as.m_if.m_true_branch) + //
         " else " + std::to_string(*expr.as.m_if.m_else_branch);
  }
  break;
  case EX::Type::Unknown:
  {
    s += "EX::T::Unknown";
  }
  break;
  case EX::Type::Let:
  {
    s += "let " + to_string(expr.as.m_let.m_var_name) + " = "
         + to_string(*expr.as.m_let.m_value) + " in "
         + to_string(*expr.as.m_let.m_continuation);
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
  return "(\\" + to_string(fndef.m_param) + " = " + to_string(*fndef.m_body)
         + ")";
}
} // namespace std

#endif // EX_HEADER
