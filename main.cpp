#include <cstddef>
#include <exception>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

using std::string;
using std::vector;

enum class Token_t
{
  Int,
  Plus,
  Minus,
  ParL,
  ParR,
  Unknown,
};

struct Token
{
  Token_t m_type;
  int m_int;
  size_t m_line;
  size_t m_offset;

  Token ()
      : m_type{ Token_t::Unknown }, m_int{ 0 }, m_line{ 0 }, m_offset{ 0 } {};
  Token (Token_t type, int integer, size_t line, size_t offset)
      : m_type{ type }, m_int{ integer }, m_line{ line }, m_offset (offset) {};
};

namespace std
{
string
to_string (Token_t t)
{
  return (Token_t::Int == t)       ? "Token_t::Int"
         : (Token_t::Minus == t)   ? "Token_t::Minus"
         : (Token_t::Plus == t)    ? "Token_t::Plus"
         : (Token_t::ParL == t)    ? "Token_t::ParL"
         : (Token_t::ParR == t)    ? "Token_t::ParR"
         : (Token_t::Unknown == t) ? "Token_t::Unknown"
                                   : "Token_t::Insane";
}

string
to_string (Token t)
{
  string s = "line: " + to_string (t.m_line) + " offset: "
             + to_string (t.m_offset) + " type: " + to_string (t.m_type);

  if (Token_t::Int == t.m_type)
  {
    s += "(" + to_string (t.m_int) + ")";
  }

  return s;
}
} // namespace std

void
token_error_report (string &error_msg)
{
  std::cerr << "ERROR: " << __func__ << ": " << error_msg << std::endl;
}

void
tokenize_integer (string &str, vector<Token> &tokens, Token token)
{
  if ("" == str)
  {
    return;
  }
  try
  {
    int integer = std::stoi (str);
    token.m_type = Token_t::Int;
    token.m_int = integer;
    tokens.push_back (token);
    str.clear ();
  }
  catch (std::exception &e)
  {
    string error_msg = "could not parse number `" + str + "`\n";
    token_error_report (error_msg);
    throw e;
  }
}

vector<Token>
tokenize (string str)
{
  vector<Token> tokens;
  string buffer{ "" };
  size_t lines{ 0 };
  size_t offset{ 0 };

  for (char c : str)
  {
    offset += 1;
    Token token{ Token_t::Unknown, 0, lines, offset };
    switch (c)
    {
    case '-':
    {
      tokenize_integer (buffer, tokens, token);
      token.m_type = Token_t::Minus;
      tokens.push_back (token);
    }
    break;
    case '+':
    {
      tokenize_integer (buffer, tokens, token);
      token.m_type = Token_t::Plus;
      tokens.push_back (token);
    }
    break;
    case '(':
    {
      tokenize_integer (buffer, tokens, token);
      token.m_type = Token_t::ParL;
      tokens.push_back (token);
    }
    break;
    case ')':
    {
      tokenize_integer (buffer, tokens, token);
      token.m_type = Token_t::ParR;
      tokens.push_back (token);
    }
    break;
    case ' ':
    {
      tokenize_integer (buffer, tokens, token);
    }
    break;
    case '\n':
    {
      tokenize_integer (buffer, tokens, token);
      lines += 1;
    }
    break;
    default: buffer += c; break;
    }
  }

  Token token{ Token_t::Unknown, 0, lines, offset };
  tokenize_integer (buffer, tokens, token);

  return tokens;
}

struct TokenGroup
{
  Token_t m_type;
  size_t m_begin;
  size_t m_end;

  TokenGroup (Token_t type, size_t begin, size_t end)
      : m_type{ type }, m_begin{ begin }, m_end{ end }
  {
    if (Token_t::Unknown == type || Token_t::ParR == type)
    {
      std::cerr << "ERROR: sanity check failed. Expected the group to be "
                   "ParL, Plus, Minus or Int, but found: "
                << std::to_string (type) << std::endl;
      throw std::exception{};
    }
  }

  TokenGroup () : m_type (Token_t::Unknown), m_begin (0), m_end (0) {};
};

bool
gen_token_groups (const std::vector<Token> &tokens,     // in
                  size_t begin,                         // in
                  size_t end,                           // in
                  std::vector<TokenGroup> &token_groups // out
)
//! return: true if successful, false otherwise
//! note: this will parse token groups at a single layer
{
  size_t i = begin;
  for (; i < end; ++i)
  {
    Token t = tokens[i];
    switch (t.m_type)
    {
    case Token_t::Int:
    case Token_t::Minus:
    case Token_t::Plus:
    {
      token_groups.push_back (TokenGroup (t.m_type, i, i));
    }
    break;
    case Token_t::Unknown:
    {
      return false;
    }
    break;
    case Token_t::ParR:
    {
      return false;
    }
    case Token_t::ParL:
    {
      size_t par_stack = 1;
      size_t subgroup_idx = i + 1;

      for (; subgroup_idx < end; ++subgroup_idx)
      {
        if (tokens[subgroup_idx].m_type == Token_t::ParL)
        {
          ++par_stack;
        }
        else if (tokens[subgroup_idx].m_type == Token_t::ParR)
        {
          --par_stack;
        }

        if (par_stack == 0)
        {
          break;
        }
      }

      if (par_stack != 0)
      {
        return false; // unbalanced
      }

      token_groups.push_back (TokenGroup (Token_t::ParL, i, subgroup_idx));

      i = subgroup_idx; // skip whole group
    }
    break; // ParL
    }
  }

  return true;
}

namespace std
{

std::string
to_string (const std::vector<TokenGroup> &token_groups)
{
  string s{ "" };

  for (auto token_group : token_groups)
  {
    s += std::to_string (token_group.m_type);
    s += "(";
    s += std::to_string (token_group.m_begin);
    s += "...";
    s += std::to_string (token_group.m_end);
    s += ") ";
  }

  return s;
}
}

enum class Expr_t
{
  Integer,
  Plus,
  Minus,
  Unknown
};

struct Expr
{
  Expr_t type;
  Expr *left;
  Expr *right;
  int integer;
};

namespace std
{
string
to_string (Expr_t expr_type)
{
  switch (expr_type)
  {
  case Expr_t::Integer: return "Expr_t::Integer";
  case Expr_t::Minus  : return "Expr_t::Minus";
  case Expr_t::Plus   : return "Expr_t::Plus";
  case Expr_t::Unknown:
  default             : return "Expr_t::Unknown";
  }
}

string to_string (Expr *expr);

string
to_string (Expr *expr)
{
  string s{ "" };

  if (!expr)
  {
    std::cerr << "ERROR: expression given is null";
    return "";
  }

  switch (expr->type)
  {
  case Expr_t::Integer:
  {
    s = std::to_string (expr->integer);
  }
  break;
  case Expr_t::Plus:
  {
    s += "(";
    s += to_string (expr->left);
    s += " + ";
    s += to_string (expr->right);
    s += ")";
  }
  break;
  case Expr_t::Minus:
  {
    s += "(";
    s += to_string (expr->left);
    s += " - ";
    s += to_string (expr->right);
    s += ")";
  }
  break;
  case Expr_t::Unknown:
  default:
  {
    s += "Expr_t::Unknown";
  }
  break;
  }

  return s;
}
}

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

int
main ()
{
  string input = "1 - (2 - (3 + 43)) + 4 - 1";
  // string input = "1 + 1";
  std::cout << input << std::endl;
  vector<Token> tokens = tokenize (input);

  // for (auto token : tokens)
  // {
  //   std::cout << std::to_string (token) << '\n';
  // }

  Expr *expr = new Expr;

  try
  {
    size_t end = 0;
    parse (tokens, end, end, tokens.size (), expr);
  }
  catch (std::exception &e)
  {
    std::cerr << "ERROR: parser failed!\n";
  }
  catch (...)
  {
    std::cerr << "ERROR: unknown exception occured\n";
  }

  std::cout << std::to_string (expr) << "\n";
}

/*
 * TODO:
     - Split code so that it's not as crazy as this
     - Add testing
 */
