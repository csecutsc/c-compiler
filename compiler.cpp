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
#include "llvm/IR/Module.h"

// -----------------------
// Tokenizer
// ----------------------

enum TokenType {
  END = 0,
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
  retlist.push_back({TokenType::END, ""}); 
  return retlist;
}

// -----------------------
// Global LLVM Constructs
// ----------------------

static llvm::LLVMContext* TheContext = new llvm::LLVMContext();
static llvm::Module* TheModule = new llvm::Module("AwesomeCompiler", *TheContext);
static llvm::IRBuilder<>* Builder = new llvm::IRBuilder<>(*TheContext);
static std::map<std::string, llvm::Value*> VariablesMap;

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
  public:
    NumberExpr(const double val){
      this->val = val;
    }
    void debug(){
      std::cout << "NumberExpr: " << val << std::endl;
    }
    llvm::Value *codegen(){
      return llvm::ConstantFP::get(*TheContext, llvm::APFloat(val));
    }
};

class VariableExpr : public Expr{
  std::string name;
  public:
    VariableExpr(const std::string &name){
      this->name = name;
    }
    void debug(){
      std::cout << "VariableExpr : " << name << std::endl;
    }
    llvm::Value *codegen(){
      return VariablesMap[name];
    }
    std::string get_name(){
      return this->name;
    }
};

class BinaryExpr : public Expr{
  Token operation;
  Expr* LHS;
  Expr* RHS;
  public:
    BinaryExpr(Token operation, Expr *LHS, Expr* RHS){
      this->operation = operation;
      this->LHS = LHS;
      this->RHS = RHS;
    }
    llvm::Value *codegen(){
      llvm::Value *L = LHS->codegen();
      llvm::Value *R = RHS->codegen();
      if (operation.lexeme == "+"){
          return Builder->CreateAdd(L, R, "addreg");
      }
      else if(operation.lexeme == "-"){
          return Builder->CreateSub(L, R, "subreg");
      }
      else if(operation.lexeme == "*"){
          return Builder->CreateMul(L, R, "mulreg");
      }
      else if(operation.lexeme == "/"){
          return Builder->CreateExactUDiv(L, R, "divreg");
      }
      else if(operation.lexeme == "="){
          VariableExpr* IdentLHS = (VariableExpr*)LHS;
          VariablesMap[IdentLHS->get_name()] = R;
          return Builder->CreateAdd(R, Builder->getInt64(0), IdentLHS->get_name());
      }
      else{
        return nullptr;
      }
    }

    void debug(){
      std::cout << "BinaryExpr : " << operation.lexeme << std::endl;
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
      llvm::Function *CalleeF = TheModule->getFunction(callee);
      std::vector<llvm::Value*> args_values;
      for (int i = 0; i < args.size(); i++){
        args_values.push_back(args[i]->codegen());
      }
      return Builder->CreateCall(CalleeF, args_values, "callreg");
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

class FunctionExpr : public Expr{
  std::string name;
  std::vector<std::string> args;
  std::vector<Expr*> body;
  
  public:
    FunctionExpr(std::string name, std::vector<std::string> args, std::vector<Expr*> body){
      this->name = name;
      this->args = args;
      this->body = body;
    }
    llvm::Function *codegen(){
      std::vector<llvm::Type*> Integers(args.size(), llvm::Type::getInt64Ty(*TheContext));
      llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getInt64Ty(*TheContext), Integers, false);
      llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, name, TheModule);

      //VariablesMap.clear();
      int i = 0;
      for (auto &arg : F->args()){
        arg.setName(args[i]); // set function arguments
        VariablesMap[args[i]] = &arg; // set this address into the mapping
        i++;
      }

      llvm::BasicBlock *BB = llvm::BasicBlock::Create(*TheContext, "entry", F);
      Builder->SetInsertPoint(BB);

      for (int i = 0; i < body.size(); i++){
        body[i]->codegen();
      }
      return F;
    }

    void debug(){
      std::cout << "FunctionExpr : " << std::endl;
      for (int i = 0; i < args.size(); i++){
        std::cout << "\t " << args[i] << std::endl;
      }
      for (int i = 0; i < body.size(); i++){
        std::cout  << "\t ";
        body[i]->debug();
      }
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
        case TokenType::Equal:
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
          if (expression != nullptr){
            ret_expressions.push_back(expression);
          }
        }
      }
      return ret_expressions;
    }

    FunctionExpr* parseFunction(){
      consume_token(); // consume the 'func' Keyword
      std::string function_name = current_token.lexeme;
      std::vector<std::string> args;
      std::vector<Expr*> expressions;
      // get arguments
      consume_token(); // consume the name
      consume_token(); // consume '('
      while (current_token.token_type != TokenType::RParen){
        args.push_back(current_token.lexeme);
        consume_token();
      }
      // get the expressions in the body
      consume_token(); // consume ')'
      consume_token(); // consume '{'
      while (current_token.token_type != TokenType::RCurlyParen){
        if (current_token.token_type == TokenType::Semicolon){
          consume_token();
        } 
        else{
          Expr* V = parseExpression();
          if (V != nullptr){
            expressions.push_back(V);
          }
        }
      }
      consume_token(); // consume '}'
      return new FunctionExpr(function_name, args, expressions);
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
        Token op_token = current_token;
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
        LHS = new BinaryExpr(op_token, LHS, RHS);
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
  
    // codegen for all expressions
    for (int i = 0; i < expressions.size(); i++){
      llvm::Value* fv = expressions[i]->codegen();
      fv->print(llvm::errs());
      std::cout << "---" << std::endl;
    }

    //for (int i = 0; i < expressions.size(); i++){
    //  expressions[i]->debug();
    //  std::cout << "---" << std::endl;
    //}

  }
  else{
    std::cout << "usage: ./compiler <filename>";
  }
  return 0;
}
