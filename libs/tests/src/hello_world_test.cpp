#include <pludux/pludux.hpp>
#include <gtest/gtest.h>

#include <sstream>

TEST(HelloWorldTest, hello_world)
{
  auto out_stream = std::ostringstream{};

  pludux::hello_world(out_stream);

  EXPECT_EQ(out_stream.str(), "Hello, Pludux!\n");
}
