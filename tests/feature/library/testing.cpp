#include <fstream>

constexpr auto kCommunicationFilePath = TESTS_PROGRAMS_COMMUNICATION_FILE;

void output(int64_t value) {
  std::ofstream os(kCommunicationFilePath, std::ios::app);
  if (!os) {
    throw std::runtime_error("Failed to open communication file.");
  }

  os << value << std::endl;
  os.close();
}
