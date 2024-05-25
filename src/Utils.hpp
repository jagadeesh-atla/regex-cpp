#pragma once

#include <iostream>
#include <string>
#include <vector>

namespace Utils {

enum Token {
  CONCAT = 300,
  OPENPAREN,
  CLOSEPAREN,
  ALTERNATION,
  ZEROORMORE,
  ONEORMORE,
  ZEROORONE,
  OPENBRACKET,
  CLOSEBRACK,
  OPENCURLY,
  CLOSECURLY,
  WILDCARD,
};

enum Type { MATCH = 256, SPLIT, WILDCARDType };

/* clang-format off */
std::string TokenToStr(int token) {
  switch (token) {
    case CONCAT:        return "CONCAT";
    case OPENPAREN:     return "OPENPAREN";
    case CLOSEPAREN:    return "CLOSEPAREN";
    case ALTERNATION:   return "ALTERNATION";
    case ZEROORMORE:    return "ZEROORMORE";
    case ONEORMORE:     return "ONEORMORE";
    case ZEROORONE:     return "ZEROORONE";
    case OPENBRACKET:   return "OPENBRACKET";
    case CLOSEBRACK:    return "CLOSEBRACK";
    case OPENCURLY:     return "OPENCURLY";
    case CLOSECURLY:    return "CLOSECURLY";
    case WILDCARD:      return "WILDCARD";
    default:
      if (token < 256)  return std::string(1, (char)token);
      else              return "UNKNOWN";
  }
}

/* clang-format on */

void printTokens(const std::vector<int>& vec) {
  for (int token : vec) std::cout << TokenToStr(token) << " ";
  std::cout << std::endl;
}
};  // namespace Utils
