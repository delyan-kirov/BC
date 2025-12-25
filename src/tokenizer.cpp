#include "tokenizer.hpp"
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

using std::string;
using std::vector;

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
