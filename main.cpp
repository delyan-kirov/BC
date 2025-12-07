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
                  std::vector<TokenGroup> &token_groups // out
)
//! return: true if successful, false otherwise
//! note: this will parse token groups at a single layer
{
  size_t i = begin;
  for (; i < tokens.size (); ++i)
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
      return true;
    }
    case Token_t::ParL:
    {
      size_t par_stack = 1;

      size_t subgroup_idx = i;
      for (; subgroup_idx < tokens.size (); ++subgroup_idx)
      {
        Token t = tokens[subgroup_idx];
        if (t.m_type == Token_t::ParL)
        {
          par_stack += 1;
        }
        else if (t.m_type == Token_t::ParR)
        {
          par_stack -= 1;
        }
        else
        {
          ; // do nothing
        }

        if (0 == par_stack)
        {
          break;
        }
      }

      if (subgroup_idx > i + 1)
      {
        token_groups.push_back (TokenGroup (t.m_type, i, subgroup_idx));
      }
      else
      {
        return false;
      }

      i = subgroup_idx;
      continue;
    }
    break; // ParL
    }
  }

  return true;
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
  union
  {
    Expr *left;
    Expr *right;
    long long int integer;
  } data;
};

namespace std
{
string
to_sting (Expr_t expr_type)
{
  string s{ "" };
  // TODO: implement
  return s;
}

string
to_sting (Expr expr)
{
  string s{ "" };
  // TODO: implement
  return s;
}
}

bool
is_operator (const std::vector<TokenGroup> &group, size_t idx, TokenGroup &out)
{
  if (group.size () > idx)
  {
    Token_t group_type = group[idx].m_type;
    if (group_type == Token_t::Minus || group_type == Token_t::Plus)
    {
      out = group[idx];
      return true;
    }
  }
  return false;
}

Expr_t
operation_t_from_token_t (Token_t token_type)
{
  Expr_t expr_type = Expr_t::Unknown;
  switch (token_type)
  {
  case Token_t::Plus : expr_type = Expr_t::Plus; break;
  case Token_t::Minus: expr_type = Expr_t::Minus; break;
  default:
  {
    std::cout << "ERROR: invalid token_type: " << std::to_string (token_type)
              << std::endl;
    throw std::exception{};
  }
  }
  return expr_type;
}

void
parse (std::vector<Token> &tokens // in
       ,
       size_t begin // in
       ,
       size_t &end // in
       ,
       Expr *expr // out
)
{
  std::vector<TokenGroup> token_groups{};
  if (!gen_token_groups (tokens, begin, token_groups))
  {
    throw std::exception{};
  }

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
      expr->data.integer = tokens[group.m_begin].m_int;
      end = idx;
      return;
    }
    break; // Int
    case Token_t::ParL:
    {
      size_t inner_end = 0;
      for (size_t i = group.m_begin + 1; i < group.m_end; ++i)
      {
        parse (tokens, i + 1, inner_end, expr);
      }
      end = inner_end;
      exit (0);
      return;
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
        left->type = expr->type;
        left->data = expr->data;

        Expr *right = new Expr;
        size_t end = 0;
        parse (tokens, idx + 1, end, right);

        expr->type = Expr_t::Plus;
        expr->data.left = left;
        expr->data.right = right;
      }
      return;
    }
    break; // Plus
    case Token_t::Minus:
    {
      Expr *left = new Expr;
      if (!expr)
      {
        left->type = Expr_t::Integer;
        left->data.integer = 0;
      }
      else
      {
        left->type = expr->type;
        left->data = expr->data;
      }

      Expr *right = new Expr;
      size_t end = 0;
      parse (tokens, idx + 1, end, right);

      expr->type = Expr_t::Plus;
      expr->data.left = left;
      expr->data.right = right;
      return;
    }
    break; // Minus
    }
  }

  return;
}

int
main ()
{
  string input = "(2 - 2 + 4 + 12) + (1 + 21) + 33";
  std::cout << input << std::endl;
  vector<Token> tokens = tokenize (input);

  for (auto token : tokens)
  {
    std::cout << std::to_string (token) << '\n';
  }

  Expr *expr = new Expr;
  size_t end = 0;

  try
  {
    parse (tokens, 0, end, expr);
  }
  catch (std::exception &e)
  {
    std::cerr << "ERROR: parser failed!\n";
  }
  catch (...)
  {
    std::cerr << "ERROR: unknown exception occured\n";
  }
}
