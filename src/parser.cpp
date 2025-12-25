#include "parser.hpp"
#include "tokenizer.hpp"

void
parse (std::vector<Token> &tokens, // in
       size_t begin,               // in
       size_t &end,                // out
       size_t stop,                // in
       Expr *expr                  // out
)
{
  std::vector<TokenGroup> token_groups{};
  if (!gen_token_groups (tokens, begin, stop, token_groups))
  {
    throw std::exception{};
  }

  std::cout << "(" << begin << "..." << stop << ") | "
            << std::to_string (token_groups) << std::endl;

  for (size_t idx = 0; idx < token_groups.size (); ++idx)
  {
    TokenGroup group = token_groups[idx];
    switch (group.m_type)
    {
    case Token_t::ParR:
    case Token_t::Unknown:
    {
      std::cerr << "ERROR: invalid token: " << std::to_string (group.m_type)
                << std::endl;
      return;
    }
    case Token_t::Int:
    {
      expr->type = Expr_t::Integer;
      expr->integer = tokens[group.m_begin].m_int;
    }
    break; // Int
    case Token_t::ParL:
    {
      end = group.m_begin + 1;
      parse (tokens, end, end, group.m_end, expr);
      end = group.m_end;
    }
    break; // Group
    case Token_t::Plus:
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
        Expr *left = new Expr;
        *left = *expr;

        Expr *right = new Expr;
        size_t right_begin = group.m_begin + 1;
        size_t right_end = right_begin;
        parse (tokens, right_begin, right_end, token_groups[idx + 1].m_end + 1,
               right);

        expr->type = Expr_t::Plus;
        expr->left = left;
        expr->right = right;

        end = right_end;
        ++idx;
        std::cout << "INFO(plus): " << end << " " << std::to_string (left)
                  << "+" << std::to_string (right) << "\n";
      }
    }
    break; // Plus
    case Token_t::Minus:
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
        Expr *left = new Expr;
        *left = *expr;

        Expr *right = new Expr;
        size_t right_begin = group.m_begin + 1;
        size_t right_end = right_begin;
        parse (tokens, right_begin, right_end, token_groups[idx + 1].m_end + 1,
               right);

        expr->type = Expr_t::Minus;
        expr->left = left;
        expr->right = right;

        end = right_end;
        ++idx;
        std::cout << "INFO(plus): " << end << " " << std::to_string (left)
                  << "+" << std::to_string (right) << "\n";
      }
    }
    break; // Minus
    }
    std::cout << std::to_string (expr) << "\n";
  }

  return;
}
