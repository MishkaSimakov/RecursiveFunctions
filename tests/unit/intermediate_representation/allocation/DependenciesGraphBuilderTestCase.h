#pragma once

#include <gtest/gtest.h>

#include <unordered_set>

#include "intermediate_representation/allocation/DependenciesGraphBuilder.h"

using namespace IR;

class DependenciesGraphBuilderTestCase : public ::testing::Test {
 protected:
  using DependenciesListT = std::unordered_set<std::pair<size_t, size_t>>;

  static TemporaryDependenciesGraph build(const Function& function) {
    return DependenciesGraphBuilder()(function);
  }

  static void test_graph(const TemporaryDependenciesGraph& graph,
                         DependenciesListT dependencieses) {
    size_t count = graph.temporaries.size();

    for (size_t i = 0; i < count; ++i) {
      for (size_t j = i; j < count; ++j) {
        auto first_temp = graph.temporaries[i].temporary;
        auto second_temp = graph.temporaries[j].temporary;

        ASSERT_EQ(graph.edges[i][j], graph.edges[j][i]);

        bool has_dependency_in_graph =
            graph.edges[i][j] == -TemporaryDependenciesGraph::kInfinity;
        bool should_dependency_exist =
            dependencieses.erase({first_temp.index, second_temp.index}) != 0 ||
            dependencieses.erase({second_temp.index, first_temp.index}) != 0;

        ASSERT_EQ(has_dependency_in_graph, should_dependency_exist)
            << fmt::format(
                   "in dependency graph temporaries {} and {} should be {}",
                   first_temp, second_temp,
                   should_dependency_exist ? "-inf" : "0");
      }
    }

    ASSERT_TRUE(dependencieses.empty()) << fmt::format(
        "there must be dependency between {} and {}",
        dependencieses.begin()->first, dependencieses.begin()->second);
  }
};
