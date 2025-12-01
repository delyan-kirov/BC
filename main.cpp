#include <iostream>
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
}
