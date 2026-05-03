#include "lexicalAnalyzer.cpp"

using namespace std;

struct syntaxNode{
    string syntaxType;
    string value;
    vector<syntaxNode*> children;
};

class Syntax {
private:
    vector<Token> tokens;
    int pos;

    Syntax (vector<Token> input) : tokens(input), pos(0) {}

    Token& currToken() { // returns current token
        return tokens[pos];
    }

    bool isAtEnd(){ // returns true if at end of file
        return pos >= tokens.size() || currToken().tokenType == "EOF";
    }

    bool check(string type) { // true if current token matches the given type

    }

    bool check(string type, string lexeme){ // true if current token matches the given type and lexeme

    }

    bool expect(string type) { // consume and return true if type match current token. otherwise throw error

    }

    bool expect(string type, string lexeme) { // consume and return true if type and lexeme match current token. otherwise throw error
        
    }

    bool match(string type){ // consume and return true if type match current token

    }

    bool match(string type, string lexeme){ // consume and return true if type and lexeme match current token

    }

    Token consume(){
        return tokens[pos++];
    }

    Token& peekNext(){
        int next = pos + 1;
        if (next >= tokens.size()){
            return tokens.back();
        }
        return tokens[next];
    }


public:
    syntaxNode* parseProgram() {
        syntaxNode* result = new syntaxNode;
        result->syntaxType = "Program";
        while (!isAtEnd()){
            result->children.push_back(parseDeclaration());
        }
        return result;
    }

    syntaxNode* parseDeclaration() {
        if (check("Keyword", "int")){
            return parseValDecl();
        }
        return parseStatement();
    }

    syntaxNode* parseValDecl(){
        expect("Keyword", "int");
        Token name = expect("Identifier");
        syntaxNode* result = new syntaxNode;
        result->syntaxType = "VarDecl";
        result->value = name.lexeme;
        if (match("Operator", "=")){
            result->children.push_back(parseExpr());
        }
        expect("Separator", ";");
        return result;
    }

    syntaxNode* parseStatement() {
        if (check("Keyword", "if")){
            return parseIf();
        }
        if (check("Keyword", "while")){
            return parseWhile();
        }
        if (check("Keyword", "return")){
            return parseReturn();
        }
        if (check("Separator", "{")){
            return parseSeparator();
        }
        return parseExprStmt();
    }

    syntaxNode* parseIf(){
        expect("Keyword", "if");
        expect("Separator", "(");
        syntaxNode* result = new syntaxNode;
        result->syntaxType = "IfStmt";
        result->children.push_back(parseExpr());
        expect("Separator", ")");
        result->children.push_back(parseBlock());
        if (match("Keyword", "else")){
            result->children.push_back(parseBlock());
        }
        return result;
    }

    syntaxNode* parseWhile(){
        expect("Keyword", "while");
        expect("Separator", "(");
        syntaxNode* result = new syntaxNode;
        result->syntaxType = "WhileStmt";
        result->children.push_back(parseExpr());
        expect("Separator", ")");
        result->children.push_back(parseBlock());
        return result;
    }

    syntaxNode* parseReturn(){
        expect("Keyword", "return");
        syntaxNode* result = new syntaxNode;
        result->syntaxType = "ReturnStmt";
        result->children.push_back(parseExpr());
        expect("Separator", ";");
        return result;
    }

    syntaxNode* parseBlock(){
        expect("Separator", "{");
        syntaxNode* result = new syntaxNode;
        while (!check("Separator", ))
    }


};
