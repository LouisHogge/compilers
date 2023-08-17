%{
    /* Includes */
    #include <string>
    #include <unordered_map>
    #include <sstream>
    #include <stack>

    #include "parser.hpp"
    #include "driver.hpp"

    std::string str = "";
    int comment_nesting = 0; 
%}

/* Flex options
     * - noyywrap: yylex will not call yywrap() function
     * - nounput: do not generate yyunput() function
     * - noinput: do not generate yyinput() function
     * - batch: tell Flex that the lexer will not often be used interactively
     */
%option noyywrap nounput noinput batch

%{
    /* Code to include at the beginning of the lexer file. */
    using namespace std;
    using namespace VSOP;

    // Create a new INTEGER_LITERAL token from the value s.
    Parser::symbol_type make_INTEGER_LITERAL(const string &s,
                                    const location &loc);
    // Create a new STRING_LITERAL token from the value s.
    Parser::symbol_type make_STRING_LITERAL(const string &s,
                                    const location &loc);
    // Create a new EOFERROR token
    Parser::symbol_type make_eoferror(const location& loc);

    // Print an lexical error message.
    static void print_error(const position &pos,
                            const string &m);

    // Code run each time a pattern is matched.
    #define YY_USER_ACTION  loc.columns(yyleng);

    // Global variable used to maintain the current location.
    location loc;
    location loc_string;
    stack<location> loc_comment;

    std::string string_content;
    const std::unordered_map<char, char> escaped_chars_map = {
        {'b', '\b'},
        {'t', '\t'},
        {'n', '\n'},
        {'r', '\r'}
    };

%}

/* Definitions */

/* LEXICAL STRUCTURE */
lowercase_letter [a-z]
uppercase_letter [A-Z]
letter {lowercase_letter}|{uppercase_letter}
bin_digit [01]
digit {bin_digit}|[2-9]
hex_digit {digit}|[a-f]|[A-F]

invalid_hex [g-z]|[G-Z]
invalid_hex_digit {hex_digit}*{invalid_hex}+{hex_digit}*

/* WHITESPACE */
whitespace [ \t\f\r]+

/* COMMENTS */
%x comment
lf "\n"
single_line_comment "//".*{lf}*

/* INTEGER LITERALS */
integer_literal_digit {digit}+
integer_literal_hex_digit "0x"{hex_digit}+
integer_literal {integer_literal_digit}|{integer_literal_hex_digit}

/* INVALID INTEGER LITERALS */
invalid_integer_literal_digit {integer_literal_digit}{letter}+{integer_literal_digit}*
invalid_integer_literal_hex_digit "0x"{invalid_hex_digit}+
invalid_integer_literal {invalid_integer_literal_digit}|{invalid_integer_literal_hex_digit}

/* TYPE IDENTIFIERS */
identifier {letter}|{digit}|_
type_identifier {uppercase_letter}{identifier}*

/* OBJECT IDENTIFIERS */
object_identifier {lowercase_letter}{identifier}*

/* STRING LITERALS */
%x string_literal
escaped_char \\{escape_sequence}
ws [ \t]
hex_sequence x{hex_digit}{hex_digit}
escape_sequence b|t|n|r|\"|\\|{hex_sequence}|({lf}{ws}*)
regular_char [^\\\n\"]

%%
%{
    // Code run each time yylex is called.
    loc.step();
%}
    /* RULES */

    /* WHITE SPACES */
    /* white space without line feed */
{whitespace}                    { loc.step(); }
    /* line feed */
\n+                             { loc.lines(yyleng); loc.step(); }

    /* COMMENTS */
{single_line_comment}           { loc.lines(); loc.step(); }
    /* multi-line comment */
"(*"                            { loc_comment.push(loc); BEGIN(comment); }
<comment>{
    /* start nested comment*/
    "(*"                        { ++comment_nesting; loc_comment.push(loc); }
    /* end comment */
    "*)"                        { 
                                    if (comment_nesting){
                                        --comment_nesting;
                                        loc_comment.pop();
                                    }
                                    else { BEGIN(INITIAL);}
                                }
    /* whitespaces in comments */
    [\t\f\r]+                   { loc.step(); loc.columns(yyleng); }
    /* all other character */
    .                           { loc.step(); }
    /* new lines */
    \n+                         { loc.lines(yyleng); loc.step(); }
    /* EOF in comment */
    <<EOF>>                     {
                                    print_error(loc_comment.top().begin, "multi-line comment not terminated");
                                    return make_eoferror(loc);
                                }
}

    /* INTEGER LITERALl */
{integer_literal}               { return make_INTEGER_LITERAL(yytext, loc); }

{invalid_integer_literal}       {
                                    print_error(loc.begin, "invalid integer literal " + string(yytext));
                                    return Parser::make_YYerror(loc);
                                }

    /* KEYWORDS */
and                             { return Parser::make_AND(loc); }
bool                            { return Parser::make_BOOL(loc); }
class                           { return Parser::make_CLASS(loc); }
do                              { return Parser::make_DO(loc); }
else                            { return Parser::make_ELSE(loc); }
extends                         { return Parser::make_EXTENDS(loc); }
false                           { return Parser::make_FALSE(loc); }
if                              { return Parser::make_IF(loc); }
in                              { return Parser::make_IN(loc); }
int32                           { return Parser::make_INT32(loc); }
isnull                          { return Parser::make_ISNULL(loc); }
let                             { return Parser::make_LET(loc); }
new                             { return Parser::make_NEW(loc); }
not                             { return Parser::make_NOT(loc); }
self                            { return Parser::make_SELF(loc); }
string                          { return Parser::make_STRING(loc); }
then                            { return Parser::make_THEN(loc); }
true                            { return Parser::make_TRUE(loc); }
unit                            { return Parser::make_UNIT(loc); }
while                           { return Parser::make_WHILE(loc); }

    /* TYPE IDENTIFIER */
{type_identifier}               { return Parser::make_TYPE_IDENTIFIER(yytext, loc); }

    /* OBJECT IDENTIFIER */
{object_identifier}             { return Parser::make_OBJECT_IDENTIFIER(yytext, loc); }

    /* STRING LITERAL*/
    /* start string */
\"                              {
                                    string_content = "";
                                    loc_string = loc;
                                    BEGIN(string_literal); 
                                }
<string_literal>{
    /* end string */
    \"                          {
                                    BEGIN(INITIAL);
                                    string_content += '\0';
                                    return make_STRING_LITERAL(string_content, loc_string);
                                }
    /* escaped character in string */
    {escaped_char}              {
                                    auto iterator = escaped_chars_map.find(yytext[1]);
                                    if (iterator != escaped_chars_map.end()) 
                                        string_content += iterator->second;
                                    else if (yytext[1] == 'x') { //chracter with byte value in hexadecimal
                                        std::stringstream ss;
                                        for (const char* c = &yytext[2]; *c; ++c) {
                                            if (*c == '\n') break; // stop at newline
                                            ss << *c;
                                        }
                                        int value;
                                        ss >> std::hex >> value;
                                        string_content += static_cast<char>(value);
                                    } else if (yytext[1] == '\n') { //backlash followed by new line
                                        string_content += '\\';
                                        string_content += yytext[1];
                                        loc.step(); 
                                        loc.lines(); 
                                        loc.columns(yyleng-2);
                                    } else
                                        string_content += yytext[1];
                                    loc.step(); 
                                }
    /* invalid escape sequence in string */
    \\                          { 
                                    print_error(loc.begin, "invalid escape sequence");
                                    return Parser::make_YYerror(loc);
                                }
    /* line feed in string */
    {lf}                        { 
                                    print_error(loc.begin, "raw line feed");
                                    return Parser::make_YYerror(loc);
                                }
    /* regular character in string */
    {regular_char}+             {
                                    string_content += yytext;
                                    loc.step(); 
                                }
    /* EOF in string */
    <<EOF>>                     {
                                    print_error(loc_string.begin, "string not terminated");
                                    return make_eoferror(loc);
                                }
}              

    /* OPERATORS */
"{"                             { return Parser::make_LBRACE(loc); }
"}"                             { return Parser::make_RBRACE(loc); }
"("                             { return Parser::make_LPAR(loc); }
")"                             { return Parser::make_RPAR(loc); }
":"                             { return Parser::make_COLON(loc); }
";"                             { return Parser::make_SEMICOLON(loc); }
","                             { return Parser::make_COMMA(loc); }
"+"                             { return Parser::make_PLUS(loc); }
"-"                             { return Parser::make_MINUS(loc); }
"*"                             { return Parser::make_TIMES(loc); }
"/"                             { return Parser::make_DIV(loc); }
"^"                             { return Parser::make_POW(loc); }
"."                             { return Parser::make_DOT(loc); }
"="                             { return Parser::make_EQUAL(loc); }
"<"                             { return Parser::make_LOWER(loc); }
"<="                            { return Parser::make_LOWER_EQUAL(loc); }
"<-"                            { return Parser::make_ASSIGN(loc); }

    /* INVALID CHARACTERS */
.                               {
                                    print_error(loc.begin, "invalid character: " + string(yytext));
                                    return Parser::make_YYerror(loc);
                                }
    
    /* END OF FILE */
<<EOF>>                         { return Parser::make_YYEOF(loc); }
%%

    /* User code */
Parser::symbol_type make_eoferror(const location& loc) {
    return Parser::make_EOFERROR(loc);
}

Parser::symbol_type make_INTEGER_LITERAL(const string &s,
                                const location& loc) {
    std::size_t pos = 0;
    std::string sNoLeadingZeros;

    if (s.substr(0, 2) == "0x") {
        // Handle hexadecimal literals
        pos = s.find_first_not_of("0123456789abcdefABCDEF", 2);
        if (pos == std::string::npos)
            sNoLeadingZeros = s;
        else
            sNoLeadingZeros = s.substr(2, pos - 2) + s.substr(pos);
    } else {
        // Handle decimal literals
        pos = s.find_first_not_of('0');
        if (pos == std::string::npos)
            sNoLeadingZeros = s;
        else
            sNoLeadingZeros = s.substr(pos);
    }

    int n = stoi(sNoLeadingZeros, nullptr, 0);

    return Parser::make_INTEGER_LITERAL(n, loc);
}

Parser::symbol_type make_STRING_LITERAL(const string &s,
                                const location& loc) {
    std::string str;

    for(int i=0; s[i] != '\0'; i++) {

        if (s[i] == '\\' && s[i+1] == '\n')
            i++;
        else if (s[i] != '\"' && s[i] != '\\' && s[i] >= 32 && s[i] <= 126)
            str += s[i];
        else {
            static const char hex[] = "0123456789abcdef";
            str += "\\x";
            str += hex[static_cast<unsigned char>(s[i]) >> 4];
            str += hex[static_cast<unsigned char>(s[i]) & 0xf];
        }
    }

    return Parser::make_STRING_LITERAL(str, loc);
}

static void print_error(const position &pos, const string &m) {
    cerr << *(pos.filename) << ":"
         << pos.line << ":"
         << pos.column << ":"
         << " lexical error: "
         << m
         << endl;
}

void Driver::scan_begin() {
    loc.initialize(&source_file);

    if (source_file.empty() || source_file == "-")
        yyin = stdin;
    else if (!(yyin = fopen(source_file.c_str(), "r")))
    {
        cerr << "cannot open " << source_file << ": " << strerror(errno) << '\n';
        exit(EXIT_FAILURE);
    }
}

void Driver::scan_end() {
    fclose(yyin);
}