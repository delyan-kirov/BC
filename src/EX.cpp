#include "EX.hpp"
#include "LX.hpp"

namespace EX
{

namespace // EX UTILITIES
{
}

void
parse (std::vector<LX::T> &tokens, // in
       size_t begin,                      // in
       size_t &end,                       // out
       size_t stop,                       // in
       EX::T *expr                      // out
)
{
  std::vector<LX::Group> token_groups{};
  if (!LX::group (tokens, begin, stop, token_groups))
  {
    throw std::exception{};
  }

  // std::cout << "(" << begin << "..." << stop << ") | "
  //           << std::to_string (token_groups) << std::endl;

  for (size_t idx = 0; idx < token_groups.size (); ++idx)
  {
    LX::Group group = token_groups[idx];
    switch (group.m_type)
    {
    case LX::Type::ParR:
    case LX::Type::Unknown:
    {
      std::cerr << "ERROR: invalid token: " << std::to_string (group.m_type)
                << std::endl;
      return;
    }
    case LX::Type::Int:
    {
      expr->type = EX::Type::Integer;
      expr->integer = tokens[group.m_begin].m_int;
    }
    break; // Int
    case LX::Type::ParL:
    {
      end = group.m_begin + 1;
      parse (tokens, end, end, group.m_end, expr);
      end = group.m_end;
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
        return;
      }
      else
      {
        EX::T *left = new EX::T;
        *left = *expr;

        EX::T *right = new EX::T;
        size_t right_begin = group.m_begin + 1;
        size_t right_end = right_begin;
        parse (tokens, right_begin, right_end, token_groups[idx + 1].m_end + 1,
               right);

        expr->type = Type::Plus;
        expr->left = left;
        expr->right = right;

        end = right_end;
        ++idx;
        // std::cout << "INFO(plus): " << end << " " << std::to_string (left)
        //           << "+" << std::to_string (right) << "\n";
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
        return;
      }
      else
      {
        EX::T *left = new EX::T;
        *left = *expr;

        EX::T *right = new EX::T;
        size_t right_begin = group.m_begin + 1;
        size_t right_end = right_begin;
        parse (tokens, right_begin, right_end, token_groups[idx + 1].m_end + 1,
               right);

        expr->type = EX::Type::Minus;
        expr->left = left;
        expr->right = right;

        end = right_end;
        ++idx;
        // std::cout << "INFO(plus): " << end << " " << std::to_string (left)
        //           << "+" << std::to_string (right) << "\n";
      }
    }
    break; // Minus
    }
    // std::cout << std::to_string (expr) << "\n";
  }

  return;
}
} // namespace EX
