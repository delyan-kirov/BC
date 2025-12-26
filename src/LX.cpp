#include "LX.hpp"
#include <iostream>
#include <string>
#include <vector>

namespace LX
{

T::T () : m_type{ Type::Unknown }, m_int{ 0 }, m_line{ 0 }, m_offset{ 0 } {};

T::T (Type type, int integer, size_t line, size_t offset)
    : m_type{ type }, m_int{ integer }, m_line{ line }, m_offset (offset) {};

Group::Group () : m_type (Type::Unknown), m_begin (0), m_end (0) {};

Group::Group (Type type, size_t begin, size_t end)
    : m_type{ type }, m_begin{ begin }, m_end{ end }
{
  if (Type::Unknown == type || Type::ParR == type)
  {
    std::cerr << "ERROR: sanity check failed. Expected the group to be "
                 "ParL, Plus, Minus or Int, but found: "
              << std::to_string (type) << std::endl;
    throw std::exception{};
  }
}

namespace
{

void
to_integer (std::string &str, Tokens &tokens, T token)
{
  if ("" == str) { return; }
  try
  {
    int integer = std::stoi (str);
    token.m_type = Type::Int;
    token.m_int = integer;
    tokens.push_back (token);
    str.clear ();
  }
  catch (std::exception &e)
  {
    std::string error_msg = "could not parse number `" + str + "`\n";
    std::cerr << "ERROR: " << __func__ << ": " << error_msg << std::endl;
    throw e;
  }
}
} // namespace anonymous

bool
group (const std::vector<T> &tokens,    // in
       size_t begin,                    // in
       size_t end,                      // in
       std::vector<Group> &token_groups // out
)
//! return: true if successful, false otherwise
//! note: this will parse token groups at a single layer
{
  size_t i = begin;
  for (; i < end; ++i)
  {
    T t = tokens[i];
    switch (t.m_type)
    {
    case Type::Int:
    case Type::Minus:
    case Type::Plus:
    {
      token_groups.push_back (Group (t.m_type, i, i));
    }
    break;
    case Type::Unknown:
    {
      return false;
    }
    break;
    case Type::ParR:
    {
      return false;
    }
    case Type::ParL:
    {
      size_t par_stack = 1;
      size_t subgroup_idx = i + 1;

      for (; subgroup_idx < end; ++subgroup_idx)
      {
        if (tokens[subgroup_idx].m_type == Type::ParL) { ++par_stack; }
        else if (tokens[subgroup_idx].m_type == Type::ParR) { --par_stack; }

        if (par_stack == 0) { break; }
      }

      if (par_stack != 0)
      {
        return false; // unbalanced
      }

      token_groups.push_back (Group (Type::ParL, i, subgroup_idx));

      i = subgroup_idx; // skip whole group
    }
    break; // ParL
    }
  }

  return true;
}

Tokens
run (std::string str)
{
  Tokens tokens;
  std::string buffer{ "" };
  size_t lines{ 0 };
  size_t offset{ 0 };

  for (char c : str)
  {
    offset += 1;
    T token{ Type::Unknown, 0, lines, offset };
    switch (c)
    {
    case '-':
    {
      to_integer (buffer, tokens, token);
      token.m_type = Type::Minus;
      tokens.push_back (token);
    }
    break;
    case '+':
    {
      to_integer (buffer, tokens, token);
      token.m_type = Type::Plus;
      tokens.push_back (token);
    }
    break;
    case '(':
    {
      to_integer (buffer, tokens, token);
      token.m_type = Type::ParL;
      tokens.push_back (token);
    }
    break;
    case ')':
    {
      to_integer (buffer, tokens, token);
      token.m_type = Type::ParR;
      tokens.push_back (token);
    }
    break;
    case ' ':
    {
      to_integer (buffer, tokens, token);
    }
    break;
    case '\n':
    {
      to_integer (buffer, tokens, token);
      lines += 1;
    }
    break;
    default: buffer += c; break;
    }
  }

  T token{ Type::Unknown, 0, lines, offset };
  to_integer (buffer, tokens, token);

  return tokens;
}

} // namespace LX
