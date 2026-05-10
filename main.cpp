#include <iostream>
#include <fstream>

#include "syntaxAnalyzer.cpp"
#include "codeGenerator.cpp"

using namespace std;

void printChildren(syntaxNode* child, int level){
    for (int i = 0; i < child->children.size(); i++){
        
        for (int j = 0; j < level; j++){
            cout << '\t';
        }
        cout << child->children[i]->syntaxType << " " << child->children[i]->value << endl;

        printChildren(child->children[i], level+1);

    }
}


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

    cout << "\n\n\n-----Syntax-----\n";

    Syntax s(tokens);
    syntaxNode* result = s.parseProgram();    

    printChildren(result, 0);

    CodeGenerator cg;
    string assembly = cg.generate(result);

    ofstream outputFile(argv[2]);
    outputFile << assembly;

    return 0;
}