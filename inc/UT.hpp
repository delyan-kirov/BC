#ifndef UT_HEADER
#define UT_HEADER

#include "AR.hpp"
#include <cstddef>
#include <cstdlib>
#include <cstring>

#if defined(__GNUC__) || defined(__clang__)
#define UT_PRINTF_LIKE(fmt_idx, arg_idx) __attribute__((format(printf, fmt_idx, arg_idx)))
#else
#define UT_PRINTF_LIKE(fmt_idx, arg_idx)
#endif

namespace UT
{

constexpr size_t V_DEFAULT_MAX_LEN = 1 << 6;

template <typename O> struct V
{
  size_t m_len;
  size_t m_max_len;
  O *m_mem;
  AR::T *m_arena;

  V () = default;
  ~V () = default;

  V (AR::T &arena, size_t len = 0)
      : m_len{ 0 }, m_max_len{ V_DEFAULT_MAX_LEN }, m_arena{ &arena }
  {
    size_t alloc_len = (0 == len)
                           ? V_DEFAULT_MAX_LEN
                           : len;
    this->m_mem = (O *)arena.alloc<O> (alloc_len);
  };

  O &
  operator[] (size_t i)
  { // for writing
    return this->m_mem[i];
  }

  const O &
  operator[] (size_t i) const
  { // for reading from const objects
    return this->m_mem[i];
  }

  void
  push (O o)
  {
    if (this->m_len >= this->m_max_len)
    {
      // We need more space
      O *new_mem
          = (O *)this->m_arena->alloc (sizeof (O) * 2 * this->m_max_len);
      std::memcpy (new_mem, this->m_mem, sizeof (O) * this->m_len);

      this->m_mem = new_mem;
      this->m_max_len *= 2;
    }
    this->m_mem[this->m_len] = o;
    this->m_len += 1;
  }
};
}

#endif // UT_HEADER
