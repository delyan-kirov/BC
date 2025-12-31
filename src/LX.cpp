#include "LX.hpp"
#include <cassert>
#include <limits>
#include <string>

namespace LX
{
constexpr size_t TOKENIZER_FAILED = std::numeric_limits<size_t>::max ();
namespace
{

size_t
look_for_matching_parenthesis (LX::L &l, const char *input, size_t offset)
{
  auto trace = ER::Trace (l.m_arena, __FUNCTION__, l.m_events);
  size_t stack = 1;

  for (size_t idx = offset; input[idx]; ++idx)
  {
    char c = input[idx];
    if (')' == c) { stack -= 1; }
    else if ('(' == c) { stack += 1; }
    if (0 == stack)
    {
      trace << "Found matching paren at: " << std::to_string (idx).c_str ()
            << trace.end ();
      return idx;
    }
  }

  return TOKENIZER_FAILED;
}

} // namespace anonymous

char
L::next_char ()
{
  if (this->m_input[this->m_cursor])
  {
    char c = this->m_input[this->m_cursor];
    if ('\n' == c) { this->m_lines += 1; }
    this->m_cursor += 1;
    return c;
  }
  return '\0';
}

void
L::push_int ()
{
  auto trace = ER::Trace (this->m_arena, __FUNCTION__, this->m_events);
  int result = 0;

  LX::L l = *this; // save lexer
  std::string s{
    this->m_input[this->m_cursor
                  - 1 /* since we entered this function, the point where we
                         need to start parsing is offset by 1 */
  ]
  };
  for (char c = this->next_char (); c; c = this->next_char ())
  {
    if (!c) { break; }
    if (std::isdigit (c)) { s += c; }
    else { break; }
  }

  try
  {
    result = std::stoi (s.c_str (), nullptr, 10);

    LX::T t{ LX::Type::Int };
    t.as.m_int = result;
    this->m_tokens.push (t);
  }
  catch (std::exception &e)
  {
    std::cerr << "ERROR: " << e.what () << std::endl;
  }

  if (this->m_input[this->m_cursor])
  {
    // We parsed one char more, we need to go back one step
    this->m_cursor -= 1;
    (void)l;
  }
}

void
L::push_operator (char c)
{
  auto trace = ER::Trace (this->m_arena, __FUNCTION__, this->m_events);

  LX::Type t_type = LX::Type::Unknown;
  switch (c)
  {
  case '-': t_type = LX::Type::Minus; break;
  case '+': t_type = LX::Type::Plus; break;
  case '*': t_type = LX::Type::Mult; break;
  case '/': t_type = LX::Type::Div; break;
  case '%': t_type = LX::Type::Modulus; break;
  default : /* UNREACHABLE */; break;
  }

  this->m_tokens.push (LX::T{ t_type });
}

void
L::run ()
{
  auto trace = ER::Trace (this->m_arena, __FUNCTION__, this->m_events);

  for (char c = this->next_char ();       //
       c && this->m_cursor < this->m_end; //
       c = this->next_char ()             //
  )
  {
    switch (c)
    {
    case '-':
    case '+':
    case '*':
    case '/':
    case '%':
    {
      this->push_operator (c);
    }
    break;
    case '(':
    {
      LX::L new_l = *this;
      size_t group_begin = this->m_cursor + 1;
      size_t group_end = look_for_matching_parenthesis (
                             *this, this->m_input, this->m_cursor)
                         + 1;

      new_l.m_begin = group_begin;
      new_l.m_end = group_end;
      new_l.run ();

      LX::T token{ new_l.m_tokens };
      LX::T token_2{ new_l.m_tokens };

      this->m_tokens.push (token);
      this->m_cursor = group_end;
    }
    break;
    case ')':
    {
      assert (0 && "case ')'");
    }
    break;
    case ' ':
    {
      ; // Do nothing
    }
    break;
    case '\n':
    {
      ; // Do nothing
    }
    break;
    default:
    {
      this->push_int ();
    }
    break;
    }
  }
}

} // namespace LX
