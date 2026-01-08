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
    case LX::Type::Mult:
    {
      EX::T root_expr = this->alloc_subexpr (2);
      EX::T left = *this->m_exprs.last ();
      root_expr.m_type = EX::Type::Mult;

      EX::Parser new_parser{ *this, i + 1, i + 2 };
      new_parser.run ();

      EX::T right = *new_parser.m_exprs.last ();
      root_expr.exprs[0] = left;
      root_expr.exprs[1] = right;

      *this->m_exprs.last () = root_expr;

      i += 2;
    }
    break;
    case LX::Type::Minus:
    {
      EX::Parser new_parser{ *this, i + 1, i + 2 };
      new_parser.run ();

      EX::T expr = this->alloc_subexpr (1);
      expr.m_type = EX::Type::Minus;
      expr.exprs[0] = *new_parser.m_exprs.last ();

      this->m_exprs.push (expr);
      i += 2;
    }
    break;
    default: break;
    }
  }

  return e;
}
} // namespace EX
