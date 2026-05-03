#include <iostream>

#include "lexicalAnalzyer.cpp"
#include "syntaxAnalyzer.cpp"
#include "codegen.cpp"

using namespace std;

int main(int argc, char *argv[]){
    // basic usage check
    if (argc != 3){
        cout << "USAGE: ./n2c2 INPUT.c OUTPUT" << endl;
        exit(1);
    }

    FILE * f = fopen(argv[1], "r");
    if (!f){
        cerr << "Failed to open: " << argv[1] << endl;
    }

    string src;
    int ch;
    while ((ch = fgetc(f)) != EOF){
        src += (char)ch;
    }
    fclose(f);

    Lexer l(src);   
    vector<Token> tokens = l.tokenize();  // returns an array of tokens (token struct)
    for (int i = 0; i < tokens.size(); i++){
        cout << tokens[i].tokenType << " " << tokens[i].lexeme << " " << tokens[i].value << endl;
    }


    // syntax analysis on array of tokens
    // .makeTree 
    
    // pass tree into code gen

    return 0;
}