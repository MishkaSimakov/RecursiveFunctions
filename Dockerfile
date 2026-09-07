FROM ubuntu:26.04

RUN apt update

# a few tools for LLVM installation
RUN apt install -y lsb-release wget software-properties-common gnupg

# LLVM 23
# llvm.sh can break if ubuntu version becomes outdated.
# Information about this behaviour can be found on https://apt.llvm.org.
RUN wget https://apt.llvm.org/llvm.sh && \
    chmod +x llvm.sh && \
    ./llvm.sh 23
RUN apt install -y llvm-23-dev llvm-23-tools
ENV PATH="/usr/lib/llvm-23/bin:$PATH"
ENV LLVM_DIR="/usr/lib/llvm-23/lib/cmake/llvm"

# LLVM lit
RUN apt install -y python3-pip
RUN apt install -y python3-venv
RUN python3 -m venv /lit-build
RUN /lit-build/bin/python3 -m pip install --no-input lit
ENV PATH="$PATH:/lit-build/bin/"

# psutil for lit
RUN apt install -y python3-psutil

# git
RUN apt install -y git

# cmake
RUN apt install -y cmake

# some libs for llvm
RUN apt install -y zstd libedit-dev curl libcurl4-openssl-dev

# use clang as compiler
ENV CC="clang"
ENV CXX="clang++"