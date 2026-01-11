#include "EX.hpp"
#include "LX.hpp"
#include "UT.hpp"
#include <cassert>

namespace EX
{
E
Parser::parse_min_precedence_arithmetic_op (EX::Type type, size_t &idx)
{
  UT_FAIL_IF (not(EX::Type::Add == type || EX::Type::Sub == type));
  E result = E::OK;

  if (this->match_token_type (idx + 1, LX::Type::Int, LX::Type::Group))
  {
    if (this->match_token_type (idx + 2, LX::Type::Mult, LX::Type::Modulus))
    {
      if (EX::Type::Sub == type)
      {
        parse_binop (EX::Type::Add, idx, this->m_end);
        idx += this->m_end + 1;
      }
      else
      {
        parse_binop (type, idx + 1, this->m_end);
        idx += this->m_end + 1;
      }
    }
    else
    {
      this->parse_binop (type, idx + 1, idx + 2);
      idx += 2;
    }
  }
  else if (this->match_token_type (idx + 1, LX::Type::Minus))
  {
    parse_binop (type, idx + 1, idx + 2);
    idx += 3;
  }
  else
  {
    UT_FAIL_IF ("Unreachable branch reached (LX::Type::Plus)"); //
  }

  return result;
}

E
Parser::parse_max_precedence_arithmetic_op (EX::Type type, size_t &idx)
{
  E result = E::OK;

  if (this->match_token_type (idx + 1, LX::Type::Int, LX::Type::Group))
  {
    this->parse_binop (type, idx + 1, idx + 2);
    idx += 2;
  }
  else if (this->match_token_type (idx + 1, LX::Type::Minus))
  {
    parse_binop (type, idx + 1, idx + 2);
    idx += 3;
  }
  else
  {
    UT_FAIL_IF ("Unreachable branch reached (LX::Type::Mult)"); //
  }

  return result;
}

EX::Expr
Parser::alloc_subexpr (size_t n)
{
  EX::Expr expr{};
  if (n) { expr.exprs = EX::Exprs{ this->m_arena, n }; }

  return expr;
};

E
Parser::parse_binop (EX::Type type, size_t start, size_t end)
{
  E result = E::OK;

  EX::Expr root_expr = this->alloc_subexpr (2);
  EX::Expr left = *this->m_exprs.last ();
  root_expr.m_type = type;

  EX::Parser new_parser{ *this, start, end };
  result = new_parser.run ();

  EX::Expr right = *new_parser.m_exprs.last ();
  root_expr.exprs[0] = left;
  root_expr.exprs[1] = right;

  *this->m_exprs.last () = root_expr;

  return result;
};

E
Parser::run ()
{
  E e{};

  for (size_t i = this->m_begin; i < this->m_end;)
  {
    LX::Token t = this->m_tokens[i];

    switch (t.m_type)
    {
    case LX::Type::Int:
    {
      EX::Expr expr = this->alloc_subexpr (0);
      expr.m_type = EX::Type::Int;
      expr.m_int = t.as.m_int;
      this->m_exprs.push (expr);

      i += 1;
    }
    break;
    case LX::Type::Group:
    {
      Parser group_parser{ *this, t.as.m_tokens };
      group_parser.run ();
      this->m_exprs.push (*group_parser.m_exprs.last ());

      i += 1;
    }
    break;
    case LX::Type::Plus:
    {
      this->parse_min_precedence_arithmetic_op (EX::Type::Add, i);
    }
    break;
    case LX::Type::Mult:
    {
      this->parse_max_precedence_arithmetic_op (EX::Type::Mult, i);
    }
    break;
    case LX::Type::Div:
    {
      this->parse_max_precedence_arithmetic_op (EX::Type::Div, i);
    }
    break;
    case LX::Type::Modulus:
    {
      this->parse_max_precedence_arithmetic_op (EX::Type::Modulus, i);
    }
    break;
    case LX::Type::Minus:
    {
      if (this->m_exprs.is_empty ()
          || this->match_token_type (i - 1,
                                     LX::Type::Mult,
                                     LX::Type::Plus,
                                     LX::Type::Div,
                                     LX::Type::Modulus)) // The minus is unary
      {
        UT_FAIL_IF (not this->match_token_type (
            i + 1, LX::Type::Group, LX::Type::Int));

        EX::Parser new_parser{ *this, i + 1, i + 2 };
        new_parser.run ();

        EX::Expr expr = this->alloc_subexpr (1);
        expr.m_type = EX::Type::Minus;
        expr.exprs[0] = *new_parser.m_exprs.last ();

        this->m_exprs.push (expr);
        i += 2;
      }
      else // Binary minus
      {
        UT_FAIL_IF (not this->match_token_type (
            i + 1, LX::Type::Group, LX::Type::Int));

        parse_min_precedence_arithmetic_op (EX::Type::Sub, i);
      }
    }
    break;
    default: break;
    }
  }

  return e;
}
} // namespace EX
