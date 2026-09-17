FROM ubuntu:26.04

RUN apt update

# LLVM
ARG LLVM_VERSION=22

RUN apt-get update && apt-get install -y \
      llvm-${LLVM_VERSION}-dev llvm-${LLVM_VERSION}-tools clang-${LLVM_VERSION} \
      cmake git python3-venv \
      zstd libedit-dev libcurl4-openssl-dev

ENV PATH="/usr/lib/llvm-${LLVM_VERSION}/bin:$PATH"
ENV LLVM_DIR="/usr/lib/llvm-${LLVM_VERSION}/lib/cmake/llvm"

# LLVM lit
RUN python3 -m venv /lit-build && /lit-build/bin/pip install --no-input lit psutil
ENV PATH="$PATH:/lit-build/bin"

ENV CC="clang" CXX="clang++"

# psutil for lit
RUN apt install -y python3-psutil

# git
RUN apt install -y git

# cmake
RUN apt install -y cmake

# some libs for llvm
RUN apt install -y zstd libedit-dev libcurl4-openssl-dev