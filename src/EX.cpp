#include "EX.hpp"
#include "LX.hpp"
#include "UT.hpp"
#include <cassert>
#include <cstdio>

namespace EX
{
E
Parser::parse_min_precedence_arithmetic_op (EX::Type type, size_t &idx)
{
  UT_FAIL_IF (not(EX::Type::Add == type || EX::Type::Sub == type));
  E result = E::OK;

  if (this->match_token_type (idx + 1, LX::Type::Int, LX::Type::Group,
                              LX::Type::Word))
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

  if (this->match_token_type (idx + 1, LX::Type::Int, LX::Type::Group,
                              LX::Type::Word))
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

E
Parser::parse_binop (EX::Type type, size_t start, size_t end)
{
  E result = E::OK;

  EX::Expr root_expr{ type, this->m_arena };
  EX::Expr left    = *this->m_exprs.last ();
  root_expr.m_type = type;

  EX::Parser new_parser{ *this, start, end };
  result = new_parser.run ();

  EX::Expr right        = *new_parser.m_exprs.last ();
  root_expr.as.exprs[0] = left;
  root_expr.as.exprs[1] = right;

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
      EX::Expr expr{ EX::Type::Int };
      expr.as.m_int = t.as.m_int;
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
    case LX::Type::Word:
    {
      // TODO: There should be a function application EX type and we should
      // check for it here
      EX::Expr expr{ EX::Type::Var };
      expr.as.m_var = t.as.m_string;
      this->m_exprs.push (expr);
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
          || this->match_token_type (i - 1, LX::Type::Mult, LX::Type::Plus,
                                     LX::Type::Div,
                                     LX::Type::Modulus)) // The minus is unary
      {
        UT_FAIL_IF (not this->match_token_type (
            i + 1, LX::Type::Group, LX::Type::Int, LX::Type::Word));

        EX::Parser new_parser{ *this, i + 1, i + 2 };
        new_parser.run ();

        EX::Expr expr{ Type::Minus, this->m_arena };
        expr.as.exprs[0] = *new_parser.m_exprs.last ();

        this->m_exprs.push (expr);
        i += 2;
      }
      else // Binary minus
      {
        UT_FAIL_IF (not this->match_token_type (
            i + 1, LX::Type::Group, LX::Type::Int, LX::Type::Word));

        parse_min_precedence_arithmetic_op (EX::Type::Sub, i);
      }
    }
    break;
    case LX::Type::Let:
    {
      // TODO: We should have a function application explicitly!
      // let var = body_expr in app_expr
      UT::String param = t.as.m_let_in.var_name;

      EX::Parser body_parser{ *this, t.as.m_let_in.in_tokens };
      body_parser.run ();
      EX::Expr body_expr = *body_parser.m_exprs.last ();

      EX::Parser app_parser{ *this, t.as.m_let_in.let_tokens };
      app_parser.run ();
      EX::Expr app_expr = *app_parser.m_exprs.last ();

      EX::Expr fn_expr{ EX::Type::FnDef };
      fn_expr.as.m_fn.param = param;
      fn_expr.as.m_fn.flags = EX::FnFlagEnum::FN_MUST_INLINE;
      fn_expr.as.m_fn.body  = { this->m_arena };
      fn_expr.as.m_fn.body.push (body_expr);

      this->m_exprs.push (fn_expr);
      this->m_exprs.push (app_expr);

      i += 1;
    }
    break;
    case LX::Type::Fn:
    {
      // \<var> = <expr>
      UT::String param = t.as.m_fn.var_name;

      EX::Parser body_parser{ *this, t.as.m_fn.body };
      body_parser.run ();
      EX::Expr body_expr = *body_parser.m_exprs.last ();

      EX::Expr fn_expr{ EX::Type::FnDef };
      fn_expr.as.m_fn.param = param;
      fn_expr.as.m_fn.flags = EX::FnFlagEnum::FN_MUST_INLINE;
      fn_expr.as.m_fn.body  = { this->m_arena };
      fn_expr.as.m_fn.body.push (body_expr);

      this->m_exprs.push (fn_expr);

      i += 1;
    }
    break;
    case LX::Type::Min:
    case LX::Type::Max:
    default:
    {
      UT_FAIL_IF ("The case default should be unreachable");
    }
    break;
    }
  }

  return e;
}
} // namespace EX
