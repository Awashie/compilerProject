#include <iostream>

#include "lexicalAnalyzer.cpp"
#include "syntaxAnalyzer.cpp"
#include "codegen.cpp"

using namespace std;

int main(int argc, char *argv[]){
    // basic usage check
    if (argc != 3){
        cout << "USAGE: ./n2c2 INPUT.c OUTPUT" << endl;
        exit(1);
    }

    // open file

    // tokenize with lexer
    // returns an array of tokens (token struct)

    // syntax analysis on array of tokens
    // .makeTree 
    
    // pass tree into code gen

    return 0;
}