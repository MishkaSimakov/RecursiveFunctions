#include <numeric>

#include "PreprocessorTestCase.h"

TEST_F(PreprocessorTestCase, test_it_process_main_source) {
  vector program = {"successor(123);"};
  add_main_source(program);

  auto result = preprocessor.process();
  ASSERT_EQ(result, program[0]);
}

TEST_F(PreprocessorTestCase, test_it_concatenate_lines) {
  vector<string> program = {"f(x,y)=x;", "g(x,y)=y;", "successor(123);"};
  add_main_source(program);

  auto result = preprocessor.process();
  ASSERT_EQ(result, std::reduce(program.begin(), program.end()));
}

TEST_F(PreprocessorTestCase, test_it_remove_spaces) {
  ASSERT_EQ(
      process_program(vector{"f(x, y) = 123;", "g ( x , y ) = test(   )  ;"}),
      "f(x,y)=123;g(x,y)=test();");

  ASSERT_EQ(process_program(vector{"argmin( f ( f ( * ) ) );"}),
            "argmin(f(f(*)));");

  ASSERT_EQ(process_program(vector{"test_test(y, x + 1)  =  g( x );",
                                   "test(y, 0) = f () ;"}),
            "test_test(y,x+1)=g(x);test(y,0)=f();");

  const string special = "\t  \t ";
  vector<string> parts = {"test", "(", "x", ",", "y", ")", ";"};

  string program = special;
  for (const auto& part : parts) {
    program += special + part;
  }

  ASSERT_EQ(process_program(vector{program}), "test(x,y);");

  ASSERT_EQ(process_program(vector{"f(x) = ", "g(", "x, y, 123", ")"}),
            "f(x)=g(x,y,123)");
}

TEST_F(PreprocessorTestCase, test_it_preserve_space_when_concatenate_tokens) {
  ASSERT_EQ(process_program(vector{"f(a  b ) = 12 32;"}), "f(a b)=12 32;");
  ASSERT_EQ(process_program(vector{"f g(x + 1 2 3)=g(x y z);"}),
            "f g(x+1 2 3)=g(x y z);");
  ASSERT_EQ(process_program(vector{"x  y  \t z;", "t 123;"}), "x y z;t 123;");
  ASSERT_EQ(process_program(vector{"12 x y 32", "21 abc();"}),
            "12 x y 32 21 abc();");
  ASSERT_EQ(process_program(vector{"f(x) = ", "g(", "x, y", "   123", ")"}),
            "f(x)=g(x,y 123)");
}

TEST_F(PreprocessorTestCase, test_simple_includes) {
  auto include_path = add_file("file", vector{"add(x)=1;"});
  preprocessor.add_source<FileSource>("file", include_path);

  add_main_source(vector{"#include \"file\"", "test123;"});

  ASSERT_EQ(preprocessor.process(), "add(x)=1;test123;");

  reset_preprocessor();

  auto first = add_file("source1", vector{"add(x)=1;"});
  preprocessor.add_source<FileSource>("include1", first);

  auto second = add_file("source2", vector{"add(x)=2;"});
  preprocessor.add_source<FileSource>("include2", second);

  add_main_source(
      vector{"#include \"include1\"", "#include \"include2\"", "test123;"});

  ASSERT_EQ(preprocessor.process(), "add(x)=1;add(x)=2;test123;");
}

TEST_F(PreprocessorTestCase, test_it_throws_when_duplicate_source) {
  auto first = add_file("first", vector{""});
  auto second = add_file("second", vector{""});

  preprocessor.add_source<FileSource>("test_name", first);
  ASSERT_THROW({ preprocessor.add_source<FileSource>("test_name", second);
  }, std::runtime_error);
}

TEST_F(PreprocessorTestCase, test_it_throws_when_no_main_source) {
  ASSERT_THROW({ preprocessor.process();}, Preprocessing::MainSourceNotFoundException);
}

TEST_F(PreprocessorTestCase, test_chained_include) {
  auto child = add_file("child", vector{"#include \"grandchild1\"", "child()",
                                        "#include \"grandchild2\""});
  auto grandchild1 = add_file("grandchild1", vector{"grandchild1()"});
  auto grandchild2 = add_file("grandchild2", vector{"grandchild2()"});

  preprocessor.add_source<FileSource>("child", child);
  preprocessor.add_source<FileSource>("grandchild1", grandchild1);
  preprocessor.add_source<FileSource>("grandchild2", grandchild2);

  add_main_source(vector{"#include \"child\"", "main_code()"});

  ASSERT_EQ(preprocessor.process(),
            "grandchild1()child()grandchild2()main_code()");
}

TEST_F(PreprocessorTestCase, test_diamond_include) {
  auto child = add_file("child", vector{"#include \"grandchild1\"", "child()",
                                        "#include \"grandchild2\""});
  auto grandchild1 = add_file(
      "grandchild1", vector{"#include \"grandgrandchild\"", "grandchild1()"});
  auto grandchild2 = add_file(
      "grandchild2", vector{"grandchild2()", "#include \"grandgrandchild\""});

  auto grandgrandchild =
      add_file("grandgrandchild", vector{"grandgrandchild()"});

  preprocessor.add_source<FileSource>("child", child);
  preprocessor.add_source<FileSource>("grandchild1", grandchild1);
  preprocessor.add_source<FileSource>("grandchild2", grandchild2);
  preprocessor.add_source<FileSource>("grandgrandchild", grandgrandchild);

  add_main_source(vector{"#include \"child\"", "main_code()"});

  ASSERT_EQ(preprocessor.process(),
            "grandgrandchild()grandchild1()child()grandchild2()main_code()");
}

TEST_F(PreprocessorTestCase, test_it_throws_when_include_not_found) {
  add_main_source(vector{"#include \"testingtest\""});

  ASSERT_THROW({ preprocessor.process(); }, Preprocessing::IncludeSourceNotFoundException);
}
