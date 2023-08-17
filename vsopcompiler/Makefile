CXX 			= clang++

CXXFLAGS 		= -Wall -Wextra

BISONFLAGS 		= -d

LLVM_CONFIG		= llvm-config 

LDFLAGS			= $(shell ${LLVM_CONFIG} --ldflags --system-libs --libs all)

EXEC			= vsopc

SRC				= main.cpp \
				  driver.cpp \
				  parser.cpp \
				  lexer.cpp \
				  ast.cpp \
				  firstPass.cpp \
				  secondPass.cpp \
				  thirdPass.cpp \
				  fourthPass.cpp \
				  codeGeneration.cpp

OBJ	  			= $(SRC:.cpp=.o)


all: $(EXEC)

main.o: driver.hpp parser.hpp

driver.o: driver.hpp parser.hpp

parser.o: driver.hpp parser.hpp

lexer.o: driver.hpp parser.hpp

$(EXEC): $(OBJ)
	$(CXX) -o $@ $(LDFLAGS) $(OBJ)

codeGeneration.o: codeGeneration.cpp
	$(CXX) $(shell ${LLVM_CONFIG} --cxxflags) -c -o $@ $<

parser.cpp: parser.y
	bison $(BISONFLAGS) -o parser.cpp $^

parser.hpp: parser.y
	bison $(BISONFLAGS) -o parser.cpp $^

lexer.cpp: lexer.lex
	flex $(LEXFLAGS) -o lexer.cpp $^

install-tools:

clean:
	@rm -f $(EXEC)
	@rm -f $(OBJ)
	@rm -f lexer.cpp
	@rm -f parser.cpp parser.hpp location.hh

.PHONY: clean
