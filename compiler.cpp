#include <iostream>
#include <vector>
#include <map>
#include <fstream>
using namespace std;

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
};

struct Token{
  TokenType token_type; // token's type ENUM
  string lexeme; // actual string value of the token
};

vector<Token> tokenize(string raw_stream){
  // continue to split a stream into Tokens
  vector<Token> retlist = {};
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
          string identifier = "";
          while (isalpha(c)){
            identifier += c;
            c = raw_stream[fileptr++];
          }
          retlist.push_back({TokenType::Identifier, identifier}); 
          fileptr--;
        }
        else if (isdigit(c)){
          string numericliteral = "";
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

int main(int argc, char* argv[]){
  if (argc >= 2){
    ifstream file(argv[1]);
    if (!file.is_open()) {
      cerr << "Failed to open file: " << argv[1] << endl;
      return 1;
    }
    string line;
    string full_string = "";
    while (getline(file, line)) {
      full_string += line;
    }
    cout << full_string << endl;

    // tokenize
    vector<Token> tokens = tokenize(full_string);
    for (int i = 0; i < tokens.size(); i++){
      cout << tokens[i].lexeme << '\n';
    }
  }
  else{
    cout << "usage: ./compiler <filename>";
  }
  return 0;
}
