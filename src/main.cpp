#include "ER.hpp"
#include "LX.hpp"

int
main ()
{
  const char *input = "(()) ()";

  AR::T arena{};
  LX::L l{ input, arena, 0, std::strlen (input) + 1 };
  {
    ER::Trace trace{ arena, __FUNCTION__, l.m_events };

    trace << input << trace.end ();
    l.run ();

    trace << std::to_string (l.m_tokens).c_str () << trace.end ();
  }
  l.m_events.dump_to_stdin ();
}
