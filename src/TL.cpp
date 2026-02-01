#include "TL.hpp"
#include "EX.hpp"
#include "LX.hpp"
#include "UT.hpp"
#include <cstdio>
#include <map>

namespace TL
{
// TODO: Don't use these global maps, that's stupid
using ExprMap = std::map<std::string, EX::Expr>;
static ExprMap var_map;

Mod::Mod(
  UT::String file_name)
{
  AR::Arena  arena{};
  UT::String source_code = UT::map_entire_file(file_name, arena);

  LX::Lexer l{ source_code.m_mem, arena, 0, source_code.m_len };
  l.run();

  for (LX::Token t : l.m_tokens)
  {
    TL::Type def_type = TL::Type::ExtDef;
    switch (t.m_type)
    {
    case LX::Type::ExtDef: def_type = TL::Type::ExtDef; break;
    case LX::Type::IntDef: def_type = TL::Type::IntDef; break;
    default              : UT_FAIL_MSG("UNREACHABLE token type: %s", UT_TCS(t.m_type));
    }

    UT::String def_name   = t.as.m_sym.m_sym_name;
    LX::Tokens def_tokens = t.as.m_sym.m_def;

    EX::Parser parser{ def_tokens, arena, source_code.m_mem };
    parser.run();

    TL::Def def{ def_type, def_name, *parser.m_exprs.last() };

    std::printf(
      "%s %s = %s\n", UT_TCS(def.m_type), UT_TCS(def_name), UT_TCS(def.m_expr));

    var_map[std::to_string(def_name)] = eval(def.m_expr);

    std::printf("INFO: %s = %s\n",
                UT_TCS(def_name),
                UT_TCS(var_map[std::to_string(def_name)]));
  }
}

static EX::Expr
eval_bi_op(
  EX::Expr expr)
{
  ssize_t  left  = eval(expr.as.exprs[0]).as.m_int;
  ssize_t  right = eval(expr.as.exprs[1]).as.m_int;
  EX::Expr result_expr{ EX::Type::Int };

  switch (expr.m_type)
  {
  case EX::Type::Add    : result_expr.as.m_int = left + right; break;
  case EX::Type::Sub    : result_expr.as.m_int = left - right; break;
  case EX::Type::Mult   : result_expr.as.m_int = left * right; break;
  case EX::Type::Div    : result_expr.as.m_int = left / right; break;
  case EX::Type::Modulus: result_expr.as.m_int = left % right; break;
  case EX::Type::IsEq   : result_expr.as.m_int = left == right; break;
  default               : UT_FAIL_MSG("UNREACHABLE expr.m_type = %s", expr.m_type);
  }
  return result_expr;
}

EX::Expr
eval(
  EX::Expr expr)
{
  // printf("Expr = %s\n", UT_TCS(expr));
  switch (expr.m_type)
  {
  case EX::Type::Add:
  case EX::Type::Minus:
  case EX::Type::Sub:
  case EX::Type::Mult:
  case EX::Type::Div:
  case EX::Type::Modulus:
  case EX::Type::IsEq   : return eval_bi_op(expr);
  case EX::Type::Int    : return expr;
  case EX::Type::Var:
  {
    auto var_name = std::to_string(expr.as.m_var);
    if (var_map.end() != var_map.find(var_name))
    {
      return var_map[var_name];
    }
    else
    {
      return expr;
    }
  }
  break;
  // TODO: This is broken, we should evaluate not like this, one way is to
  // generate dynamic instances of a function call if the function is recursive
  case EX::Type::FnApp:
  {
    EX::Expr    param      = *expr.as.m_fnapp.m_param.last();
    EX::FnDef   fn_def     = expr.as.m_fnapp.m_body;
    EX::Expr    body       = *fn_def.m_body.last();
    std::string param_name = std::to_string(fn_def.m_param);

    var_map[param_name] = eval(param);
    printf("Expr = %s\n", UT_TCS(var_map[param_name]));
    return eval(body);
  }
  case EX::Type::FnDef:
  {
    return expr;
  }
  case EX::Type::VarApp:
  {
    EX::Expr    param_expr = *expr.as.m_varapp.m_param.last();
    std::string fn_name    = std::to_string(expr.as.m_varapp.m_fn_name);
    auto        fn_def_it  = var_map.find(fn_name);

    if (var_map.end() != fn_def_it)
    {
      EX::Expr fn_def_expr = fn_def_it->second;
      EX::Expr application_expr{ EX::Type::FnApp,
                                 *expr.as.m_varapp.m_param.m_arena };

      application_expr.as.m_fnapp.m_body = fn_def_expr.as.m_fn;
      application_expr.as.m_fnapp.m_param.push(param_expr);

      return eval(application_expr);
    }

    return expr;
  }
  case EX::Type::If:
  {
    return eval(*expr.as.m_if.m_condition.last()).as.m_int
             ? eval(*expr.as.m_if.m_true_branch.begin())
             : eval(*expr.as.m_if.m_else_branch.begin());
  }
  case EX::Type::Let:
  {
    UT::String var_name        = expr.as.m_let.m_var_name;
    EX::Expr  *value           = expr.as.m_let.m_value;
    EX::Expr  *continuation    = expr.as.m_let.m_continuation;
    EX::Expr   value_evaluated = eval(*value);

    var_map[std::to_string(var_name)] = value_evaluated;

    return eval(*continuation);
  }
  case EX::Type::Unknown:
  default:
  {
    UT_FAIL_MSG("Type <%s> not supported yet\n", UT_TCS(expr.m_type));
  }
  break;
  }

  UT_FAIL_MSG("Expr type not resolved, type = %s", expr.m_type);
  return EX::Expr{ EX::Type::Unknown };
}
} // namespace TL
