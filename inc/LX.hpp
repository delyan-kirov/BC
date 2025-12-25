#ifndef TOKENIZER_HEADER
#define TOKENIZER_HEADER

#include <iostream>
#include <string>
#include <vector>

namespace LX
{
enum class Type
{
  Int,
  Plus,
  Minus,
  ParL,
  ParR,
  Unknown,
};

struct T
{
  Type m_type;
  int m_int;
  size_t m_line;
  size_t m_offset;

  T ();
  T (Type type, int integer, size_t line, size_t offset);
};

struct Group
{
  Type m_type;
  size_t m_begin;
  size_t m_end;

  Group ();
  Group (Type type, size_t begin, size_t end);
};

} // LX

namespace std
{
inline string
to_string (LX::Type t)
{
  return (LX::Type::Int == t)       ? "Tokeninzer::Type::Int"
         : (LX::Type::Minus == t)   ? "LX::Type::Minus"
         : (LX::Type::Plus == t)    ? "LX::Type::Plus"
         : (LX::Type::ParL == t)    ? "LX::Type::ParL"
         : (LX::Type::ParR == t)    ? "LX::Type::ParR"
         : (LX::Type::Unknown == t) ? "LX::Type::Unknown"
                                           : "LX::Type::Insane";
}

inline string
to_string (LX::T t)
{
  string s = "line: " + to_string (t.m_line) + " offset: "
             + to_string (t.m_offset) + " type: " + to_string (t.m_type);

  if (LX::Type::Int == t.m_type)
  {
    s += "(" + to_string (t.m_int) + ")";
  }

  return s;
}

inline std::string
to_string (const std::vector<LX::Group> &token_groups)
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

} // namespace std

namespace LX
{
inline T::T ()
    : m_type{ Type::Unknown }, m_int{ 0 }, m_line{ 0 }, m_offset{ 0 } {};

inline T::T (Type type, int integer, size_t line, size_t offset)
    : m_type{ type }, m_int{ integer }, m_line{ line }, m_offset (offset) {};

inline Group::Group () : m_type (Type::Unknown), m_begin (0), m_end (0) {};

inline Group::Group (Type type, size_t begin, size_t end)
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

bool group (const std::vector<T> &tokens,    // in
            size_t begin,                    // in
            size_t end,                      // in
            std::vector<Group> &token_groups // out
);

std::vector<LX::T> run (std::string str);
} // namespace LX

#endif // TOKENIZER_HEADER
