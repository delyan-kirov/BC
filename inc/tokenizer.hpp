#ifndef TOKENIZER_HEADER
#define TOKENIZER_HEADER

#include <iostream>
#include <string>
#include <vector>

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
string inline to_string (Token_t t)
{
  return (Token_t::Int == t)       ? "Token_t::Int"
         : (Token_t::Minus == t)   ? "Token_t::Minus"
         : (Token_t::Plus == t)    ? "Token_t::Plus"
         : (Token_t::ParL == t)    ? "Token_t::ParL"
         : (Token_t::ParR == t)    ? "Token_t::ParR"
         : (Token_t::Unknown == t) ? "Token_t::Unknown"
                                   : "Token_t::Insane";
}

inline string
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

namespace std
{

inline std::string
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

bool gen_token_groups (const std::vector<Token> &tokens,     // in
                       size_t begin,                         // in
                       size_t end,                           // in
                       std::vector<TokenGroup> &token_groups // out
);

std::vector<Token> tokenize (std::string str);

#endif // TOKENIZER_HEADER
