
bin\win_bison --debug -d generator/calc.y -o src/calc.tab.cc
bin\win_flex --wincompat -o src/lex.yy.cc --header-file=src/lex.yy.h generator/calc.l
