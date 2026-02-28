#include "EX.hpp"
#include "LX.hpp"
#include "UT.hpp"

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
    if (this->match_token_type(
          idx + 2, LX::Type::Mult, LX::Type::Modulus, LX::Type::Div))
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

  EX::Expr right               = *new_parser.m_exprs.last();
  *root_expr.as.m_pair.begin() = left;
  *root_expr.as.m_pair.last()  = right;

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
      if (this->m_exprs.is_empty()) goto CASE_INT_SINGLE_EXPR;

      if (EX::Type::VarApp == this->m_exprs.last()->m_type)
      {
        // (\x = \y = ...) expr expr
        this->m_exprs.last()->as.m_varapp.m_param.push(expr);
      }
      else if (EX::Type::FnDef == this->m_exprs.last()->m_type)
      {
        // (\x = \y = ...) expr expr
        EX::Expr fnapp{ EX::Type::FnApp, m_arena };
        fnapp.as.m_fnapp.m_param.push(expr);
        fnapp.as.m_fnapp.m_body.m_body  = this->m_exprs.last()->as.m_fn.m_body;
        fnapp.as.m_fnapp.m_body.m_param = this->m_exprs.last()->as.m_fn.m_param;
        fnapp.as.m_fnapp.m_body.m_flags = this->m_exprs.last()->as.m_fn.m_flags;

        (void)m_exprs.pop();
        m_exprs.push(fnapp);
      }
      else if (EX::Type::FnApp == this->m_exprs.last()->m_type)
      {
        m_exprs.last()->as.m_fnapp.m_param.push(expr);
      }
      else
      {
      CASE_INT_SINGLE_EXPR:
        this->m_exprs.push(expr);
      }

      i += 1;
    }
    break;
    case LX::Type::Group:
    {
      Parser group_parser{ *this, t.as.m_tokens };
      group_parser.run();
      EX::Expr expr = *group_parser.m_exprs.last();
      if (this->m_exprs.is_empty()) goto CASE_GROUP_SINGLE_PARAM;

      if (EX::Type::VarApp == this->m_exprs.last()->m_type)
      {
        // (\x = \y = ...) expr expr
        this->m_exprs.last()->as.m_varapp.m_param.push(expr);
      }
      else if (EX::Type::FnDef == this->m_exprs.last()->m_type)
      {
        // (\x = \y = ...) expr expr
        EX::Expr fnapp{ EX::Type::FnApp, m_arena };
        fnapp.as.m_fnapp.m_param.push(expr);
        fnapp.as.m_fnapp.m_body.m_body  = this->m_exprs.last();
        fnapp.as.m_fnapp.m_body.m_param = this->m_exprs.last()->as.m_fn.m_param;
        fnapp.as.m_fnapp.m_body.m_flags = this->m_exprs.last()->as.m_fn.m_flags;

        (void)m_exprs.pop();
      }
      else if (EX::Type::FnApp == this->m_exprs.last()->m_type)
      {
        m_exprs.last()->as.m_fnapp.m_param.push(expr);
      }
      else
      {
      CASE_GROUP_SINGLE_PARAM:
        this->m_exprs.push(expr);
      }

      i += 1;
    }
    break;
    case LX::Type::Word:
      // TODO: candidate for refactor, label abuse unnecessary
      {
        EX::Expr var{ EX::Type::Var };
        var.as.m_var = t.as.m_string;
        i += 1;

        if (this->m_exprs.is_empty()) goto CASE_WORD_NOT_APPLIED;

        if (EX::Type::VarApp == this->m_exprs.last()->m_type)
        {
          // (\x = \y = ...) expr expr
          this->m_exprs.last()->as.m_varapp.m_param.push(var);
        }
        else if (EX::Type::FnDef == this->m_exprs.last()->m_type)
        {
          // (\x = \y = ...) expr expr
          EX::Expr fnapp{ EX::Type::FnApp, m_arena };
          fnapp.as.m_fnapp.m_param.push(var);
          fnapp.as.m_fnapp.m_body.m_body = this->m_exprs.last();
          fnapp.as.m_fnapp.m_body.m_param
            = this->m_exprs.last()->as.m_fn.m_param;
          fnapp.as.m_fnapp.m_body.m_flags
            = this->m_exprs.last()->as.m_fn.m_flags;

          (void)m_exprs.pop();
        }
        else if (EX::Type::FnApp == this->m_exprs.last()->m_type)
        {
          m_exprs.last()->as.m_fnapp.m_param.push(var);
        }
        goto CASE_WORD_END;

      CASE_WORD_NOT_APPLIED:
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
          this->m_exprs.push(var);
        }
      }
    CASE_WORD_END:
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
        *expr.as.m_expr = *new_parser.m_exprs.last();

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
      // FIXME: https://github.com/delyan-kirov/BC/issues/25
      // let var = body_expr in app_expr
      UT::String var_name = t.as.m_let_tokens.m_var_name;

      EX::Parser value_parser{ *this, t.as.m_let_tokens.m_let_tokens };
      value_parser.run();
      EX::Expr *value_expr = value_parser.m_exprs.last();

      EX::Parser continuation_parser{ *this, t.as.m_let_tokens.m_in_tokens };
      continuation_parser.run();
      EX::Expr *continuation_expr = continuation_parser.m_exprs.last();

      EX::Expr let_expr{ EX::Type::Let };
      let_expr.as.m_let.m_continuation = continuation_expr;
      let_expr.as.m_let.m_var_name     = var_name;
      let_expr.as.m_let.m_value        = value_expr;

      this->m_exprs.push(let_expr);

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
      *fn_def.m_body = body_expr;

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
        *fn_def.as.m_fn.m_body = body_expr;

        this->m_exprs.push(fn_def);
      }

      i += 1;
    }
    break;
    case LX::Type::If:
    {
      EX::Parser condition_parser{ *this, t.as.m_if_tokens.m_condition };
      condition_parser.run();
      EX::Expr condition = *condition_parser.m_exprs.last();

      EX::Parser true_branch_parser{ *this, t.as.m_if_tokens.m_true_branch };
      true_branch_parser.run();
      EX::Expr true_branch_expr = *true_branch_parser.m_exprs.last();

      EX::Parser else_branch_parser{ *this, t.as.m_if_tokens.m_else_branch };
      else_branch_parser.run();
      EX::Expr else_branch_expr = *else_branch_parser.m_exprs.last();

      EX::Expr if_expr{ EX::Type::If, this->m_arena };
      *if_expr.as.m_if.m_condition   = condition;
      *if_expr.as.m_if.m_else_branch = else_branch_expr;
      *if_expr.as.m_if.m_true_branch = true_branch_expr;

      this->m_exprs.push(if_expr);
      i += 1;
    }
    break;
    case LX::Type::IsEq:
    {
      this->parse_max_precedence_arithmetic_op(EX::Type::IsEq, i);
    }
    break;
    case LX::Type::Not:
    {
      i += 1;

      LX::Tokens next_token = { this->m_arena };
      next_token.push(this->m_tokens[i]);

      EX::Parser not_parser{ *this, next_token };
      not_parser.run();

      EX::Expr not_expr{ EX::Type::Not };
      not_expr.as.m_expr = not_parser.m_exprs.last();

      m_exprs.push(not_expr);

      i += 1;
    }
    break;
    case LX::Type::Str:
    {
      i += 1;

      EX::Expr string_expr{ Type::Str };
      string_expr.as.m_string = t.as.m_string;

      m_exprs.push(string_expr);
    }
    break;
    case LX::Type::While:
    {
      i += 1;

      EX::Parser condition_parser{ *this, t.as.m_while.m_condition };
      condition_parser.run();

      // FIXME: variable str should result in function app but currently, the
      // variable is ignored
      EX::Parser body_parser{ *this, t.as.m_while.m_body };
      body_parser.run();

      EX::Expr while_expr{ EX::Type::While };
      while_expr.as.m_while.m_body      = body_parser.m_exprs.last();
      while_expr.as.m_while.m_condition = condition_parser.m_exprs.last();

      m_exprs.push(while_expr);
    }
    break;
    case LX::Type::Min:
    case LX::Type::Max:
    default:
    {
      // TODO: instead of UT_FAIL_IF, implement error reporting macros
      UT_FAIL_MSG("The token type <%s> is unhandled", UT_TCS(t.m_type));
    }
    break;
    }
  }

  return e;
}
} // namespace EX
