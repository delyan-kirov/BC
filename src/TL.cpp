#include "TL.hpp"
#include "EX.hpp"
#include "UT.hpp"
#include <map>
namespace TL
{

static std::map<std::string, ssize_t> var_map;

ssize_t
eval(
  EX::Expr expr)
{
  int result = 0;

  switch (expr.m_type)
  {
  case EX::Type::Add:
  {
    return result = eval(*expr.as.exprs.begin()) + eval(*expr.as.exprs.last());
  }
  case EX::Type::Minus:
  {
    return result = -1 * eval(*expr.as.exprs.last());
  }
  break;
  case EX::Type::Sub:
  {
    return result = eval(*expr.as.exprs.begin()) - eval(*expr.as.exprs.last());
  }
  case EX::Type::Mult:
  {
    return result = eval(*expr.as.exprs.begin()) * eval(*expr.as.exprs.last());
  }
  break;
  case EX::Type::Div:
  {
    return result = eval(*expr.as.exprs.begin()) / eval(*expr.as.exprs.last());
  }
  break;
  case EX::Type::Modulus:
  {
    return result = eval(*expr.as.exprs.begin()) % eval(*expr.as.exprs.last());
  }
  break;
  case EX::Type::Int:
  {
    return result = expr.as.m_int;
  }
  break;
  case EX::Type::Var:
  {
    auto var_name = std::to_string(expr.as.m_var);
    if (var_map.end() != var_map.find(var_name))
    {
      return var_map[var_name];
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
  break;
  case EX::Type::If:
  {
    return eval(*expr.as.m_if.m_condition.last())
             ? eval(*expr.as.m_if.m_true_branch.begin())
             : eval(*expr.as.m_if.m_else_branch.begin());
  }
  break;
  case EX::Type::Unknown:
  default:
  {
    UT_FAIL_MSG("Type <%s> not supported yet\n", UT_TCS(expr.m_type));
  }
  break;
  }

  return result;
}
} // namespace TL
