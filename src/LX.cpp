#include "LX.hpp"
#include "ER.hpp"
#include <cassert>

namespace LX
{
namespace
{

size_t
look_for_matching_parenthesis (LX::L &l, const char *input, size_t offset)
{
  auto trace = ER::Trace (l.m_arena, __PRETTY_FUNCTION__, l.m_events);
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

  l.m_events.push (ER::ErrorE{
      __PRETTY_FUNCTION__, (void *)" could not find matching parenthesis" });
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

size_t
L::push_int ()
{
  auto trace = ER::Trace (this->m_arena, __PRETTY_FUNCTION__, this->m_events);
  int result = 0;
  size_t cursor = this->m_cursor;
  size_t lines = this->m_lines;

  std::string s{
    this->m_input[this->m_cursor
                  - 1 /* since we entered this function, the point
                         where we need to start parsing is offset by 1 */
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
    this->m_events.push (
        ER::ErrorE{ __PRETTY_FUNCTION__, (void *)" could not parse integer" });
    this->m_cursor = cursor;
    this->m_lines = lines;
    return TOKENIZER_FAILED;
  }

  if (this->m_input[this->m_cursor])
  {
    // We parsed one char more, we need to go back one step
    this->m_cursor -= 1;
  }

  return this->m_cursor;
}

void
L::push_operator (char c)
{
  auto trace = ER::Trace (this->m_arena, __PRETTY_FUNCTION__, this->m_events);

  LX::Type t_type = LX::Type::Unknown;
  switch (c)
  {
  case '-': t_type = LX::Type::Minus; break;
  case '+': t_type = LX::Type::Plus; break;
  case '*': t_type = LX::Type::Mult; break;
  case '/': t_type = LX::Type::Div; break;
  case '%': t_type = LX::Type::Modulus; break;
  default : /* UNREACHABLE */ assert (false && "push_operator");
  }
  this->m_tokens.push (LX::T{ t_type });
}

size_t
L::run ()
{
  auto trace = ER::Trace (this->m_arena, __PRETTY_FUNCTION__, this->m_events);

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
      size_t result = 0;

      size_t group_begin = this->m_cursor + 1;
      size_t group_end = 0;

      result = look_for_matching_parenthesis (
          *this, this->m_input, this->m_cursor);
      if (LX::TOKENIZER_FAILED == result)
      {
        this->m_events.push (
            ER::ErrorE{ __PRETTY_FUNCTION__, (void *)" function run failed" });
        return result;
      }
      else { group_end = result + 1; }

      LX::L new_l = LX::L (*this, group_begin, group_end);
      result = new_l.run ();

      if (LX::TOKENIZER_FAILED != result) { this->subsume_sub_lexer (new_l); }
      else
      {
        this->m_events.push (
            ER::ErrorE{ __PRETTY_FUNCTION__, (void *)" new_l failed" });
        return result;
      }
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
      size_t result = this->push_int ();
      if (TOKENIZER_FAILED == result) { return result; }
    }
    break;
    }
  }

  return this->m_cursor;
}

} // namespace LX
