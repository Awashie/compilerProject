#include <iostream>
#include <vector>
#include <string>
#include <unordered_set>
#include <climits>

using namespace std;

struct Token {
    int value;
    string tokenType;
    string lexeme;
};

unordered_set<string> keywords = {
    "else",
    "if",
    "int",
    "return",
    "while"
};

unordered_set<string> operators = {
    "+",
    "-",
    "*",
    "/",
    "%",
    "=",
    "<",
    ">",
    "<=",
    ">=",
    "==",
    "!=",
    "&&",
    "||",
    "!",
    "&",
    "|",
    "^",
    "~",
    "<<",
    ">>"
};

unordered_set<string> separator = {
    "(",
    ")",
    "{",
    "}",
    ";",
    ":",
};

class Lexer {
private:
    string src; // this is the input source for the code
    int pos; // position in string

    // line and column are used for errors
    int line;
    int column;
    int errorLine;
    int errorColumn;

    
    char peek() { // value of current character
        if (pos < src.length()){
            return src[pos];
        }
        return '\0';
    }

    char consume() { //returns current value then increment pos
        char ch;
        if (pos < src.length()){
            ch = src[pos];
            pos++;
            if (ch == '\n'){
                column = 1;
                line++;
            }
            else{
                column++;
            }
            return ch;
        }
        return '\0';
    }

    char peekNext() { // look at source[pos + 1]
        if (pos + 1 < src.length()) {
            return src[pos + 1];
        }
        return '\0';
    }

    bool eofCheck() {
        if (pos >= src.length()){
            return true;
        }
        return false;
    }

    Token nextToken(){ // calls all check functions and takes the longest match
        errorLine = line;
        errorColumn = line;

        // this checks the end of file
        if (eofCheck()){
            return {0, "EOF", "EOF"};
        }


        // check if any comments or whitespace can be removed in a loop
        // this needs to be looped cause if theres multiple that need to be skipped you need to go through all types repeatedly
        bool skipped = true; 
        while (skipped){
            skipped = false;
            int beforePos = pos;
            skipWhitespace();
            skipSingleLineComment(); 
            if (skipMultiLineComment() == -1){ // multiline comment
                return {-1, "ERROR", "unclosed multiline comment"};
            }
            if (pos != beforePos){
                skipped = true;
            }
            // second check for eof after removing whitespace
            if (eofCheck()){
                return {0, "EOF", "EOF"};
            }
        }

        // keyword and identifier a-z | A-Z | _
        if (isalpha(peek()) || peek() == '_'){
            return checkKeywordAndIdentifier();
        }

        // starts with + | - | 0-9
        if (isdigit(peek()) || (((peek() == '+') || peek() == '-') && isdigit(peekNext()))){
            return checkNumber();
        }

        Token t = checkOperatorAndSeparator();
        if (t.tokenType == ""){
            cerr << "Error: unknown token" << endl;
            return {-1, "ERROR", "unknown token"};
        }
        
        return t;
    } 

    // whitespace will be skipped if the pos starts at whitespace.
    void skipWhitespace(){
        while (isspace(peek())){
            consume();
        }
    }   

    void skipSingleLineComment(){
        if (peek() == '/' && peekNext() == '/'){
            while (peek() != '\n' && peek() != '\0'){
                consume();
            }
        }
    }    
    
    int skipMultiLineComment() {
        if (peek() == '/' && peekNext() == '*'){
            consume();
            consume();
            while (peek() != '\0'){
                if (peek() == '*' && peekNext() == '/'){
                    consume();
                    consume();
                    return 0;
                }
                consume();
            }
            cerr << "Error: unclosed multiline comment" << endl;
            return -1;
        }
        return 0;
    }

    Token checkKeywordAndIdentifier(){ // compares with keywords set and returns longest found match
        string lexeme;
        while (isalnum(peek()) || peek() == '_'){
            lexeme += consume();
        }

        if (keywords.count(lexeme)){
            return {0, "Keyword", lexeme};
        }
        return {0, "Identifier", lexeme};
    }

    Token checkOperatorAndSeparator(){ // compares with operator and separator set and returns longest found match
        string lexeme;
        
        // two characters - do two characters first because this will be longer than 1
        lexeme += peek();
        lexeme += peekNext();

        if (operators.count(lexeme)){
            consume();
            consume();
            return {0, "Operator", lexeme};
        }
        if (separator.count(lexeme)){
            consume();
            consume();
            return {0, "Separator", lexeme};
        }

        // one character
        lexeme = peek();

        if (operators.count(lexeme)){
            consume();
            return {0, "Operator", lexeme};
        }
        if (separator.count(lexeme)){
            consume();
            return {0, "Separator", lexeme};
        }
        return {0, "", ""};
    }

Token checkNumber(){
    string lexeme;

    // optional sign
    if (peek() == '+' || peek() == '-'){
        lexeme += consume();
    }

    int sign = (!lexeme.empty() && lexeme[0] == '-') ? -1 : 1;

    // hex
    if (peek() == '0' && (peekNext() == 'x' || peekNext() == 'X')){
        lexeme += consume(); // 0
        lexeme += consume(); // x
        if (!isxdigit(peek())){
            cerr << "Error: expected digits after 0x" << endl;
            return {-1, "ERROR", lexeme};
        }
        while (isxdigit(peek())){
            lexeme += consume();
        }
        try {
            return {stoi(lexeme, nullptr, 16), "Number", lexeme};
        } catch (out_of_range&) {
            cerr << "Error: number out of range: " << lexeme << endl;
            return {-1, "ERROR", lexeme};
        }
    }

    // binary
    if (peek() == '0' && (peekNext() == 'b' || peekNext() == 'B')){
        lexeme += consume(); // 0
        lexeme += consume(); // b
        if (peek() != '0' && peek() != '1'){
            cerr << "Error: expected digits after 0b" << endl;
            return {-1, "ERROR", lexeme};
        }
        while (peek() == '0' || peek() == '1'){
            lexeme += consume();
        }
        try {
            int val = sign * stoi(lexeme.substr(lexeme.find_first_of("bB") + 1), nullptr, 2);
            return {val, "Number", lexeme};
        } catch (out_of_range&) {
            cerr << "Error: number out of range: " << lexeme << endl;
            return {-1, "ERROR", lexeme};
        }
    }

    // decimal
    int baseStart = lexeme.length();
    while (isdigit(peek())){
        lexeme += consume();
    }
    string baseLexeme = lexeme.substr(baseStart);

    if (baseLexeme.empty()){
        cerr << "Error: expected digits" << endl;
        return {-1, "ERROR", lexeme};
    }

    // optional exponent
    if (peek() == 'e' || peek() == 'E'){
        lexeme += consume();
        if (!isdigit(peek())){
            cerr << "Error: expected digits after exponent" << endl;
            return {-1, "ERROR", lexeme};
        }
        string expLexeme;
        while (isdigit(peek())){
            char c = consume();
            expLexeme += c;
            lexeme += c;
        }
        try {
            int base = stoi(baseLexeme, nullptr, 10);
            int exp = stoi(expLexeme, nullptr, 10);
            int result = base;
            for (int i = 0; i < exp; i++){
                if (result > INT_MAX / 10){
                    cerr << "Error: number out of range: " << lexeme << endl;
                    return {-1, "ERROR", lexeme};
                }
                result *= 10;
            }
            return {result, "Number", lexeme};
        } catch (out_of_range&) {
            cerr << "Error: number out of range: " << lexeme << endl;
            return {-1, "ERROR", lexeme};
        }
    }

    try {
        return {stoi(lexeme, nullptr, 10), "Number", lexeme};
    } catch (out_of_range&) {
        cerr << "Error: number out of range: " << lexeme << endl;
        return {-1, "ERROR", lexeme};
    }
}

public:
    Lexer(string input) : src(input), pos(0), line(1), column(1) {} 

    vector<Token> tokenize(){
        vector<Token> result;
        Token t = nextToken();
        while (t.tokenType != "EOF"){
            if (t.tokenType == "ERROR"){
                result.clear();
                cerr << "Tokenizer stopped due to error at:\nLine: " << errorLine << ", Column: " << errorColumn << endl;
                return result;
            }
            result.push_back(t);
            t = nextToken();
        }

        return result;
    }
};