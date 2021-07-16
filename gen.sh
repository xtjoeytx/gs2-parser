#!/bin/bash
PATH="/opt/homebrew/opt/bison/bin:/opt/homebrew/opt/flex/bin:$PATH"
extra=""

while getopts d: flag
do
    case "${flag}" in
        d) extra=${OPTARG};;
    esac
done

# with no debug
#bison -d generator/calc.y -o src/calc.tab.cc

#with debug
bison --debug -d generator/calc.y -o src/calc.tab.cc $extra
# bison -d generator/calc.y -o src/calc.tab.cc -Wcounterexamples

# flex
flex -o src/lex.yy.cc --header-file=src/lex.yy.h generator/calc.l

#compile
g++ -std=c++17 src/calc.tab.cc src/lex.yy.cc src/main.cpp src/encoding/buffer.cpp src/ast.cpp src/Parser.cpp src/GS2CompilerVisitor.cpp src/GS2Bytecode.cpp -o calc -lm
