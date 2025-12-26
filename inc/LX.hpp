#ifndef LX_HEADER
#define LX_HEADER

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

typedef std::vector<LX::Group> Groups;

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

  if (LX::Type::Int == t.m_type) { s += "(" + to_string (t.m_int) + ")"; }

  return s;
}

inline std::string
to_string (const LX::Groups &groups)
{
  string s{ "" };

  for (auto group : groups)
  {
    s += std::to_string (group.m_type);
    s += "(";
    s += std::to_string (group.m_begin);
    s += "...";
    s += std::to_string (group.m_end);
    s += ") ";
  }

  return s;
}

} // namespace std

namespace LX
{
typedef std::vector<T> Tokens;

bool group (const Tokens &tokens, // in
            size_t begin,         // in
            size_t end,           // in
            Groups &groups        // out
);

Tokens run (std::string str);

} // namespace LX

#endif // LX_HEADER
