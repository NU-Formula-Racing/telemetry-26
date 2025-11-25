#include <iostream>

#include "app/app.hpp"

int main() {
  App app;
  int result = app.run();
  std::cout << result << std::endl;

  return 0;
}