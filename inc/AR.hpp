#ifndef ARENA_HEADER
#define ARENA_HEADER

#include <cstddef>
#include <cstdint>

namespace AR
{
constexpr size_t BLOCK_DEFAULT_LEN = (1 << 10);

class T;
class Block
{
  friend class T;

private:
  size_t len;
  size_t max_len;
  uint8_t mem[];

  Block () = delete;
  ~Block () = delete;

  Block (const Block &) = delete;
  Block &operator= (const Block &) = delete;

  Block (Block &&) = delete;
  Block &operator= (Block &&) = delete;
};

class T
{
public:
  void *alloc (size_t size);

  template <typename Type>
  void *
  alloc ()
  {
    return alloc (sizeof (Type));
  }

  template <typename Type>
  void *
  alloc (Type *t)
  {
    return alloc (sizeof (t));
  }

  T ();
  ~T ();

private:
  size_t len;
  size_t max_len;
  Block **mem;
};
}

#endif
