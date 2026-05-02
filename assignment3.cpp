#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <unordered_map>
using namespace std;

// Tokens
enum TokenType {
    TOKEN_NUMBER, TOKEN_IDENT, 
    TOKEN_ASSIGN, TOKEN_PLUS_ASSIGN, 
    TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH,
    TOKEN_XOR, TOKEN_AND, TOKEN_OR,
    TOKEN_SHR, TOKEN_SHL,
    TOKEN_INC, TOKEN_DEC,
    TOKEN_QUESTION, TOKEN_COLON,
    TOKEN_LPAREN, TOKEN_RPAREN,
    TOKEN_SEMI, TOKEN_EOF
};

struct Token {
    TokenType type;
    string text;
    int value = 0;
};

vector<Token> tokens;
int pos = 0;

void add_token(TokenType t, string str, int val=0){
    tokens.push_back({t, str, val});
}

// Lexer
void lex(string s) {
    for (int i = 0; i < s.size(); i++){
        if (isspace(s[i])){ // skip whitespace
            continue;
        }

        if (isalpha(s[i]) || s[i] == '_'){ // identifier
            string str;
            while (i < s.size() && (isalnum(s[i]) || s[i] == '_')){
                str += s[i];
                i++;
            }
            add_token(TOKEN_IDENT, str);
            i--;
            continue;
        }

        if (isdigit(s[i])){
            string str;
            if (s[i] == '0' && i + 1 < s.size() && (s[i + 1] == 'x' || s[i + 1] == 'X')){ // implement hex inputs
                str += s[i++]; 
                str += s[i++];
                while (i < s.size() && isxdigit(s[i])){
                    str += s[i++];
                }
                add_token(TOKEN_NUMBER, str, stoi(str, nullptr, 16));
            }
            else {
                while (i < s.size() && isdigit(s[i])){
                    str += s[i++];
                }
                add_token(TOKEN_NUMBER, str, stoi(str));
            } 
            i--;
            continue;
        }

        if (i + 1 < s.size()){
            string two = s.substr(i, 2);
            if(two=="+="){
                add_token(TOKEN_PLUS_ASSIGN,two);
                i+=1; 
                continue;
            }
            if(two=="++"){add_token(TOKEN_INC,two);
                i+=1; continue;}
            if(two=="--"){add_token(TOKEN_DEC,two);
                i+=1; 
                continue;}
            if(two==">>"){
                add_token(TOKEN_SHR,two); 
                i+=1; 
                continue;}
            if(two=="<<"){
                add_token(TOKEN_SHL,two);
                i+=1;
                continue;
            }
        }

        switch(s[i]){
            case '=': add_token(TOKEN_ASSIGN,"="); break;
            case '+': add_token(TOKEN_PLUS,"+"); break;
            case '-': add_token(TOKEN_MINUS,"-"); break;
            case '*': add_token(TOKEN_STAR,"*"); break;
            case '/': add_token(TOKEN_SLASH,"/"); break;
            case '^': add_token(TOKEN_XOR,"^"); break;
            case '&': add_token(TOKEN_AND,"&"); break;
            case '|': add_token(TOKEN_OR,"|"); break;
            case '?': add_token(TOKEN_QUESTION,"?"); break;
            case ':': add_token(TOKEN_COLON,":"); break;
            case '(': add_token(TOKEN_LPAREN,"("); break;
            case ')': add_token(TOKEN_RPAREN,")"); break;
            case ';': add_token(TOKEN_SEMI,";"); break;
            default: cerr <<"lexer unknown token" << s[i]; exit(1);
        }
    }
    add_token(TOKEN_EOF, "EOF");
}

// syntax tree
enum SyntaxType {S_ASSIGN, S_BINARY, S_UNARY, S_POSTFIX, S_TERNARY, S_IDENT, S_NUMBER, S_DEREF};

struct SyntaxNode { 
    SyntaxType type;
    string op;
    string name;
    int value = 0;
    SyntaxNode *l = nullptr, *r = nullptr, *c = nullptr;
};

Token& peek(){ // returns current token
    return tokens[pos];
}

Token advance(){ // returns the next token
    return tokens[pos++];
}

bool check(TokenType t){ // 
    return peek().type==t;
}

void expect(TokenType t){ // if the check is good then advance, otherwise error
    if (!check(t)) {
        cerr << "syntax error" << t << endl;
        exit(1);
    }
    else{
        advance();
    }
}

SyntaxNode* expr(); // lowest precedence 
SyntaxNode* assign();
SyntaxNode* ternary();
SyntaxNode* bxor(); 
SyntaxNode* bor();
SyntaxNode* band();
SyntaxNode* shift();
SyntaxNode* add();
SyntaxNode* mult();
SyntaxNode* unary();
SyntaxNode* postfix();
SyntaxNode* parentheses(); // highest precedence 

SyntaxNode* sn(SyntaxType t){ // new node
    return new SyntaxNode{t};
}

SyntaxNode* expr() {
    return assign();
}

SyntaxNode* assign() {
    SyntaxNode* l = ternary();
    if (check(TOKEN_ASSIGN) || check(TOKEN_PLUS_ASSIGN)) {
        string op = advance().text;
        SyntaxNode* r = assign();
        SyntaxNode* x = sn(S_ASSIGN);
        x->op = op;
        x->l = l;
        x->r = r;
        return x;
    }
    return l;
}

SyntaxNode* ternary() {
    SyntaxNode* c = bxor();
    if (check(TOKEN_QUESTION)){
        advance(); 
        SyntaxNode* t = expr();
        expect(TOKEN_COLON);
        SyntaxNode* e = ternary();
        SyntaxNode* x = sn(S_TERNARY); 
        x->c = c;
        x->l = t;
        x->r = e;
        return x;
    }
    return c;
}

SyntaxNode* bxor() {
    SyntaxNode* l = bor();
    while (check(TOKEN_XOR)){
        SyntaxNode* x = sn(S_BINARY);
        x->op = advance().text;
        x->l = l;
        x->r = bor();
        l = x;
    }
    return l;
}

SyntaxNode* bor(){
    SyntaxNode* l = band();
    while (check(TOKEN_OR)){
        SyntaxNode* x = sn(S_BINARY);
        x->op = advance().text;
        x->l = l;
        x->r = band();
        l = x;
    }
    return l;
}

SyntaxNode* band(){
    SyntaxNode* l = shift();
    while (check(TOKEN_AND)){
        SyntaxNode* x = sn(S_BINARY);
        x->op = advance().text;
        x->l = l;
        x->r = shift();
        l = x;
    }
    return l;
}

SyntaxNode* shift() {
    SyntaxNode* l = add();
    while (check(TOKEN_SHR) || check(TOKEN_SHL)) {
        SyntaxNode* x = sn(S_BINARY);
        x->op = advance().text;
        x->l = l;
        x->r = add();
        l = x;
    }
    return l;
}

SyntaxNode* add() {
    SyntaxNode* l = mult();
    while (check(TOKEN_PLUS) || check(TOKEN_MINUS)) {
        SyntaxNode* x = sn(S_BINARY);
        x->op = advance().text;
        x->l = l;
        x->r = mult();
        l = x;
    }
    return l;
}

SyntaxNode* mult() {
    SyntaxNode* l = unary();
    while (check(TOKEN_STAR) || check(TOKEN_SLASH)) {
        SyntaxNode* x = sn(S_BINARY);
        x->op = advance().text;
        x->l = l;
        x->r = unary();
        l = x;
    }
    return l;
}

SyntaxNode* unary() {
    if (check(TOKEN_MINUS)){
        SyntaxNode* x = sn(S_UNARY);
        x->op = "-";
        advance();
        x->l = unary();
        return x;
    }
    if (check(TOKEN_STAR)){
        advance();
        SyntaxNode* x = sn(S_DEREF);
        x->l = unary();
        return x;
    }
    return postfix();
}

SyntaxNode* postfix() {
    SyntaxNode* p = parentheses();
    if (check(TOKEN_INC)){
        SyntaxNode* x = sn(S_POSTFIX);
        x->op = "++";
        x->l = p;
        advance();
        return x;
    }
    if (check(TOKEN_DEC)){
        SyntaxNode* x = sn(S_POSTFIX);
        x->op = "--";
        x->l = p;
        advance();
        return x;
    }
    return p;
}

SyntaxNode* parentheses() {
    if (check(TOKEN_NUMBER)){
        SyntaxNode* x = sn(S_NUMBER);
        x->value = advance().value;
        return x;
    }
    if (check(TOKEN_IDENT)){
        SyntaxNode* x = sn(S_IDENT);
        x->name = advance().text;
        return x;
    }
    if (check(TOKEN_LPAREN)){
        advance(); 
        SyntaxNode* e = expr();
        expect(TOKEN_RPAREN);
        return e;
    }
    cerr << "parse error at: " << peek().text << endl;
    exit(1);
}

// semantic 
void semantic_check(SyntaxNode* n){
    if (n == nullptr){
        return;
    }
    if (n->type == S_POSTFIX && n->l->type == S_NUMBER) {
        cerr << "error: can't increment or decrement fixed number." << endl;
        exit(1);
    }
    semantic_check(n->l);
    semantic_check(n->r);
    semantic_check(n->c);
}

// intermediate generation
struct Intermediate {
    string operation;
    string operand1;
    string operand2;
    string result;
};

vector<Intermediate> intermediateBuffer; // stores the print outs without the tmp and label numbers

int tmpVarCounter = 1;
int labelCounter = 1;

string tmp() {
    return "T" + to_string(tmpVarCounter++);
};

string label() {
    return "L" + to_string(labelCounter++);
}

void print(string operation, string operand1, string operand2, string result){
    intermediateBuffer.push_back({operation, operand1, operand2, result});
}

string generate_intermediate(SyntaxNode* n) {
    if (n == nullptr){
        return "";
    }
    if (n->type == S_NUMBER){
        return to_string(n->value);
    }
    if (n->type == S_IDENT){
        return n->name;
    }
    if (n->type == S_BINARY){
        string l = generate_intermediate(n->l);
        string r = generate_intermediate(n->r);
        string t = tmp();
        print(n->op, l, r, t);
        return t;
    }
    if (n->type == S_ASSIGN){
        string r = generate_intermediate(n->r);
        string l = n->l->name;
        if (n->op == "+=") {
            string t = tmp();
            print("+", l, r, t);
            print("MOV", t, "", l);
        }
        else {
            print("MOV", r, "", l);
        } 
        return l;
    }
    if (n->type == S_TERNARY){
        string c = generate_intermediate(n->c);
        string false_label = label();
        string end_label = label();
        string result = tmp();
    
        print("JZ", c, "", false_label);

        // true will generate intermediate only if left is not equal to 0 
        string true_val = generate_intermediate(n->l);
        print("MOV", true_val, "", result);
        print("JMP", end_label, "", "");
    
        // false will generate intermediate only if right is not equal to 0
        print("LABEL", false_label, "", "");
        string false_val = generate_intermediate(n->r);
        print("MOV", false_val, "", result);

        print("LABEL", end_label, "", "");
        return result;
    }
    if (n->type == S_DEREF) {
        string p = generate_intermediate(n->l);
        string t = tmp(); 
        print("DEREF", p, "", t);
        return t;
    }
    if (n->type == S_POSTFIX){
        string v = generate_intermediate(n->l);
        string original_val = tmp();
        print("MOV", v, "", original_val);

        string new_val = tmp();
        print(n->op == "++" ? "+" : "-", v, "1", new_val); 
        print("MOV", new_val, "", v); 
        return original_val;
    }
    if (n->type == S_UNARY) { 
        string v = generate_intermediate(n->l),
        t = tmp();
        print("NEG", v, "", t);
        return t;
    }
    return "";
}

// code generator

unordered_map<string, string> regs;

int rc = 0;
bool is_literal(string v) { // check if first character is - or digit
    return !v.empty() && (isdigit(v[0]) || v[0] == '-');
}
bool is_temporary(string v) { // check if first character is T
    return !v.empty() && v[0] == 'T';
}

string get_loc(string v) {
    if (is_literal(v)) {
        return "#" + v;
    }
    if (is_temporary(v)) {
        if (!regs.count(v)){
            regs[v] = "R" + to_string(rc++);
        }
        return regs[v];
    }
    return "[" + v + "]";
}

void code_gen(const vector<Intermediate>& code){
    for (int i = 0; i < code.size(); i++){
        if (code[i].operation == "LABEL"){
            cout << code[i].operand1 << ":" << endl;
            continue;
        }

        if (code[i].operation == "JMP"){
            cout << "\tJMP " << code[i].operand1 << endl;
            continue;
        }

        if (code[i].operation == "JZ"){
            cout << "\tCMP " << get_loc(code[i].operand1) << ", #0" << endl;
            cout << "\tJZ " << code[i].result << endl;
            continue; 
        }

        string destination_register = get_loc(code[i].result);

        if (code[i].operation == "MOV"){
            cout << "\tMOV " << destination_register << ", " << get_loc(code[i].operand1) << endl;
        }

        else if (code[i].operation == "DEREF") {
            cout << "\tLOAD " << destination_register << ", [" << get_loc(code[i].operand1) << "]" << endl;
        }

        else if (code[i].operation == "NEG"){
            cout << "\tNEG " << destination_register << ", " << get_loc(code[i].operand1) << endl;
        }

        else { 
            string operation = code[i].operation;
            string instruction = "";
            if (operation == "+"){
                instruction = "ADD";
            }
            else if (operation == "-"){
                instruction = "SUB";
            }
            else if (operation == "*"){
                instruction = "MUL";
            }
            else if (operation == "/"){
                instruction = "DIV";
            }
            else if (operation == "&"){
                instruction = "AND";
            }
            else if (operation == "^"){
                instruction = "XOR";
            }
            else if (operation == "|"){
                instruction = "OR";
            }
            else if (operation == ">>"){
                instruction = "SHR";
            }
            else if (operation == "<<"){
                instruction = "SHL";
            }

            cout << "\t" << instruction << " " << destination_register << ", " << get_loc(code[i].operand1);
            if (!code[i].operand2.empty()){
                cout << ", " << get_loc(code[i].operand2);
            }
            cout << endl;
        }

        if (!is_temporary(code[i].result) && !code[i].result.empty() && code[i].operation != "LABEL"){
            if (destination_register.empty() || destination_register[0] != '[') {
                cout << "\tSTORE [" << code[i].result << "], " << destination_register << endl;
            }
        }
    }
}

int main() {
    string source_code = "result = (x += y * (z >> 2)) ^ (flag ? *p++ : -q) & 0xFF;";
    cout << "SOURCE CODE: " << source_code << "\n";

    lex(source_code);

    SyntaxNode* root = expr(); 
    
    vector<SyntaxNode*> statements;
    statements.push_back(root);
    while (check(TOKEN_SEMI)) {
        advance();
        if (check(TOKEN_EOF)) break;
        statements.push_back(expr());
    }

    for (int i = 0; i < statements.size(); i++) {
        semantic_check(statements[i]);
    }

    for (int i = 0; i < statements.size(); i++) {
        generate_intermediate(statements[i]);
    }

    code_gen(intermediateBuffer);

    return 0;
}