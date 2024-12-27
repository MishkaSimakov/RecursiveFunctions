#pragma once
#include <iostream>

class Serializer {
protected:
  static void write_bytes(size_t value, std::ostream& os) {
    os.write(reinterpret_cast<const char*>(&value), sizeof(size_t));
  }

  static size_t read_bytes(std::istream& is) {
    size_t result;
    is.read(reinterpret_cast<char*>(&result), sizeof(size_t));
    return result;
  }
};