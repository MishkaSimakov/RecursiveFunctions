FROM teeks99/gcc-ubuntu:14

# installing cmake
RUN apt-get update && apt-get install -y cmake

# updating std library version
RUN add-apt-repository ppa:ubuntu-toolchain-r/test && \
    apt-get update && \
    apt-get install --only-upgrade -y libstdc++6

# installing gtest
RUN git clone https://github.com/google/googletest.git -b v1.14.0 && \
    cd googletest && \
    mkdir build && \
    cd build && \
    cmake .. && \
    cmake --build . && \
    cmake --install .