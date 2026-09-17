# This script builds every example in examples directory

mkdir build
cd build || exit 1
cmake -DCMAKE_BUILD_TYPE=Debug .. || exit 1
cmake --build . -t tlang || exit 1

cd .. || exit 1

for dir in examples/*/; do
  (cd "$dir" && make clean && make main TLANG=../../build/bin/tlang) || exit 1;
done