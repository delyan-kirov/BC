#include "EX.hpp"
#include "LX.hpp"
#include <cassert>
#include <cstdlib>

namespace EX
{

EX::T
Parser::alloc_subexpr (size_t n)
{
  EX::T expr{};
  if (n) { expr.exprs = EX::Exprs{ this->m_arena, n }; }

  return expr;
};

E
Parser::parse_binop (EX::Type type, size_t start, size_t end)
{
  E result = E::OK;

  EX::T root_expr = this->alloc_subexpr (2);
  EX::T left = *this->m_exprs.last ();
  root_expr.m_type = type;

  EX::Parser new_parser{ *this, start, end };
  result = new_parser.run ();

  EX::T right = *new_parser.m_exprs.last ();
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
    LX::T t = this->m_tokens[i];

    switch (t.m_type)
    {
    case LX::Type::Int:
    {
      EX::T expr = this->alloc_subexpr (0);
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
      if (this->match_token_type (LX::Type::Int, i + 1)
          || this->match_token_type (LX::Type::Group, i + 1))
      {
        if (this->match_token_type (LX::Type::Mult, i + 2))
        {
          parse_binop (EX::Type::Add, i + 1, this->m_end);
          i += this->m_end + 1;
        }
        else
        {
          this->parse_binop (EX::Type::Add, i + 1, i + 2);
          i += 2;
        }
      }
      else { asm ("int3"); }
    }
    break;
    case LX::Type::Mult:
    {
      if (this->match_token_type (LX::Type::Int, i + 1))
      {
        if (this->match_token_type (LX::Type::Mult, i + 2))
        {
          parse_binop (EX::Type::Mult, i + 1, this->m_end);
          i += this->m_end + 1;
        }
        else
        {
          this->parse_binop (EX::Type::Mult, i + 1, i + 2);
          i += 2;
        }
      }
      else { asm ("int3"); }
    }
    break;
    case LX::Type::Minus:
    {
      if (this->m_exprs.is_empty ()) // The minus is unary
      {
        EX::Parser new_parser{ *this, i + 1, i + 2 };
        new_parser.run ();

        EX::T expr = this->alloc_subexpr (1);
        expr.m_type = EX::Type::Minus;
        expr.exprs[0] = *new_parser.m_exprs.last ();

        this->m_exprs.push (expr);
        i += 2;
      }
      else // Binary minus
      {
        if (this->match_token_type (LX::Type::Int, i + 1))
        {
          if (this->match_token_type (LX::Type::Mult, i + 2))
          {
            parse_binop (EX::Type::Add, i, this->m_end);
            i += this->m_end + 1;
          }
          else
          {
            this->parse_binop (EX::Type::Sub, i + 1, i + 2);
            i += 2;
          }
        }
        else { asm ("int3"); }
      }
    }
    break;
    default: break;
    }
  }

  return e;
}
} // namespace EX
