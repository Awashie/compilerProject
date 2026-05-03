#include "lexicalAnalyzer.cpp"

using namespace std;

struct syntaxNode{
    string syntaxType;
    string value;
    vector<syntaxNode*> children;
};

class syntax {
private:
    vector<Token> tokens;
    int pos;

public:
    


};
