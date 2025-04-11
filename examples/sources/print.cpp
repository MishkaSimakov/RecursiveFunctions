#include <iostream>

struct Rectangle {
  int64_t x;
  int64_t y;
  int64_t width;
  int64_t height;

  Rectangle(int64_t x, int64_t y, int64_t width, int64_t height):
    x(x),
    y(y),
    width(width),
    height(height) {}
};

void print(int64_t value) {
  std::cout << value << std::endl;
}

Rectangle get_rectangle() {
  return Rectangle(1, 2, 3, 4);
}

