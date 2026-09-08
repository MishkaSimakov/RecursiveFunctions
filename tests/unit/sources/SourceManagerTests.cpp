#include <fcntl.h>
#include <unistd.h>

#include "SourceManagerTestCase.h"

namespace {

constexpr std::string_view kProgram = "main: () -> i64 = { return 0; }";

// open returns the lowest unused descriptor, so this number grows whenever a
// descriptor is leaked.
int lowest_unused_descriptor() {
  int fd = open("/dev/null", O_RDONLY);
  close(fd);
  return fd;
}

}  // namespace

TEST_F(SourceManagerTestCase, test_it_loads_read_only_file) {
  if (geteuid() == 0) {
    GTEST_SKIP() << "root opens files regardless of their permissions";
  }

  auto path = create_source(kProgram, std::filesystem::perms::owner_read);

  SourceManager source_manager;
  SourceView view = source_manager.load(path);

  ASSERT_EQ(view.string_view(), kProgram);
}

TEST_F(SourceManagerTestCase, test_it_closes_descriptor_after_load) {
  auto path = create_source(kProgram, std::filesystem::perms::owner_all);

  SourceManager source_manager;

  int before = lowest_unused_descriptor();
  source_manager.load(path);
  int after = lowest_unused_descriptor();

  ASSERT_EQ(before, after);
}
