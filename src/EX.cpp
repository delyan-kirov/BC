#include "EX.hpp"
#include "AR.hpp"
#include "LX.hpp"
#include <vector>

namespace EX
{
size_t parse (const Tokens &tokens, // in
              AR::T &arena,         // in
              size_t begin,         // in
              size_t end,           // in
              EX::T *expr           // out
);

namespace // EX UTILITIES
{

size_t
parse_op (const Tokens &tokens,     // in
          const LX::Groups &groups, // in
          AR::T &arena,             // in
          size_t idx,               // in
          EX::Type op_type,         // in
          EX::T *expr               // out
)
{
  size_t result = 0;

  auto left = (EX::T *)arena.alloc<EX::T> ();
  *left = *expr;
  LX::Group group = groups[idx];

  auto right = (EX::T *)arena.alloc<EX::T> ();
  size_t right_begin = group.m_begin + 1;
  size_t right_end = right_begin;

  result
      = parse (tokens, arena, right_begin, groups[idx + 1].m_end + 1, right);

  expr->m_type = op_type;
  expr->m_left = left;
  expr->m_right = right;

  result = right_end;

  return result;
}
} // EX UTILITIES

size_t
parse (const std::vector<LX::T> &tokens, // in
       AR::T &arena,                     // in
       size_t begin,                     // in
       size_t end,                       // in
       EX::T *expr                       // out
)
{
  size_t result = begin;

  LX::Groups groups{};
  if (!LX::group (tokens, begin, end, groups)) { throw std::exception{}; }

  for (size_t idx = 0; idx < groups.size (); ++idx)
  {
    LX::Group group = groups[idx];
    switch (group.m_type)
    {
    case LX::Type::ParR:
    case LX::Type::Unknown:
    {
      std::cerr << "ERROR: invalid token: " << std::to_string (group.m_type)
                << std::endl;
      return PARSER_FAILED;
    }
    case LX::Type::Int:
    {
      expr->m_type = EX::Type::Int;
      expr->m_int = tokens[group.m_begin].m_int;
      result += 1;
    }
    break; // Int
    case LX::Type::ParL:
    {
      result = group.m_begin + 1;
      result = parse (tokens, arena, result, group.m_end, expr);
      result = group.m_end;
    }
    break; // Group
    case LX::Type::Plus:
    {
      // We have the left, we need to check if it's valid
      if (!expr)
      {
        std::cerr << "ERROR: expected the left expression to be valid, but "
                     "nullptr found"
                  << std::endl;
        return PARSER_FAILED;
      }
      else
      {
        result = parse_op (tokens, groups, arena, idx, EX::Type::Plus, expr);
        ++idx;
      }
    }
    break; // Plus
    case LX::Type::Minus:
    {
      // We have the left, we need to check if it's valid
      if (!expr)
      {
        std::cerr << "ERROR: expected the left expression to be valid, but "
                     "nullptr found"
                  << std::endl;
        return PARSER_FAILED;
      }
      else
      {
        result = parse_op (tokens, groups, arena, idx, EX::Type::Minus, expr);
        ++idx;
      }
    }
    break; // Minus
    }
  }

  return result;
}
} // namespace EX
