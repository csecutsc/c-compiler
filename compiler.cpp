#include <iostream>
#include <vector>
#include <map>
#include <fstream>
using namespace std;

enum Tokens {
  EOL = -1,
  Semicolon = 1,
  LParen = 2,
  RParen = 3,
  LCurlyParen = 4,
  RCurlyParen = 5,
  LSquareParen = 6,
  RSquareParen = 7,
  Star = 8,
  Slash = 9,
  Equal = 10,
  Minus = 11,
  Plus = 12,
  Carrot = 13,
  Identifier = 14,
  Literal = 15,
};

int main(int argc, char* argv[]){
  if (argc >= 2){
    ifstream file(argv[1]);
    if (!file.is_open()) {
      cerr << "Failed to open file: " << argv[1] << endl;
      return 1;
    }
    string line;
    while (getline(file, line)) {
      cout << line << endl;
    }
    
  }
  else{
    cout << "usage: ./compiler <filename>";
  }
  return 0;
}
