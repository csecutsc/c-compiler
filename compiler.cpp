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
        retlist.push_back({TokenType::Equal, "="}); 
        break;
      case '^':
        retlist.push_back({TokenType::Carrot, "^"}); 
        break;
      case ';':
        retlist.push_back({TokenType::Semicolon, ";"}); 
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
    virtual void debug() = 0;
};

class NumberExpr: public Expr{
  double val;
  llvm::Value *codegen(){
    //
  }
  public:
    NumberExpr(const double val){
      this->val = val;
    }
    void debug(){
      std::cout << "NumberExpr: " << val << std::endl;
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

    void debug(){
      std::cout << "VariableExpr : " << name << std::endl;
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

    void debug(){
      std::cout << "BinaryExpr : " << operation << std::endl;
      LHS->debug();
      RHS->debug();
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
    void debug(){
      std::cout << "CallExpr : " << callee << std::endl;
      std::cout << "arguments: " << std::endl;
      for (int i = 0; i < args.size(); i++){
        std::cout << "\t ";
        args[i]->debug();
      }
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
    void debug(){
      std::cout << "ProtoExpr : " << name << std::endl;
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
    void debug(){
      std::cout << "FunctionExpr : " << std::endl;
      proto->debug();
      body->debug();
    }
};

// -----------------------
// Parser
// ----------------------
class Parser{
  private:
    std::vector<Token> tokens;
    int token_index = 0;
    Token current_token;

  public:
    Parser(std::vector<Token> tokens){
      this->tokens = tokens;
    }
    
    void consume_token(){
      token_index++;
      current_token = tokens[token_index];
    }

    int get_current_token_precedence(){
      switch(current_token.token_type){
        case TokenType::Plus:
          return 10;
        case TokenType::Minus:
          return 10;
        case TokenType::Star:
          return 20;
        case TokenType::Slash:
          return 20;
        default:
          return -1;
      }
    }

    std::vector<Expr*> parse(){
      std::vector<Expr*> ret_expressions;
      while (token_index < tokens.size()-1){
        current_token = tokens[token_index];
        if(current_token.token_type == TokenType::Keyword && current_token.lexeme=="fun"){
          FunctionExpr* expression = parseFunction();
          ret_expressions.push_back(expression);
        }
        else if(current_token.token_type == TokenType::Semicolon){
          consume_token();
        }
        else{
          Expr* expression = parseExpression();
          ret_expressions.push_back(expression);
        }
      }
      return ret_expressions;
    }

    FunctionExpr* parseFunction(){
    }
  
    Expr* parsePrimary(){
      if (current_token.token_type == TokenType::Identifier){
        if (tokens[token_index+1].token_type == TokenType::LParen){
          // parse a call
          std::string call_name = current_token.lexeme;
          consume_token(); // eat the identifier
          std::vector<Expr*> args;
          consume_token(); // eat the '('
          // create a list of arguments
          while (current_token.token_type != TokenType::RParen){
            args.push_back(parseExpression()); // parse the current thing here
            //consume_token(); // move forwards
          }
          return new CallExpr(call_name, args);
        }
        else{
          // parse a variable
          return new VariableExpr(current_token.lexeme);
        }
      }
      else if (current_token.token_type == TokenType::NumericLiteral){
        return new NumberExpr(std::stod(current_token.lexeme));
      }
      else if(current_token.token_type == TokenType::LParen)
        // consume the token
        consume_token(); // consume (
        Expr* V = parseExpression();
        //consume_token(); // consume )
        return V;
    }
     
    Expr* parseBinary(int expr_precedence, Expr* LHS){
      while (true) {
        int token_precedence = get_current_token_precedence();
        if (token_precedence < expr_precedence) return LHS;
        consume_token(); // consume the operation
        Expr* RHS = parsePrimary();
        consume_token(); // get next operation or ;
        int next_precedence = get_current_token_precedence();
        if (token_precedence < next_precedence){
          // then, the RHS is more precedence, parse that first
          RHS = parseBinary(token_precedence+1, RHS);
        }
        LHS = new BinaryExpr(current_token.token_type, LHS, RHS);
      }
    }

    Expr* parseExpression(){
      Expr* LHS = parsePrimary();
      consume_token(); // consume the next operation, either ; or a +
      return parseBinary(0, LHS);
    }
};




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
    //for (int i = 0; i < tokens.size(); i++){
    //  std::cout << tokens[i].token_type << " " << tokens[i].lexeme << '\n';
    //}

    // get expression nodes
    Parser* parser = new Parser(tokens);
    std::vector<Expr*> expressions = parser->parse();


    for (int i = 0; i < expressions.size(); i++){
      expressions[i]->debug();
      std::cout << "---" << std::endl;
    }

  }
  else{
    std::cout << "usage: ./compiler <filename>";
  }
  return 0;
}
