# This script builds and runs all tests for this project
# You can run this script using docker image described in Dockerfile

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . -t tests.unit
./tests/unit/tests.unit --gtest_output="xml:unit-report.xml"