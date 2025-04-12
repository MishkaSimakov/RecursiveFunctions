FROM ubuntu:plucky

RUN apt update

# LLVM
RUN apt install -y llvm llvm-dev

# Clang and co
RUN apt install -y clang clang-tools libclang-dev libclang1 clang-format python3-clang clangd clang-tidy

# LLVM lit
RUN apt install -y python3-pip
RUN apt install -y python3-venv
RUN python3 -m venv /lit-build
RUN /lit-build/bin/python3 -m pip install --no-input lit
ENV PATH="$PATH:/lit-build/bin/"

# git
RUN apt install -y git

# cmake
RUN apt install -y cmake

# some libs for llvm
RUN apt install -y zstd libedit-dev curl libcurl4-openssl-dev

## create symbolic links to required programs
RUN ln /usr/bin/FileCheck-20 /usr/bin/FileCheck

# use clang as compiler
ENV CC="clang"
ENV CXX="clang++"