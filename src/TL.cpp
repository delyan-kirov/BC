#include "TL.hpp"

namespace TL
{

int
eval (EX::T *expr)
{
  int result = 0;

  while (expr)
  {
    switch (expr->m_type)
    {
    case EX::Type::Add:
    {
      return result = eval (expr->m_left) + eval (expr->m_right);
    }
    case EX::Type::Minus:
    {
      return result = -1 * eval (expr->m_left);
    }
    break;
    case EX::Type::Sub:
    {
      return result = eval (expr->m_left) - eval (expr->m_right);
    }
    case EX::Type::Mult:
    {
      return result = eval (expr->m_left) * eval (expr->m_right);
    }
    break;
    case EX::Type::Int:
    {
      return result = expr->m_int;
    }
    break;
    case EX::Type::Unknown:
    {
      throw std::exception{};
    }
    break;
    }
  }

  return result;
}
}
