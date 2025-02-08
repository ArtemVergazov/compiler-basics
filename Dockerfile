FROM ubuntu

WORKDIR /ubuntu-nasm-compiler

RUN apt update && apt install -y cmake gdb g++ nasm
RUN mkdir build && mkdir debug

COPY . /ubuntu-nasm-compiler
