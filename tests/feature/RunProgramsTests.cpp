#include <fmt/format.h>
#include <fmt/ranges.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <random>

const auto kCompilerPath = TESTS_RUNTIME_DIR "/cli";
const auto kProgramsPath = TESTS_RUNTIME_DIR "/programs";
const auto kTestingLibraryPath = TESTS_RUNTIME_DIR "/liblibrary.a";
const auto kLLCPath = "/opt/homebrew/Cellar/llvm/19.1.7_1/bin/llc";
const auto kCommunicationFilePath = TESTS_PROGRAMS_COMMUNICATION_FILE;

std::filesystem::path get_temp_filepath() {
  auto temp_dir = std::filesystem::temp_directory_path();

  // random numbers generator
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis;

  int random_num = dis(gen);
  return temp_dir / std::to_string(random_num);
}

std::filesystem::path compile_tea_program(std::string program) {
  // save program to temp file
  auto program_filepath = get_temp_filepath();
  std::ofstream os(program_filepath);
  os << program;
  os.close();

  std::vector<std::string> parts;
  parts.push_back(kCompilerPath);

  // add path to program file
  parts.push_back(program_filepath);

  // specify output directory
  auto ir_filepath = get_temp_filepath();
  parts.push_back("-o");
  parts.push_back(ir_filepath);

  // compile program to llvm ir
  std::string command = fmt::format("{}", fmt::join(parts, " "));
  std::system(command.c_str());

  // compile program to asm
  auto asm_filepath = get_temp_filepath();
  asm_filepath.replace_extension(".s");

  std::string llc_command = fmt::format(
      "{} {} -o {}", kLLCPath, ir_filepath.c_str(), asm_filepath.c_str());
  std::system(llc_command.c_str());

  return asm_filepath;
}

std::filesystem::path compile_and_link_for_testsing(std::string program) {
  auto tea_program = compile_tea_program(program);
  auto exe_filepath = get_temp_filepath();

  auto link_command = fmt::format("clang++ {} {} -o {}", tea_program.c_str(),
                                  kTestingLibraryPath, exe_filepath.c_str());
  std::system(link_command.c_str());

  return exe_filepath;
}

bool compare_output(std::string_view expected, std::string_view real) {
  return expected == real;
}

TEST(ProgramTests, program_tests) {
  for (auto const& dir_entry :
       std::filesystem::directory_iterator{kProgramsPath}) {
    auto path = dir_entry.path();
    if (path.extension() != ".tea") {
      continue;
    }

    std::cout << "Testing " << path << std::endl;

    std::string expected_output;
    std::string program_text;
    std::string buffer;

    std::ifstream is(path);
    while (std::getline(is, buffer)) {
      if (buffer == "//") {
        break;
      }

      expected_output.append(buffer);
      expected_output.append("\n");
    }

    program_text.append("extern output: (value: i64) -> ()\n");
    while (std::getline(is, buffer)) {
      program_text.append(buffer);
      program_text.append("\n");
    }

    auto executable = compile_and_link_for_testsing(program_text);

    // clear testing channel
    {
      std::ofstream os(kCommunicationFilePath,
                       std::ofstream::out | std::ofstream::trunc);
      os.close();
    }

    std::system(executable.c_str());

    // test that output match expected output
    {
      std::ifstream is(kCommunicationFilePath);
      std::string real_output;
      while (std::getline(is, buffer)) {
        real_output.append(buffer);
        real_output.append("\n");
      }

      bool is_equal = compare_output(expected_output, real_output);
      if (!is_equal) {
        FAIL() << fmt::format("expected:\n{}\ngot:\n{}\n", expected_output,
                              real_output);
      }
    }
  }
}
