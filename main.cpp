#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

#include "Regex.hpp"

void test_line(std::fstream& file) {
  std::string line;
  struct Test {
    std::string re;
    std::string str;
  };

  while (getline(file, line)) {
    std::stringstream ss(line);
    Test test;
    ss >> test.re >> test.str;

    Regex* r = new Regex(test.re);
    r->compile();

    std::map<int, int> results = r->matchAll(test.str);

    std::cout << "LINE: " << test.str << std::endl;
    for (auto [idx, size] : results) {
      std::cout << test.str.substr(idx, size) << std::endl;
    }

    std::cout << "---------------------------" << std::endl;
  }
}

void test_simple(std::fstream& file) {
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
}

int main(int, char** argv) {
  std::fstream file(argv[1]);

  if (file.is_open())
    test_line(file);
  else
    std::cerr << "File not found" << std::endl;
  return 0;
}