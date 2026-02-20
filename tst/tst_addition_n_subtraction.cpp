#include "TL.hpp"
#include "UT.hpp"

constexpr UT::String sut_file       = "./dat/addition.se";
constexpr UT::String sut_file_basic = "./dat/basic.se";

int
main()
{
  AR::Arena arena{};
  (void)sut_file_basic;
  (void)sut_file;

  TL::Mod mod(sut_file_basic, arena);
}
