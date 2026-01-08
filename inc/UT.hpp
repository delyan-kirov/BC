#ifndef UT_HEADER
#define UT_HEADER

#include "AR.hpp"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <string>
#include <utility>

#if defined(__GNUC__) || defined(__clang__)
#define UT_PRINTF_LIKE(fmt_idx, arg_idx)                                      \
  __attribute__ ((format (printf, fmt_idx, arg_idx)))
#else
#define UT_PRINTF_LIKE(fmt_idx, arg_idx)
#endif

// TODO: `TODO` `UNREACHABLE` `ENUM UNREACHABLE` `ASSERT` `Views` macros

#define UT_TODO(_TODO_MSG) UT::IMPL::todo (__PRETTY_FUNCTION__, _TODO_MSG)

namespace UT
{

namespace IMPL
{
__attribute__ ((noreturn)) inline void
todo (const char *fn_name, const char *msg)
{
  std::printf ("[TODO] %s: %s\n", fn_name, msg);
  abort ();
}

__attribute__ ((noreturn)) inline void
todo (const char *fn_name, std::string msg)
{
  std::printf ("[TODO] %s: %s\n", fn_name, msg.c_str ());
  abort ();
}

__attribute__ ((noreturn)) inline void
todo (const char *fn_name)
{
  std::printf ("[TODO] %s\n", fn_name);
  abort ();
}

__attribute__ ((noreturn)) inline void
assert (const char *fn_name, const char *msg)
{
  std::printf ("[ERROR] %s: %s\n", fn_name, msg);
  abort ();
}

__attribute__ ((noreturn)) inline void
assert (const char *fn_name)
{
  std::printf ("[ERROR] asserting failure %s\n", fn_name);
  abort ();
}

__attribute__ ((noreturn)) inline void
unreachable (const char *fn_name, const char *msg)
{
  std::printf ("[ERROR] UNREACHABLE BRANCH ENTERED (%s): %s\n", fn_name, msg);
  abort ();
}

__attribute__ ((noreturn)) inline void
unreachable (const char *fn_name, std::string msg)
{
  std::printf (
      "[ERROR] UNREACHABLE BRANCH ENTERED (%s): %s\n", fn_name, msg.c_str ());
  abort ();
}

__attribute__ ((noreturn)) inline void
unreachable (const char *fn_name)
{
  std::printf ("[ERROR] UNREACHABLE BRANCH ENTERED (%s)\n", fn_name);
  abort ();
}

__attribute__ ((noreturn)) inline void
unreachable_enum (ssize_t enum_value, const char *fn_name, const char *msg)
{
  std::printf ("[ERROR] UNREACHABLE ENUM(%ld) BRANCH ENTERED (%s): %s\n",
               enum_value,
               fn_name,
               msg);
  abort ();
}

__attribute__ ((noreturn)) inline void
unreachable_enum (ssize_t enum_value, const char *fn_name, std::string msg)
{
  std::printf ("[ERROR] UNREACHABLE ENUM(%ld) BRANCH ENTERED (%s): %s\n",
               enum_value,
               fn_name,
               msg.c_str ());
  abort ();
}

__attribute__ ((noreturn)) inline void
unreachable_enum (ssize_t enum_value, const char *fn_name)
{
  std::printf ("[ERROR] UNREACHABLE ENUM(%ld) BRANCH ENTERED (%s)\n",
               enum_value,
               fn_name);
  abort ();
}

} // namespace UT::IMPL

constexpr size_t V_DEFAULT_MAX_LEN = 1 << 6;

template <typename O> struct V
{
  size_t m_len;
  size_t m_max_len;
  O *m_mem;
  AR::T *m_arena;

  V () = default;
  ~V () = default;
  V (const V &other) = default;
  V &operator= (const V &) = default;

  V (V &&other) : V{ other }
  {
    other.m_arena = nullptr;
    other.m_len = 0;
    other.m_max_len = 0;
    other.m_mem = nullptr;
  }

  V (AR::T &arena, size_t len = 0)
      : m_len{ 0 }, m_max_len{ V_DEFAULT_MAX_LEN }, m_arena{ &arena }
  {
    size_t alloc_len = (0 == len) ? V_DEFAULT_MAX_LEN : len;
    this->m_mem = (O *)arena.alloc<O> (alloc_len);
  };

  V (std::initializer_list<size_t> lst)
      : m_arena{ 0 }, m_len{ 0 }, m_max_len{ 0 }, m_mem{ 0 }
  {
    (void)lst;
  };

  V (AR::T &arena, std::initializer_list<O> lst)
      : m_arena{ 0 }, m_len{ 0 }, m_max_len{ V_DEFAULT_MAX_LEN }, m_mem{ 0 }
  {
    this->m_mem = (O *)arena.alloc<O> (this->m_max_len);
    for (const O &o : lst)
    {
      this->push (o);
    }
  };

  const O *
  begin () const
  {
    return this->m_mem;
  };
  const O *
  end () const
  {
    return this->m_mem + this->m_len;
  };
  O *
  begin ()
  {
    return this->m_mem;
  };
  O *
  end ()
  {
    return this->m_mem + this->m_len;
  };

  O *
  last ()
  {
    return this->m_mem + (this->m_len - 1);
  };

  O &
  operator[] (size_t i)
  { // for writing
    return this->m_mem[i];
  };

  const O &
  operator[] (size_t i) const
  { // for reading from const objects
    return this->m_mem[i];
  };

  void
  push (O o)
  {
    if (this->m_len >= this->m_max_len)
    {
      // We need more space
      O *new_mem
          = (O *)this->m_arena->alloc (sizeof (O) * 2 * this->m_max_len);
      std::memcpy ((void *)new_mem, this->m_mem, sizeof (O) * this->m_len);

      this->m_mem = new_mem;
      this->m_max_len *= 2;
    }
    this->m_mem[this->m_len] = o;
    this->m_len += 1;
  };
};

class SB
{
  size_t m_len;
  size_t m_max_len;
  char *m_mem;

public:
  SB () : m_len{ 0 }
  {
    this->m_mem = new char[sizeof (char) * V_DEFAULT_MAX_LEN];
    this->m_max_len = V_DEFAULT_MAX_LEN;
    std::memset (this->m_mem, 0, this->m_max_len);
  }

  static const char *
  strdup (const char *s)
  {
    SB sb{};
    sb.concat (s);
    return sb.collect ();
  }

  SB (const SB &) = delete;            // copy constructor
  SB &operator= (const SB &) = delete; // copy assignment
  SB (SB &&) = delete;                 // move constructor
  SB &operator= (SB &&) = delete;      // move assignment

  ~SB ()
  {
    if (this->m_mem)
    {
      delete[] this->m_mem;
      this->m_mem = nullptr;
    }
  }

  void
  resize (size_t new_len)
  {
    size_t new_max_len = 2 * (this->m_max_len + new_len);
    char *new_mem = new char[new_max_len];
    std::memset (new_mem, 0, new_max_len);
    std::strcpy (new_mem, this->m_mem);
    delete[] this->m_mem;
    this->m_mem = new_mem;
    this->m_max_len = new_max_len;
  }

  void
  add (const char *s)
  {
    size_t available_space = this->m_max_len - this->m_len;
    size_t s_len = std::strlen (s);
    if (available_space < s_len) { this->resize (s_len); }
    std::strcat (this->m_mem, s);
    this->m_len += s_len;
  }

  template <typename... Args> void concat (Args &&...args);
  template <typename... Args> void concatf (const char *fmt, Args &&...args);
  template <typename... Args> void append (Args &&...args);

  const char *
  collect ()
  {
    const char *mem = this->m_mem;
    this->m_mem = nullptr;
    return mem;
  }
};

template <typename... Args>
void
SB::concatf (const char *fmt, Args &&...args)
{
  char *buffer;
  asprintf (&buffer, fmt, std::forward<Args> (args)...);
  this->concat (buffer);
  free (buffer);
}

template <typename... Args>
void
SB::concat (Args &&...args)
{
  (..., this->add (std::forward<Args> (args)));
}

template <typename... Args>
void
SB::append (Args &&...args)
{
  (..., this->concat (std::forward<Args> (args), " "));
}

class B : public V<char>
{
public:
  B () = delete;

  B (AR::T &arena, size_t size)
  {
    this->m_arena = &arena;
    this->m_len = 0;
    this->m_max_len = size;
    this->m_mem = (char *)arena.alloc (this->m_max_len * sizeof (char));
    std::memset (this->m_mem, 0, this->m_max_len);
  }

  B (AR::T &arena) : B{ arena, V_DEFAULT_MAX_LEN } {}

  using V<char>::push;
  void
  push (const char *s)
  {
    if (!s || !(*s)) { return; }
    size_t s_len = std::strlen (s);
    size_t available_mem = this->m_max_len - this->m_len;
    if (s_len >= available_mem)
    {
      size_t new_max_len = 2 * (this->m_max_len + s_len);
      char *new_mem
          = (char *)this->m_arena->alloc (new_max_len * sizeof (char));
      std::memset (new_mem, 0, new_max_len);
      std::memcpy (new_mem, this->m_mem, this->m_len);
      this->m_max_len = new_max_len;
      this->m_mem = new_mem;
    }
    std::memcpy (this->m_mem + this->m_len, s, s_len);
    this->m_len += s_len;
  }

  B &
  operator<< (char c)
  {
    this->push (c);
    return *this;
  }

  B &
  operator<< (const char *s)
  {
    while (*s)
    {
      char c = *s;
      this->push (c);
      ++s;
    }
    return *this;
  }
};
}

#endif // UT_HEADER
