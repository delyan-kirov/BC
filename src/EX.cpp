#include "EX.hpp"
#include "LX.hpp"
#include "UT.hpp"
#include <cassert>
#include <cstdio>

namespace EX
{
E
Parser::parse_min_precedence_arithmetic_op(
  EX::Type type, size_t &idx)
{
  UT_FAIL_IF(not(EX::Type::Add == type || EX::Type::Sub == type));
  E result = E::OK;

  if (this->match_token_type(
        idx + 1, LX::Type::Int, LX::Type::Group, LX::Type::Word))
  {
    if (this->match_token_type(idx + 2, LX::Type::Mult, LX::Type::Modulus))
    {
      if (EX::Type::Sub == type)
      {
        parse_binop(EX::Type::Add, idx, this->m_end);
        idx += this->m_end + 1;
      }
      else
      {
        parse_binop(type, idx + 1, this->m_end);
        idx += this->m_end + 1;
      }
    }
    else
    {
      this->parse_binop(type, idx + 1, idx + 2);
      idx += 2;
    }
  }
  else if (this->match_token_type(idx + 1, LX::Type::Minus))
  {
    parse_binop(type, idx + 1, idx + 2);
    idx += 3;
  }
  else
  {
    UT_FAIL_IF("Unreachable branch reached (LX::Type::Plus)"); //
  }

  return result;
}

E
Parser::parse_max_precedence_arithmetic_op(
  EX::Type type, size_t &idx)
{
  E result = E::OK;

  if (this->match_token_type(
        idx + 1, LX::Type::Int, LX::Type::Group, LX::Type::Word))
  {
    this->parse_binop(type, idx + 1, idx + 2);
    idx += 2;
  }
  else if (this->match_token_type(idx + 1, LX::Type::Minus))
  {
    parse_binop(type, idx + 1, idx + 2);
    idx += 3;
  }
  else
  {
    UT_FAIL_IF("Unreachable branch reached (LX::Type::Mult)"); //
  }

  return result;
}

E
Parser::parse_binop(
  EX::Type type, size_t start, size_t end)
{
  E result = E::OK;

  EX::Expr root_expr{ type, this->m_arena };
  EX::Expr left    = *this->m_exprs.last();
  root_expr.m_type = type;

  EX::Parser new_parser{ *this, start, end };
  result = new_parser.run();

  EX::Expr right        = *new_parser.m_exprs.last();
  root_expr.as.exprs[0] = left;
  root_expr.as.exprs[1] = right;

  *this->m_exprs.last() = root_expr;

  return result;
};

E
Parser::run()
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
      this->m_exprs.push(expr);

      i += 1;
    }
    break;
    case LX::Type::Group:
    {
      Parser group_parser{ *this, t.as.m_tokens };
      group_parser.run();
      this->m_exprs.push(*group_parser.m_exprs.last());

      i += 1;
    }
    break;
    case LX::Type::Word:
    {
      i += 1;
      if (this->match_token_type(
            i, LX::Type::Group, LX::Type::Int, LX::Type::Fn, LX::Type::Word))
      {
        LX::Tokens next_token = { this->m_arena };
        next_token.push(this->m_tokens[i]);

        EX::Parser param_parser{ *this, next_token };
        param_parser.run();
        EX::Exprs param_expr = param_parser.m_exprs;

        EX::Expr var_app{ EX::Type::VarApp, this->m_arena };
        var_app.as.m_varapp.m_fn_name = t.as.m_string;
        var_app.as.m_varapp.m_param   = param_expr;

        this->m_exprs.push(var_app);
        i += 1;
      }
      else
      {
        EX::Expr var{ EX::Type::Var };
        var.as.m_var = t.as.m_string;
        this->m_exprs.push(var);
      }
    }
    break;
    case LX::Type::Plus:
    {
      this->parse_min_precedence_arithmetic_op(EX::Type::Add, i);
    }
    break;
    case LX::Type::Mult:
    {
      this->parse_max_precedence_arithmetic_op(EX::Type::Mult, i);
    }
    break;
    case LX::Type::Div:
    {
      this->parse_max_precedence_arithmetic_op(EX::Type::Div, i);
    }
    break;
    case LX::Type::Modulus:
    {
      this->parse_max_precedence_arithmetic_op(EX::Type::Modulus, i);
    }
    break;
    case LX::Type::Minus:
    {
      if (this->m_exprs.is_empty()
          || this->match_token_type(i - 1,
                                    LX::Type::Mult,
                                    LX::Type::Plus,
                                    LX::Type::Div,
                                    LX::Type::Modulus)) // The minus is unary
      {
        UT_FAIL_IF(not this->match_token_type(
          i + 1, LX::Type::Group, LX::Type::Int, LX::Type::Word));

        EX::Parser new_parser{ *this, i + 1, i + 2 };
        new_parser.run();

        EX::Expr expr{ Type::Minus, this->m_arena };
        expr.as.exprs[0] = *new_parser.m_exprs.last();

        this->m_exprs.push(expr);
        i += 2;
      }
      else // Binary minus
      {
        UT_FAIL_IF(not this->match_token_type(
          i + 1, LX::Type::Group, LX::Type::Int, LX::Type::Word));

        parse_min_precedence_arithmetic_op(EX::Type::Sub, i);
      }
    }
    break;
    case LX::Type::Let:
    {
      // TODO: We should have a function application explicitly!
      // let var = body_expr in app_expr
      UT::String param = t.as.m_let_in.m_var_name;

      EX::Parser body_parser{ *this, t.as.m_let_in.m_in_tokens };
      body_parser.run();
      EX::Expr body_expr = *body_parser.m_exprs.last();

      EX::Parser app_parser{ *this, t.as.m_let_in.m_let_tokens };
      app_parser.run();
      EX::Expr app_expr = *app_parser.m_exprs.last();

      EX::FnDef fn_def{};
      fn_def.m_flags = EX::FnFlags::FN_MUST_INLINE;
      fn_def.m_param = param;
      fn_def.m_body  = { this->m_arena };
      fn_def.m_body.push(body_expr);

      EX::Expr fn_app{ EX::Type::FnApp, this->m_arena };
      fn_app.as.m_fnapp.m_param.push(app_expr);
      fn_app.as.m_fnapp.m_body = fn_def;

      this->m_exprs.push(fn_app);

      i += 1;
    }
    break;
    case LX::Type::Fn:
    {
      // \<var> = <expr>
      UT::String param = t.as.m_fn.m_var_name;

      EX::Parser body_parser{ *this, t.as.m_fn.m_body };
      body_parser.run();
      EX::Expr body_expr = *body_parser.m_exprs.last();

      EX::FnDef fn_def{ EX::FnFlags::FN_MUST_INLINE, param, this->m_arena };
      fn_def.m_body.push(body_expr);

      i += 1;
      if (this->match_token_type(
            i, LX::Type::Group, LX::Type::Int, LX::Type::Fn))
      {
        UT_TODO("This branch should be explored");

        LX::Tokens next_token = { this->m_arena };
        next_token.push(this->m_tokens[i]);

        EX::Parser param_parser{ *this, next_token };
        param_parser.run();
        EX::Exprs param_expr = param_parser.m_exprs;

        EX::Expr fn_app{ EX::Type::FnApp, this->m_arena };
        fn_app.as.m_fnapp.m_param.push(*param_expr.last());
        fn_app.as.m_fnapp.m_body = fn_def;

        this->m_exprs.push(fn_app);
      }
      else
      {
        EX::Expr fn_def{ EX::Type::FnDef, this->m_arena };
        fn_def.as.m_fn.m_flags = FnFlags::FN_MUST_INLINE;
        fn_def.as.m_fn.m_param = param;
        fn_def.as.m_fn.m_body.push(body_expr);

        this->m_exprs.push(fn_def);
      }

      i += 1;
    }
    break;
    case LX::Type::Min:
    case LX::Type::Max:
    default:
    {
      std::printf("%s\n", std::to_string(t.m_type).c_str());
      // TODO: instead of UT_FAIL_IF, implement error reporting macros
      UT_FAIL_IF("The case default should be unreachable");
    }
    break;
    }
  }

  return e;
}
} // namespace EX
