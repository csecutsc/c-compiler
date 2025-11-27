#include <iostream>
#include <vector>
#include <map>
#include <fstream>
// --- LLVM Stuff ----
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"




// -----------------------
// Tokenizer
// ----------------------

enum TokenType {
  Semicolon = 1,
  LParen = 2,
  RParen = 3,
  LCurlyParen = 4,
  RCurlyParen = 5,
  Plus = 6,
  Minus = 7,
  Star = 8,
  Slash = 9,
  Equal = 10,
  Carrot = 11,
  Identifier = 12,
  NumericLiteral = 13,
  Keyword = 14,
};

struct Token{
  TokenType token_type; // token's type ENUM
  std::string lexeme; // actual string value of the token
};

std::vector<Token> tokenize(std::string raw_stream){
  // continue to split a stream into Tokens
  std::vector<Token> retlist = {};
  int fileptr = 0;
  while (fileptr < raw_stream.size()){
    char c = raw_stream[fileptr++];
    switch(c){
      case '(':
        retlist.push_back({TokenType::LParen, "("}); 
        break;
      case ')':
        retlist.push_back({TokenType::RParen, ")"}); 
        break;
      case '{':
        retlist.push_back({TokenType::LCurlyParen, "{"}); 
        break;
      case '}':
        retlist.push_back({TokenType::RCurlyParen, "}"}); 
        break;
      case '+':
        retlist.push_back({TokenType::Plus, "+"}); 
        break;
      case '-':
        retlist.push_back({TokenType::Minus, "-"}); 
        break;
      case '*':
        retlist.push_back({TokenType::Star, "*"}); 
        break;
      case '/':
        retlist.push_back({TokenType::Slash, "/"}); 
        break;
      case '=':
        retlist.push_back({TokenType::Slash, "="}); 
        break;
      case '^':
        retlist.push_back({TokenType::Carrot, "^"}); 
        break;
      default:
        // check if identifier, if it is, consume all
        if (isalpha(c)){
          std::string identifier = "";
          while (isalpha(c)){
            identifier += c;
            c = raw_stream[fileptr++];
          }
          // check if keyword
          if (identifier == "fun"){
            retlist.push_back({TokenType::Keyword, "fun"}); 
          }
          else{
            retlist.push_back({TokenType::Identifier, identifier}); 
          }
          fileptr--; // dec as we reach this only if last character is not alpha, and we still want to parse this
        }
        else if (isdigit(c)){
          std::string numericliteral = "";
          while (isdigit(c)){
            numericliteral += c;
            c = raw_stream[fileptr++];
          }
          retlist.push_back({TokenType::NumericLiteral, numericliteral}); 
          fileptr--;
        }
        break;
    }
  }
  return retlist;
}

// -----------------------
// AST NODES DEFINITIONS
// ----------------------
class Expr{
  public:
    virtual ~Expr() = default;
    virtual llvm::Value *codegen() = 0;
};

class NumberExpr: public Expr{
  double Val;
  llvm::Value *codegen(){
    //
  }
};

class VariableExpr : public Expr{
  std::string name;
  llvm::Value *codegen(){
    //
  }
  public:
    VariableExpr(const std::string &name){
      this->name = name;
    }
};

class BinaryExpr : public Expr{
  TokenType operation;
  Expr* LHS;
  Expr* RHS;
  public:
    BinaryExpr(TokenType operation, Expr *LHS, Expr* RHS){
      this->operation = operation;
      this->LHS = LHS;
      this->RHS = RHS;
    }
    llvm::Value *codegen(){
      //
    }
};

class CallExpr : public Expr{
  std::string callee;
  std::vector<Expr*> args;
  public:
    CallExpr(const std::string &callee, std::vector<Expr*> args){
      this->callee = callee;
      this->args = args;
    }
    llvm::Value *codegen(){
      //
    }

};

class ProtoExpr : public Expr{
  std::string name;
  std::vector<std::string> args;
  public:
    ProtoExpr(const std::string &name, std::vector<std::string> args){
      this->name = name;
      this->args = args;
    }
    llvm::Function *codegen(){
      //
    }
    const std::string getName() {
      return this->name;
    }
};

class FunctionExpr : public Expr{
  ProtoExpr* proto;
  Expr* body;
  
  public:
    FunctionExpr(ProtoExpr* proto, Expr* body){
      this->proto = proto;
      this->body = body;
    }
    llvm::Function *codegen(){
      //
    }
};

// -----------------------
// Parser
// ----------------------
std::vector<Expr*> parse(std::vector<Token> tokens){
  // no backtracking
  vector<Expr*> ret_expressions;
  int tokenindex = 0;
  while (tokenindex < tokens.size()){
    Token current_token = tokens[tokenindex];

    if(current_token.token_type == TokenType::Keyword && current_token.lexeme=="fun"){
      FunctionExpr* expression = parseFunction(tokens, &tokenindex);
      ret_expressions.push_back(expression);
    }
    else{
      Expr* expression = parseExpression(tokens, &tokenindex);
      ret_expressions.push_back(expression);
    }
  }
  return ret_expressions;
}

Expr* parseFunction(std::vector<Token> tokens, int* tokenindex){
  //
}

Expr* parseExpression(std::vector<Token> tokens, int* tokenindex){
  //
}


int main(int argc, char* argv[]){
  if (argc >= 2){
    std::ifstream file(argv[1]);
    if (!file.is_open()) {
      std::cerr << "Failed to open file: " << argv[1] << std::endl;
      return 1;
    }
    std::string line;
    std::string full_string = "";
    while (getline(file, line)) {
      full_string += line;
    }
    std::cout << full_string << std::endl;

    // tokenize
    std::vector<Token> tokens = tokenize(full_string);
    // get expression nodes
    std::vector<Expr*> expressions = parse(tokens);

    for (int i = 0; i < tokens.size(); i++){
      std::cout << tokens[i].token_type << " " << tokens[i].lexeme << '\n';
    }
  }
  else{
    std::cout << "usage: ./compiler <filename>";
  }
  return 0;
}
