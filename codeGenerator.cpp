#include "syntaxAnalyzer.cpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <stdexcept>

using namespace std;

struct Symbol { string name, label; };

class CodeGenerator {
private:
    ostringstream dataSec, textSec;
    unordered_map<string, Symbol> symbolTable;
    int labelCounter = 0;

    void emit(const string& s)        { textSec << "    " << s << "\n"; }
    void emitLabel(const string& s)   { textSec << "\n" << s << ":\n"; }
    void emitBlank()                  { textSec << "\n"; }

    static string varRef(const string& label) { return "DWORD PTR [" + label + "]"; }

    string lookupVar(const string& name) {
        if (!symbolTable.count(name)) throw runtime_error("Undeclared variable: " + name);
        return symbolTable.at(name).label;
    }

    void genNode(syntaxNode* node) {
        if (!node) throw runtime_error("Null AST node");
        const string& t = node->syntaxType;
        if      (t == "Program")       genProgram(node);
        else if (t == "VarDecl")       genVarDecl(node);
        else if (t == "Block")         genBlock(node);
        else if (t == "IfStmt")        genIf(node);
        else if (t == "WhileStmt")     genWhile(node);
        else if (t == "ReturnStmt")    genReturn(node);
        else if (t == "ExprStmt")      { genNode(node->children[0]); emitBlank(); }
        else if (t == "Assignment")    genAssignment(node);
        else if (t == "BinaryExpr")    genBinaryExpr(node);
        else if (t == "UnaryExpr")     genUnaryExpr(node);
        else if (t == "NumberLiteral") emit("mov     eax, " + node->value);
        else if (t == "Identifier")    emit("mov     eax, " + varRef(lookupVar(node->value)));
        else throw runtime_error("Unknown node type: " + t);
    }

    void genProgram(syntaxNode* node) {
        textSec << ".text\n.globl  main\n.type   main, @function\n\nmain:\n";
        emit("push    rbp");
        emit("mov     rbp, rsp");
        emitBlank();
        for (syntaxNode* child : node->children) genNode(child);
        emit("mov     eax, 0");
        emit("pop     rbp");
        emit("ret");
        textSec << "\n.size   main, .-main\n";
    }

    void genVarDecl(syntaxNode* node) {
        const string& name = node->value;
        if (symbolTable.count(name)) throw runtime_error("Variable already declared: " + name);
        string label = "var_" + name;
        symbolTable[name] = {name, label};
        dataSec << "    " << label << ":    dd  0\n";
        if (!node->children.empty()) {
            genNode(node->children[0]);
            emit("mov     " + varRef(label) + ", eax");
        }
        emitBlank();
    }

    void genBlock(syntaxNode* node) {
        for (syntaxNode* child : node->children) genNode(child);
    }

    void genIf(syntaxNode* node) {
        string n          = to_string(labelCounter++);
        string trueLabel  = "if" + n + "_true";
        string falseLabel = "if" + n + "_false";
        string endLabel   = "if" + n + "_end";

        genNode(node->children[0]);
        emit("cmp     eax, 0");
        emit("jne     " + trueLabel);
        emit("jmp     " + falseLabel);

        emitLabel(trueLabel);
        genNode(node->children[1]);
        emit("jmp     " + endLabel);

        emitLabel(falseLabel);
        if (node->children.size() >= 3) genNode(node->children[2]);

        emitLabel(endLabel);
        emitBlank();
    }

    void genWhile(syntaxNode* node) {
        string n        = to_string(labelCounter++);
        string loopTop  = "while" + n + "_top";
        string loopBody = "while" + n + "_body";
        string loopEnd  = "while" + n + "_end";

        emitLabel(loopTop);
        genNode(node->children[0]);
        emit("cmp     eax, 0");
        emit("jne     " + loopBody);
        emit("jmp     " + loopEnd);

        emitLabel(loopBody);
        genNode(node->children[1]);
        emit("jmp     " + loopTop);

        emitLabel(loopEnd);
        emitBlank();
    }

    void genReturn(syntaxNode* node) {
        genNode(node->children[0]);
        emit("pop     rbp");
        emit("ret");
        emitBlank();
    }

    void genAssignment(syntaxNode* node) {
        genNode(node->children[1]);
        emit("mov     " + varRef(lookupVar(node->children[0]->value)) + ", eax");
    }

    void genBinaryExpr(syntaxNode* node) {
        const string& op = node->value;

        if (op == "||" || op == "&&") { genLogical(node, op); return; }

        genNode(node->children[0]);
        emit("mov     ebx, eax");
        emit("push    rbx");
        genNode(node->children[1]);
        emit("mov     ecx, eax");
        emit("pop     rbx");
        emit("mov     eax, ebx");
        emit("mov     edx, ecx");

        if      (op == "+")  emit("add     eax, edx");
        else if (op == "-")  emit("sub     eax, edx");
        else if (op == "*")  emit("imul    eax, edx");
        else if (op == "/")  { emit("cdq"); emit("idiv    edx"); }
        else if (op == "%")  { emit("cdq"); emit("idiv    edx"); emit("mov     eax, edx"); }
        else if (op == "&")  emit("and     eax, edx");
        else if (op == "|")  emit("or      eax, edx");
        else if (op == "^")  emit("xor     eax, edx");
        else if (op == "<<") { emit("mov     ecx, edx"); emit("shl     eax, cl"); }
        else if (op == ">>") { emit("mov     ecx, edx"); emit("sar     eax, cl"); }
        else {
            // comparison operators
            static const unordered_map<string, string> setcc = {
                {"==","sete"}, {"!=","setne"},
                {"<","setl"},  {"<=","setle"},
                {">","setg"},  {">=","setge"}
            };
            emit("cmp     eax, edx");
            emit(setcc.at(op) + "   al");
            emit("movzx   eax, al");
        }
    }

    void genLogical(syntaxNode* node, const string& op) {
        bool isOr        = (op == "||");
        string n          = to_string(labelCounter++);
        string shortLabel = (isOr ? "or" : "and") + n + "_short";
        string endLabel   = (isOr ? "or" : "and") + n + "_end";

        genNode(node->children[0]);
        emit("cmp     eax, 0");
        emit((isOr ? "jne     " : "je      ") + shortLabel);

        genNode(node->children[1]);
        emit("cmp     eax, 0");
        emit((isOr ? "jne     " : "je      ") + shortLabel);

        emit("mov     eax, " + string(isOr ? "0" : "1"));
        emit("jmp     " + endLabel);

        emitLabel(shortLabel);
        emit("mov     eax, " + string(isOr ? "1" : "0"));

        emitLabel(endLabel);
    }

    void genUnaryExpr(syntaxNode* node) {
        genNode(node->children[0]);
        const string& op = node->value;
        if      (op == "-") emit("neg     eax");
        else if (op == "~") emit("not     eax");
        else if (op == "!") { emit("cmp     eax, 0"); emit("sete    al"); emit("movzx   eax, al"); }
        else if (op != "+") throw runtime_error("Unknown unary operator: " + op);
    }

public:
    string generate(syntaxNode* root) {
        if (!root) { cerr << "Error: null AST root\n"; return ""; }
        try {
            genNode(root);
            ostringstream out;
            out << ".intel_syntax noprefix\n.file   \"output.s\"\n\n";
            if (!dataSec.str().empty())
                out << ".data\n" << dataSec.str() << "\n";
            out << textSec.str();
            return out.str();
        } catch (const runtime_error& e) {
            cerr << "CodeGen error: " << e.what() << "\n";
            return "";
        }
    }
};