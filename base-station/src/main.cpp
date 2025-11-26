#include "app/app.hpp"

extern "C" void BspInit(void);

int main() {
  BspInit();

  App app;

  while (true) {
    app.run();
  }

  return 0;
}