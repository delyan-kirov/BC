#include "TL.hpp"
#include "UT.hpp"
#include <cstdio>

constexpr UT::String sut_file        = "./dat/addition.se";
constexpr UT::String sut_file_basic  = "./dat/basic.se";
constexpr UT::String sut_file_raylib = "./dat/raylib.se";

constexpr bool RUN        = true;
constexpr bool RUN_BASIC  = false;
constexpr bool RUN_RAYLIB = false;

int
main()
{
  AR::Arena arena{};

  if (RUN)
  {
    TL::Mod mod(sut_file, arena);
  }
  else if (RUN_BASIC)
  {
    TL::Mod mod_basic(sut_file_basic, arena);
  }
  else if (RUN_RAYLIB)
  {
    TL::Mod mod_raylib(sut_file_raylib, arena);
  }
  else
  {
    std::printf("tst_addition_n_subtraction -> OK [no target ran]\n");
  }
}
