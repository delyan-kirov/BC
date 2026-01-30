#include "TL.hpp"
#include "EX.hpp"
#include "UT.hpp"
#include <cstdio>
#include <map>
namespace TL
{

using ExprMap = std::map<std::string, EX::Expr>;
static ExprMap var_map;

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
  printf("Expr = %s\n", UT_TCS(expr));
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
  case EX::Type::FnApp:
  {
    EX::Expr    param      = *expr.as.m_fnapp.m_param.last();
    EX::FnDef   fn_def     = expr.as.m_fnapp.m_body;
    EX::Expr    body       = *fn_def.m_body.last();
    std::string param_name = std::to_string(fn_def.m_param);

    var_map[param_name] = eval(param);
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
