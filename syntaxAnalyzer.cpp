#pragma once
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

    Token& currToken() { // returns current token
        return tokens[pos];
    }

    bool isAtEnd(){ // returns true if at end of file
        return pos >= tokens.size() || currToken().tokenType == "EOF";
    }

    bool check(string type) { // true if current token matches the given type
        if(isAtEnd()){
            return false;
        }
        return currToken().tokenType == type;
    }

    bool check(string type, string lexeme){ // true if current token matches the given type and lexeme
        if (isAtEnd()) return false;
        return currToken().tokenType == type && currToken().lexeme == lexeme;
    }

    Token expect(string type) { // consume and return token if type match current token. otherwise throw error
        if (check(type)) {
            return tokens[pos++];
        }
        throw runtime_error("Expected " + type + " but got " + currToken().tokenType);
    }

    Token expect(string type, string lexeme) { // consume and return token if type and lexeme match current token. otherwise throw error
        if (check(type, lexeme)){
            return tokens[pos++];
        }
        throw runtime_error("Expected " + type + ", " + lexeme + "but got " + currToken().tokenType + ", " + currToken().lexeme);
    }

    bool match(string type){ // consume and return true if type match current token
        if (check(type)){
            pos++;
            return true;
        }
        return false;
    }

    bool match(string type, string lexeme){ // consume and return true if type and lexeme match current token
        if (check(type, lexeme)){
            pos++;
            return true;
        }
        return false;
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

    Syntax (vector<Token> input) : tokens(input), pos(0) {}


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
            return parseBlock();
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
        result->syntaxType = "Block";
        while (!check("Separator", "}") && !isAtEnd()){
            result->children.push_back(parseDeclaration());
        }
        expect("Separator", "}");
        return result;
    }

    syntaxNode* parseExprStmt(){
        syntaxNode* result = new syntaxNode;
        result->syntaxType = "ExprStmt";
        result->children.push_back(parseExpr());
        expect("Separator", ";");
        return result;
    }

    syntaxNode* parseExpr(){
        return parseAssignment();
    }

    syntaxNode* parseAssignment(){
        if (check("Identifier") && peekNext().tokenType == "Operator" && peekNext().lexeme == "="){
            Token name = consume();
            consume();
            syntaxNode* result = new syntaxNode;
            result->syntaxType = "Assignment";
            result->value = "=";
            syntaxNode* identifierChild = new syntaxNode;
            identifierChild->syntaxType = "Identifier";
            identifierChild->value = name.lexeme;
            result->children.push_back(identifierChild);
            result->children.push_back(parseAssignment());
            return result;
        }
        return parseLogicalOr();
    }

    syntaxNode* parseLogicalOr(){
        syntaxNode* left = parseLogicalAnd();
        while (check("Operator", "||")){
            consume();
            syntaxNode* result = new syntaxNode;
            result->syntaxType = "BinaryExpr";
            result->value = "||";
            result->children.push_back(left);
            result->children.push_back(parseLogicalAnd());
            left = result;
        }
        return left;
    }

    syntaxNode* parseLogicalAnd() {
        syntaxNode* left = parseBitwiseOr();
        while (check("Operator", "&&")) {
            consume();
            syntaxNode* result = new syntaxNode;
            result->syntaxType = "BinaryExpr";
            result->value = "&&";
            result->children.push_back(left);
            result->children.push_back(parseBitwiseOr());
            left = result;
        }
        return left;
    }

    syntaxNode* parseBitwiseOr() {
        syntaxNode* left = parseBitwiseXor();
        while (check("Operator", "|")) {
            consume();
            syntaxNode* result = new syntaxNode;
            result->syntaxType = "BinaryExpr";
            result->value = "|";
            result->children.push_back(left);
            result->children.push_back(parseBitwiseXor());
            left = result;
        }
        return left;
    }

    syntaxNode* parseBitwiseXor() {
        syntaxNode* left = parseBitwiseAnd();
        while (check("Operator", "^")) {
            consume();
            syntaxNode* result = new syntaxNode;
            result->syntaxType = "BinaryExpr";
            result->value = "^";
            result->children.push_back(left);
            result->children.push_back(parseBitwiseAnd());
            left = result;
        }
        return left;
    }

    syntaxNode* parseBitwiseAnd() {
        syntaxNode* left = parseEquality();
        while (check("Operator", "&")) {
            consume();
            syntaxNode* result = new syntaxNode;
            result->syntaxType = "BinaryExpr";
            result->value = "&";
            result->children.push_back(left);
            result->children.push_back(parseEquality());
            left = result;
        }
        return left;
    }

    syntaxNode* parseEquality() {
        syntaxNode* left = parseComparison();
        while (check("Operator", "==") || check("Operator", "!=")) {
            string op = consume().lexeme;
            syntaxNode* result = new syntaxNode;
            result->syntaxType = "BinaryExpr";
            result->value = op;
            result->children.push_back(left);
            result->children.push_back(parseComparison());
            left = result;
        }
        return left;
    }

syntaxNode* parseComparison() {
        syntaxNode* left = parseShift();
        while (check("Operator", "<")  || check("Operator", ">") ||
               check("Operator", "<=") || check("Operator", ">=")) {
            string op = consume().lexeme;
            syntaxNode* result = new syntaxNode;
            result->syntaxType = "BinaryExpr";
            result->value = op;
            result->children.push_back(left);
            result->children.push_back(parseShift());
            left = result;
        }
        return left;
    }

    syntaxNode* parseShift() {
        syntaxNode* left = parseAddition();
        while (check("Operator", "<<") || check("Operator", ">>")) {
            string op = consume().lexeme;
            syntaxNode* result = new syntaxNode;
            result->syntaxType = "BinaryExpr";
            result->value = op;
            result->children.push_back(left);
            result->children.push_back(parseAddition());
            left = result;
        }
        return left;
    }

syntaxNode* parseAddition() {
        syntaxNode* left = parseMultiplication();
        while (check("Operator", "+") || check("Operator", "-")) {
            string op = consume().lexeme;
            syntaxNode* result = new syntaxNode;
            result->syntaxType = "BinaryExpr";
            result->value = op;
            result->children.push_back(left);
            result->children.push_back(parseMultiplication());
            left = result;
        }
        return left;
    }

    syntaxNode* parseMultiplication() {
        syntaxNode* left = parseUnary();
        while (check("Operator", "*") || check("Operator", "/") || check("Operator", "%")) {
            string op = consume().lexeme;
            syntaxNode* result = new syntaxNode;
            result->syntaxType = "BinaryExpr";
            result->value = op;
            result->children.push_back(left);
            result->children.push_back(parseUnary());
            left = result;
        }
        return left;
    }

    syntaxNode* parseUnary() {
        if (check("Operator", "!") || check("Operator", "~") ||
            check("Operator", "-") || check("Operator", "+")) {
            string op = consume().lexeme;
            syntaxNode* result = new syntaxNode;
            result->syntaxType = "UnaryExpr";
            result->value = op;
            result->children.push_back(parseUnary());
            return result;
        }
        return parsePrimary();
    }

    syntaxNode* parsePrimary(){
        if(check("Number")){
            syntaxNode* result = new syntaxNode;
            result->syntaxType = "NumberLiteral";
            result->value = consume().lexeme;
            return result;
        }
        if(check("Identifier")){
            syntaxNode* result = new syntaxNode;
            result->syntaxType = "Identifier";
            result->value = consume().lexeme;
            return result;
        }
        if (check("Separator", "(")){
            consume();
            syntaxNode* result = parseExpr();
            expect("Separator", ")");
            return result;
        }
        throw runtime_error("Unexpected token in expression: " + currToken().lexeme);
    }
};
