#include <chrono>
#include <iostream>
#include <thread>

void print(char* str) { std::cout << str; }
void println(char* str) { std::cout << str << std::endl; }

void print_i64(long long value) { std::cout << value; }
void println_i64(long long value) { std::cout << value << std::endl; }

long long read_i64() {
  long long result;
  std::cin >> result;
  return result;
}

void sleep_ms(long long ms) {
  std::cout.flush();
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void print_slow(char* str, long long ms_per_char) {
  for (char* it = str; *it != '\0'; ++it) {
    std::cout << *it << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(ms_per_char));
  }
}