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
flex -o src/lex.yy.cc generator/calc.l

#compile
g++ -std=c++17 src/calc.tab.cc src/lex.yy.cc -o calc -lm
