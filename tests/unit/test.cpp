#include <algorithm>

#include "gtest/gtest.h"

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);

  if (argc > 1)
  {
    if (argv[1] != std::string("--forceFail"))
    {
      throw "Unexpected argument";
    }
  }
  return RUN_ALL_TESTS();
}
