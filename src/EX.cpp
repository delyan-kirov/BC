#include "EX.hpp"
#include "AR.hpp"
#include "LX.hpp"
#include <exception>

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
  size_t right_end = 0;

  {
    size_t next_idx = idx + 1;
    for (; next_idx < groups.size (); ++next_idx)
    {
      auto next_group = groups[next_idx];
      if (LX::Type::Minus != next_group.m_type) { break; }
    }
    right_end = groups[next_idx].m_end + 1;
  }

  result = parse (tokens, arena, right_begin, right_end, right);

  expr->m_type = op_type;
  expr->m_left = left;
  expr->m_right = right;

  if (EX::Type::Minus == op_type) { expr->m_left = right; }
  else { result = right_end; }

  return result;
}

size_t
next_group_idx (size_t idx, size_t result, LX::Groups &groups)
{
  size_t new_idx = idx + 1;
  for (; new_idx < groups.size (); ++new_idx)
  {
    auto next_group = groups[new_idx];
    if (next_group.m_end == result - 1) { break; }
  }
  return new_idx;
}

} // EX UTILITIES

size_t
parse (const Tokens &tokens, // in
       AR::T &arena,         // in
       size_t begin,         // in
       size_t end,           // in
       EX::T *expr           // out
)
{
  size_t result = begin;

  LX::Groups groups{};
  if (!LX::group (tokens, begin, end, groups))
  {
    std::cerr << "ERROR: creation of groups failed." << std::endl;
    throw std::exception{};
  }

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
    case LX::Type::Mult:
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
        result = parse_op (tokens, groups, arena, idx, EX::Type::Mult, expr);
        idx = next_group_idx (idx, result, groups);
      }
    }
    break; // Mult
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
        result = parse_op (tokens, groups, arena, idx, EX::Type::Add, expr);
        idx = next_group_idx (idx, result, groups);
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
        if (0 == idx
            || (                                  //
                EX::Type::Add != expr->m_type     //
                && EX::Type::Sub != expr->m_type  //
                && EX::Type::Int != expr->m_type  //
                && EX::Type::Mult != expr->m_type //
                )
            || EX::Type::Unknown == expr->m_type)
        {
          result
              = parse_op (tokens, groups, arena, idx, EX::Type::Minus, expr);
        }
        else
        {
          result = parse_op (tokens, groups, arena, idx, EX::Type::Sub, expr);
        }
        idx = next_group_idx (idx, result, groups);
      }
    }
    break; // Minus
    }
  }

  return result;
}
} // namespace EX
