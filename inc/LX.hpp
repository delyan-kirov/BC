#ifndef LX_HEADER
#define LX_HEADER

#include <string>
#include <vector>

namespace LX
{
enum class Type
{
  Int,
  Plus,
  Minus,
  Mult,
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

typedef std::vector<T> Tokens;

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
  switch (t)
  {
  case LX::Type::Int    : return "Tokeninzer::Type::Int";
  case LX::Type::Minus  : return "LX::Type::Minus";
  case LX::Type::Mult   : return "LX::Type::Mult";
  case LX::Type::Plus   : return "LX::Type::Plus";
  case LX::Type::ParL   : return "LX::Type::ParL";
  case LX::Type::ParR   : return "LX::Type::ParR";
  case LX::Type::Unknown: return "LX::Type::Unknown";
  }
}

inline string
to_string (LX::T t)
{
  string s = "line: " + to_string (t.m_line) + " offset: "
             + to_string (t.m_offset) + " type: " + to_string (t.m_type);

  if (LX::Type::Int == t.m_type) { s += "(" + to_string (t.m_int) + ")"; }

  return s;
}

inline string
to_string (const LX::Tokens &tokens)
{
  string s {"[ \n"};

  for (size_t i = 0; i < tokens.size(); ++i)
  {
    LX::T t = tokens[i];
    s += to_string(t);
    s += i == tokens.size() - 1 ? "\n]\n" : ",\n";
  }

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

bool group (const Tokens &tokens, // in
            size_t begin,         // in
            size_t end,           // in
            Groups &groups        // out
);

Tokens run (std::string str);

} // namespace LX

#endif // LX_HEADER
