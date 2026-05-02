#include <iostream>
#include <unordered_set>
#include <fstream>
#include <string>

using namespace std;

struct Token {
    string type;
    string lexeme;
};

class Lexer {
private:
    string source;
    int pos = 0;

    unordered_set<string> keywords = {
        "else",
        "for",
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

public:
    Lexer(const string& input) : source(input) {}

    char peek() {
        if (pos < source.length()){
            return source[pos];
        }
        return '\0';
    }

    char consume() { // returns peek then advance to next position
        char ch;
        if (pos < source.length()){
            ch = source[pos];
            pos++;
            return ch;
        }
        return '\0';
    }

    char peekNext() { // look at source[pos + 1]
        if (pos + 1 < source.size()) {
            return source[pos + 1];
        }
        return '\0';
    }

    void skipWhitespace() { // go to next source[pos] that is not a whitespace character
        while (isspace(peek())){
            consume();
        }
    }

    Token scanFloat() {
        int startPos = pos;
        int state = 0;

        while (true){
            char ch = peek();
            if (state == 0){
             if (ch == '+' || ch == '-') {
                    consume();
                    state = 1;
                }
                else if (isdigit(ch)) {
                    consume();
                    state = 2;
                }
                else {
                    state = -1;
                    break;
                }
            }
            else if (state == 1) { // sign seen
                if (isdigit(ch)) {
                    consume();
                    state = 2;
                }
                else {
                    state = -1;
                    break;
                }
            }
            else if (state == 2) { // integer part
                if (isdigit(ch)) {
                    consume();
                }
                else if (ch == '.') {
                    consume();
                    state = 3;
                }
                else if (ch == 'E' || ch == 'e') {
                    consume();
                    state = 4;
                }
                else {
                    break; // integer finished
                }
            }
            else if (state == 3) { // decimal part
                if (isdigit(ch)) {
                    consume();
                }
                else if (ch == 'E' || ch == 'e') {
                    consume();
                    state = 4;
                }
                else {
                    break; // decimal finished
                }
            }
            else if (state == 4) { // exponent marker
                if (ch == '+' || ch == '-') {
                    consume();
                    state = 5;
                }
                else if (isdigit(ch)) {
                    consume();
                    state = 6;
                }
                else {
                    state = -1;
                    break;
                }
            }
            else if (state == 5) { // exponent sign
                if (isdigit(ch)) {
                    consume();
                    state = 6;
                }
                else {
                    state = -1;
                    break;
                }
            }
            else if (state == 6) { // exponent digits
                if (isdigit(ch)) {
                    consume();
                }
                else {
                    break; // exponent finished
                }
            }
            else { // invalid state
                state = -1;
                break;
            }
        }

        if (state == 2 || state == 3 || state == 6){
            string lexeme = source.substr(startPos, pos - startPos);
            return {"Float", lexeme};
        }
        else { // invalid state
            pos = startPos;
            return {"",""};
        } 
    }

    Token nextToken() { // returns next token
        skipWhitespace();

        if (peek() == '\0') return {"EOF", ""};

        string lexeme;

        // identifiers and keywords
        if (isalpha(peek()) || peek() == '_'){
            while (isalnum(peek()) || peek() == '_'){
                lexeme += consume();
            }

            if (keywords.count(lexeme) == 1){
                return {"Keyword", lexeme};
            }
            return {"Identifier", lexeme};
        }

        if (isdigit(peek()) || peek() == '+' || peek() == '-'){
            Token num = scanFloat();
            if (num.type != ""){
                return num;
            }
        } 

        char first = peek();
        char second = peekNext();

        // check if two character operator or separators exist
        string twoChar;
        twoChar += first;
        twoChar += second;

        if (operators.count(twoChar) == 1){
            consume();
            consume();
            return {"Operator", twoChar};
        }
        if (separator.count(twoChar) == 1){
            consume();
            consume();
            return {"Separator", twoChar};
        }

        lexeme += consume();

        if (operators.count(lexeme) == 1){
            return {"Operator", lexeme};
        }
        if (separator.count(lexeme) == 1){
            return {"Separator", lexeme};
        }

        return {"Unknown", lexeme};
    }
};

string readFile(string fileLocation){ // loads all file contents into a string
    string result;

    ifstream file;
    file.open(fileLocation);

    if (!file){
        cout << fileLocation << " could not be opened.\n";
        exit (1);
    }

    string line;
    while (getline(file, line)) {
        result += line + '\n';
    }

    return result;
}
/*
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "USAGE: ./lexer source_file\n";
        return 1;
    }

    string code = readFile(argv[1]);

    

    Lexer lexer(code);
    Token t = {"", ""};

    while (t.type != "EOF"){
        t = lexer.nextToken();
        cout << t.type << ": " << t.lexeme << '\n';
    }

    return 0;
}
    */