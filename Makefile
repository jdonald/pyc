CXX = g++
CXXFLAGS = -std=c++14 -Wall -Wno-unused-function
LEX = flex
YACC = bison

TARGET = pyc
OBJS = main.o codegen.o parser.tab.o lex.yy.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

main.o: main.cpp ast.h codegen.h parser.tab.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp

codegen.o: codegen.cpp codegen.h ast.h
	$(CXX) $(CXXFLAGS) -c codegen.cpp

parser.tab.cpp parser.tab.hpp: parser.y ast.h
	$(YACC) -d -o parser.tab.cpp parser.y

parser.tab.o: parser.tab.cpp ast.h
	$(CXX) $(CXXFLAGS) -c parser.tab.cpp

lex.yy.cpp: lexer.l parser.tab.cpp
	$(LEX) -o lex.yy.cpp lexer.l

lex.yy.o: lex.yy.cpp parser.tab.cpp
	$(CXX) $(CXXFLAGS) -c lex.yy.cpp

clean:
	rm -f $(TARGET) $(OBJS) parser.tab.cpp parser.tab.hpp lex.yy.cpp
	rm -f *.ll *.o *.out test_*.py

.PHONY: all clean
