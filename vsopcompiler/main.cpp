#include <iostream>
#include <string>

#include "driver.hpp" // /!\ here error with parser.hpp not being present anymore
#include "ast.hpp"
#include "firstPass.hpp"
#include "secondPass.hpp"
#include "thirdPass.hpp"
#include "fourthPass.hpp"
#include "codeGeneration.hpp"

using namespace std;

// Adapted from https://www.gnu.org/software/bison/manual/html_node/A-Complete-C_002b_002b-Example.html

enum class Mode {
    LEXER,
    PARSER,
    CHECKER
};

static const map<string, Mode> flag_to_mode = {
    {"-l", Mode::LEXER},
    {"-p", Mode::PARSER},
    {"-c", Mode::CHECKER},
    {"-i", Mode::CHECKER}
};

extern bool checkMode;
bool llvmMode;

int main(int argc, char const *argv[]) {
    Mode mode;
    string source_file;

    if (argc == 2) {
        mode = Mode::PARSER; //TODO change standard mode
        source_file = argv[1];
    }
    else if (argc == 3) {
        if (flag_to_mode.count(argv[1]) == 0) {
            cerr << "Invalid mode: " << argv[1] << endl;
            return -1;
        }
        mode = flag_to_mode.at(argv[1]);
        if (std::string(argv[1]).compare("-i") == 0) {
            llvmMode = true;
        } else {
            llvmMode = false;
        }
        source_file = argv[2];
    }
    else {
        cerr << "Usage: " << argv[0] << " [-l|-p|-c|-i] <source_file>" << endl;
        return -1;
    }

    VSOP::Driver driver = VSOP::Driver(source_file);

    int res;
    switch (mode) {
        case Mode::LEXER:
            res = driver.lex();

            driver.print_tokens();

            return res;

        case Mode::PARSER:
            res = driver.lex();
            if (res != 0) {return res;}

            // Parse the source code and generate the AST
            res = driver.parse();

            if (res == 0)
                cout << driver.res->evaluate() << endl;

            return res;

        case Mode::CHECKER:
            checkMode = true; // use in evaluate() to print type

            res = driver.lex();
            if (res != 0) {return res;}

            // Parse the source code and generate the AST
            res = driver.parse();
            if (res != 0) {return res;}

            FirstPass firstPass;
            driver.res->accept(&firstPass);

            SecondPass secondPass(firstPass.getClassInfoMap());
            driver.res->accept(&secondPass);

            ThirdPass thirdPass(firstPass.getClassInfoMap(), secondPass.getClassMethodTypes(), secondPass.getClassFieldTypes());
            driver.res->accept(&thirdPass);

            FourthPass fourthPass(firstPass.getClassInfoMap(), secondPass.getClassMethodTypes(), secondPass.getClassFieldTypes(), thirdPass.getClassMethodFormalTypes());
            driver.res->accept(&fourthPass);

            if (!llvmMode){
                if (res == 0) {
                    cout << driver.res->evaluate() << endl;
                }
            } else { //LLVM Mode
            
                CodeGeneration codeGeneration(
                    firstPass.getClassInfoMap(),
                    secondPass.getClassMethodTypes(),
                    secondPass.getClassFieldTypes(),
                    thirdPass.getClassMethodFormalTypes());
                driver.res->accept(&codeGeneration);
            }
            return res;
    }
    return 0;
}
