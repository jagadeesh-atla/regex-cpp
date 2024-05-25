#include <fstream>
#include <iostream>
#include <sstream>

#include "Regex.hpp"

int main(int, char** argv) {
  std::fstream file(argv[1]);

  struct Test {
    std::string re;
    std::string str;
    bool res;
  };

  int num = 1;

  std::string line;
  while (getline(file, line)) {
    std::stringstream ss(line);
    Test test;
    std::string bl;
    ss >> test.re >> test.str >> bl;
    if (bl == "1")
      test.res = 1;
    else
      test.res = 0;

    Regex* r = new Regex(test.re);
    r->compile();

    if (r->match(test.str) == test.res) {
      std::cout << num++ << "\tResult : Correct" << std::endl;
    } else {
      std::cout << num++ << "\tResult : Wrong" << std::endl;
      Utils::printTokens(r->getPolish());
    }
    //   std::cout << r->match(test.str) << std::endl;
  }

  return 0;
}