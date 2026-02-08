#include "TL.hpp"
#include "EX.hpp"
#include "LX.hpp"
#include "UT.hpp"
#include <cstdio>
#include <map>
#include <string>

namespace TL
{
Mod::Mod(
  UT::String file_name, AR::Arena &arena)
{
  UT::String source_code = UT::read_entire_file(file_name, arena);
  this->m_defs           = { arena };
  this->m_name           = file_name;

  LX::Lexer l{ source_code.m_mem, arena, 0, source_code.m_len };
  l.run();
  l.generate_event_report();

  Env global_env{};

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

    Instance instance{ *parser.m_exprs.last(), global_env };
    instance                             = eval(instance);
    global_env[std::to_string(def_name)] = instance.m_expr;
    TL::Def def{ def_type, def_name, instance.m_expr };

    this->m_defs.push(def);

    std::printf("%s %s = %s\n",
                UT_TCS(def.m_type),
                UT_TCS(def_name),
                UT_TCS(global_env[std::to_string(def_name)]));
  }

  for (auto it = global_env.begin(); it != global_env.end(); ++it)
  {
    std::printf("INFO: %s -> %s\n", it->first.c_str(), UT_TCS(it->second));
  }
}

static Instance
eval_bi_op(
  Instance &inst)
{
  Env      env  = inst.m_env;
  EX::Expr expr = inst.m_expr;

  Instance left_instance  = Instance{ expr.as.exprs[0], env };
  ssize_t  left           = eval(left_instance).m_expr.as.m_int;
  Instance right_instance = Instance{ expr.as.exprs[1], env };
  ssize_t  right          = eval(right_instance).m_expr.as.m_int;
  Instance result_instance{ EX::Type::Int, env };

  switch (expr.m_type)
  {
  case EX::Type::Add    : result_instance.m_expr.as.m_int = left + right; break;
  case EX::Type::Sub    : result_instance.m_expr.as.m_int = left - right; break;
  case EX::Type::Mult   : result_instance.m_expr.as.m_int = left * right; break;
  case EX::Type::Div    : result_instance.m_expr.as.m_int = left / right; break;
  case EX::Type::Modulus: result_instance.m_expr.as.m_int = left % right; break;
  case EX::Type::IsEq   : result_instance.m_expr.as.m_int = left == right; break;
  default               : UT_FAIL_MSG("UNREACHABLE expr.m_type = %s", expr.m_type);
  }

  return result_instance;
}

// TODO: finish impl
void
alpha_conversion(
  EX::Expr *expr, UT::String old_var, UT::String new_var)
{
  UT_TODO(Finish impl);
  switch (expr->m_type)
  {
  case EX::Type::Add:
  case EX::Type::Sub:
  case EX::Type::Div:
  case EX::Type::IsEq:
  case EX::Type::Modulus:
  case EX::Type::Mult:
  {
    EX::Exprs exprs = expr->as.exprs;
    alpha_conversion(exprs.m_mem + 0, old_var, new_var);
    alpha_conversion(exprs.m_mem + 1, old_var, new_var);
    return;
  }
  case EX::Type::Minus:
  {
    EX::Exprs exprs = expr->as.exprs;
    alpha_conversion(exprs.m_mem, old_var, new_var);
    return;
  }
  case EX::Type::Var:
  {
    if (expr->as.m_var == old_var)
    {
      expr->as.m_var = new_var;
    }
    return;
  }
  case EX::Type::FnApp:
  {
    alpha_conversion(expr->as.m_fnapp.m_param.m_mem, old_var, new_var);
    if (expr->as.m_fnapp.m_body.m_param == old_var)
    {
      expr->as.m_fnapp.m_body.m_param = new_var;
    }
    alpha_conversion(expr->as.m_fnapp.m_body.m_body.m_mem, old_var, new_var);
    return;
  }
  case EX::Type::VarApp:
  {
    if (expr->as.m_varapp.m_fn_name == old_var)
    {
      expr->as.m_varapp.m_fn_name = new_var;
    }
    alpha_conversion(expr->as.m_varapp.m_param.m_mem, old_var, new_var);

    return;
  }
  case EX::Type::Let:
  {
    if (expr->as.m_let.m_var_name == old_var)
    {
      expr->as.m_let.m_var_name = new_var;
    }
    alpha_conversion(expr->as.m_let.m_value, old_var, new_var);
    alpha_conversion(expr->as.m_let.m_continuation, old_var, new_var);

    return;
  }
  case EX::Type::FnDef:
  {
    if (expr->as.m_fn.m_param == old_var)
    {
      expr->as.m_fn.m_param = new_var;
    }
    alpha_conversion(expr->as.m_fn.m_body.m_mem, old_var, new_var);

    return;
  }
  case EX::Type::Int:
  {
    return;
  }
  case EX::Type::If:
  {
    alpha_conversion(expr->as.m_if.m_condition.m_mem, old_var, new_var);
    alpha_conversion(expr->as.m_if.m_true_branch.m_mem, old_var, new_var);
    alpha_conversion(expr->as.m_if.m_else_branch.m_mem, old_var, new_var);
    return;
  }
  case EX::Type::Unknown:
  {
    UT_FAIL_IF("UNREACHABLE");
  }
  }
}

// TODO: finish impl
EX::Expr
clone_expression(
  EX::Expr expr, AR::Arena)
{
  UT_TODO(Finish impl);
  EX::Expr new_expr;
  new_expr.m_type = expr.m_type;

  switch (expr.m_type)
  {
  case EX::Type::Add:
  case EX::Type::Sub:
  case EX::Type::Div:
  case EX::Type::IsEq:
  case EX::Type::Modulus:
  case EX::Type::Mult:
  {
    return new_expr;
  }
  case EX::Type::Minus:
  {
    return new_expr;
  }
  case EX::Type::Var:
  {
    return new_expr;
  }
  case EX::Type::FnApp:
  {
    return new_expr;
  }
  case EX::Type::VarApp:
  {
    return new_expr;
  }
  case EX::Type::Let:
  {
    return new_expr;
  }
  case EX::Type::FnDef:
  {
    return new_expr;
  }
  case EX::Type::Int:
  {
    return new_expr;
  }
  case EX::Type::If:
  {
    return new_expr;
  }
  case EX::Type::Unknown:
  {
    UT_FAIL_IF("UNREACHABLE");
  }
  }
  return new_expr;
}

Instance
eval(
  Instance &inst)
{
  EX::Expr expr = inst.m_expr;
  Env      env  = inst.m_env;

  switch (expr.m_type)
  {
  case EX::Type::Add:
  case EX::Type::Minus:
  case EX::Type::Sub:
  case EX::Type::Mult:
  case EX::Type::Div:
  case EX::Type::Modulus:
  case EX::Type::IsEq   : return eval_bi_op(inst);
  case EX::Type::Int    : return inst;
  case EX::Type::Var:
  {
    UT::String var_name = expr.as.m_var;
    auto       var_expr = env.find(std::to_string(var_name));

    if (var_expr != env.end())
    {
      Instance new_instance{ var_expr->second, env };
      return new_instance;
    }
  }
  break;
  case EX::Type::FnApp:
  {
    EX::Expr    param      = *expr.as.m_fnapp.m_param.last();
    EX::FnDef   fn_def     = expr.as.m_fnapp.m_body;
    EX::Expr    body       = *fn_def.m_body.last();
    std::string param_name = std::to_string(fn_def.m_param);
    Env         env        = inst.m_env;

    Instance param_instance{ param, env };
    env[param_name] = eval(param_instance).m_expr;

    Instance body_instance{ body, env };
    body_instance = eval(body_instance);

    return body_instance;
  }
  case EX::Type::FnDef:
  {
    return inst;
  }
  case EX::Type::VarApp:
  {
    // TODO: See if this works for fn application
    std::string fn_name   = std::to_string(expr.as.m_varapp.m_fn_name);
    auto        fn_def_it = env.find(fn_name);
    EX::Expr    fndef{};

    if (env.end() != fn_def_it)
    {
      fndef            = fn_def_it->second;
      EX::Exprs params = expr.as.m_varapp.m_param;
      for (EX::Expr &param_expr : params)
      {
        Instance param_inst{ param_expr, env };
        env[std::to_string(fndef.as.m_fn.m_param)] = eval(param_inst).m_expr;
        fndef = *fndef.as.m_fn.m_body.last();
      }

      Instance app_instance{ fndef, env };
      return eval(app_instance);
    }
    return inst;
  }
  case EX::Type::If:
  {
    Instance cond_instance{ *expr.as.m_if.m_condition.last(), env };
    Instance true_instance{ *expr.as.m_if.m_true_branch.last(), env };
    Instance else_instance{ *expr.as.m_if.m_else_branch.last(), env };

    return eval(cond_instance).m_expr.as.m_int ? eval(true_instance)
                                               : eval(else_instance);
  }
  case EX::Type::Let:
  {
    UT::String var_name = expr.as.m_let.m_var_name;
    Instance   value_instance{ *expr.as.m_let.m_value, env };
    value_instance = eval(value_instance);

    Env local_env                       = env;
    local_env[std::to_string(var_name)] = value_instance.m_expr;

    Instance continuation_instrance{ *expr.as.m_let.m_continuation, local_env };

    return eval(continuation_instrance);
  }
  case EX::Type::Unknown:
  default:
  {
    UT_FAIL_MSG("Type <%s> not supported yet\n", UT_TCS(expr.m_type));
  }
  break;
  }

  UT_FAIL_MSG("Expr type not resolved, type = %s", expr.m_type);
  return Instance{};
}
} // namespace TL
