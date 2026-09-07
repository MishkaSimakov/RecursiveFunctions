# This script builds and runs all tests for this project
# You can run this script using docker image described in Dockerfile

mkdir build
cd build || exit 1
cmake -DCMAKE_BUILD_TYPE=Debug .. || exit 1
cmake --build . -t tests.unit cli tests.lit.execution.library || exit 1

./tests/unit/tests.unit --gtest_output="xml:unit-report.xml"
unit_status=$?

lit tests/lit -v --timeout=10 --xunit-xml-output=lit-report.xml
lit_status=$?

[ "$unit_status" -eq 0 ] && [ "$lit_status" -eq 0 ]
