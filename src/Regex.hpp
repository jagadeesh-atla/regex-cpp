#pragma once

#include <cassert>
#include <iostream>
#include <set>
#include <stack>
#include <string>
#include <vector>

#include "NFA.hpp"
#include "Utils.hpp"

struct ParenthesisInfo {
  int noOfAlternations;
  int noOfAtoms;
};

class Regex {
 public:
  Regex(std::string _re) : rexp(_re) {}

  void compile() {
    std::cout << rexp << std::endl;
    polish = parseRe(rexp);
    nfa = new NFA();
    start = nfa->getNFA(polish);
  }

  std::vector<int> getPolish() { return polish; }
  State* getNFAState() { return start; }

  bool match(std::string s) { return nfa->match(start, s); }

 private:
  std::string rexp;
  State* start;
  NFA* nfa;
  std::vector<int> polish;
  std::vector<int> parseRe(std::string re);
  //   std::set<char> metaChars{'?', '.', '|', ')', '(', '+', '*',
  //                            '{', '}', '[', '[', '~', '^'};
};

std::vector<int> Regex::parseRe(std::string re) {
  int nofAlternations = 0;
  int noOfAtoms = 0;

  std::vector<int> buf;
  std::stack<ParenthesisInfo> stack;

  size_t len = re.size();
  for (size_t i = 0; i < len; i++) {
    int c = re[i];
    switch (c) {
      case '\\': {  // Escaping of meta-characters
        if (i + 1 >= len) {
          std::cerr << "Invalid Escaping \\" << std::endl;
          exit(1);
        }

        int next = re[++i];
        switch (next) {
            // clang-format off
          case '(': case '|': case ')': case '*':
          case '+': case '?': case '.': case '[':
          case ']': case '{': case '}': case '\\':
          if (noOfAtoms > 1) {
            --noOfAtoms;
            buf.push_back(Utils::Token::CONCAT);
          }
          buf.push_back(next);
          ++noOfAtoms;
          break;
          default:
            assert(0 && "Unreachable or Unsported Escape Character " && char(next));
            exit(1);
            // clang-format on
        }
      } break;
      case '(': {  // Start of paranthesis
        if (noOfAtoms > 1) {
          --noOfAtoms;
          buf.push_back(Utils::Token::CONCAT);
        }
        stack.push({nofAlternations, noOfAtoms});
        nofAlternations = noOfAtoms = 0;
      } break;
      case '|': {  // Alternation
        if (noOfAtoms == 0) {
          std::cerr << "Invalid Altrnation |" << std::endl;
          exit(1);
        }
        while (--noOfAtoms > 0) buf.push_back(Utils::Token::CONCAT);
        nofAlternations++;
      } break;
      case ')': {  // Close Parenthesis
        if (stack.empty() or noOfAtoms == 0) {
          std::cerr << "Invalid Paranthesis Closing" << std::endl;
          exit(1);
        }
        while (--noOfAtoms > 0) buf.push_back(Utils::Token::CONCAT);
        for (; nofAlternations > 0; nofAlternations--)
          buf.push_back(Utils::Token::ALTERNATION);
        auto [_nofAlternations, _noOfAtoms] = stack.top();
        stack.pop();
        nofAlternations = _nofAlternations;
        noOfAtoms = _noOfAtoms + 1;
      } break;
      case '*': {  // Zero-or-More
        if (noOfAtoms == 0) {
          std::cerr << "Invalid use of Quantifer *" << std::endl;
          exit(1);
        }
        buf.push_back(Utils::Token::ZEROORMORE);
      } break;
      case '+': {  // One-or-More
        if (noOfAtoms == 0) {
          std::cerr << "Invalid use of Quantifer +" << std::endl;
          exit(1);
        }
        buf.push_back(Utils::Token::ONEORMORE);
      } break;
      case '?': {  // Zero-or-One
        if (noOfAtoms == 0) {
          std::cerr << "Invalid use of Quantifer ?" << std::endl;
          exit(1);
        }
        buf.push_back(Utils::Token::ZEROORONE);
      } break;
      case '{': {  // Range Quantifer
        if (i == 0) {
          std::cout << "Invalid {" << std::endl;
          exit(1);
        }

        char prev = re[i - 1];
        std::vector<int> sub_post{};

        switch (prev) {  // cosntruct parse of sub_post;
          case ')': {    // sub-expression
            std::string sub;
            int nested = 1;
            size_t j = i - 2;
            for (; j != 0; j--) {
              if (re[j] == '(' and --nested == 0)
                break;
              else if (re[j] == ')')
                ++nested;
              sub.push_back(re[j]);
            }

            if (nested != 0 or (j == 0 and re[j] != '(')) {
              std::cerr << "Invalid Expression" << std::endl;
              exit(1);
            }
            sub_post = parseRe(std::string(sub.rbegin(), sub.rend()));
          } break;
          case ']': {  // character class
            std::string sub;
            int nestedBrackets = 1;
            size_t j = i - 2;
            for (; j != 0; j--) {
              if (re[j] == '[' && --nestedBrackets == 0)
                break;
              else if (re[j] == ']')
                nestedBrackets++;
              sub.push_back(re[j]);
            }
            if (nestedBrackets != 0 || (j == 0 && re[j] != '[')) {
              std::cerr << "Invalid Expression." << std::endl;
              exit(1);
            }
            sub_post =
                parseRe("[" + std::string(sub.rbegin(), sub.rend()) + "]");
          } break;
          default:
            sub_post.push_back(prev);
            break;
        }

        std::string rangeBuf;
        int noOfComma = 0;
        long int posOfComma = -(long int)i;
        while (++i < re.size() && re[i] != '}') {
          if (isdigit(re[i]) || re[i] == ',') {
            rangeBuf.push_back(re[i]);
            if (re[i] == ',') {
              if (noOfComma >= 1) {
                std::cerr << "Too many commas..." << std::endl;
                exit(1);
              }
              noOfComma++;
              posOfComma += (long int)i - 1;
            }
          } else {
            std::cerr << "Expected number in range{}" << std::endl;
            exit(1);
          }
        }

        if (i >= re.size() || re[i] != '}') {
          std::cerr << "No Closing }" << std::endl;
          exit(1);
        }
        if (noOfComma == 0) {  // {n} exactly n times
          int n = stoi(rangeBuf);
          while (--n > 0) {
            buf.insert(buf.end(), sub_post.begin(), sub_post.end());
            buf.push_back(Utils::Token::CONCAT);
          }
        } else {  // {n,}, {,m}, {n,m}
          int n = 0, m = 0;
          if (rangeBuf.front() == ',') {  // {,m}
            m = stoi(rangeBuf.substr(1));
            sub_post.push_back(Utils::Token::ZEROORONE);
            buf.push_back(Utils::Token::ZEROORONE);
            while (--m > 0) {
              buf.push_back(Utils::Token::CONCAT);
              buf.insert(buf.end(), sub_post.begin(), sub_post.end());
            }
          } else {  // {n,}, {n,m}
            n = stoi(rangeBuf.substr(0, (size_t)posOfComma));
            if (rangeBuf.size() > (size_t)posOfComma + 1) {  // {n,m}
              m = stoi(rangeBuf.substr((size_t)posOfComma + 1));
              m = m - n;
              while (--n > 0) {
                buf.push_back(Utils::Token::CONCAT);
                buf.insert(buf.end(), sub_post.begin(), sub_post.end());
              }
              sub_post.push_back(Utils::Token::ZEROORONE);
              while (m-- > 0) {
                buf.push_back(Utils::Token::CONCAT);
                buf.insert(buf.end(), sub_post.begin(), sub_post.end());
              }
            } else {  // {n,}
              n--;
              int tn = n;
              while (n--) {
                buf.insert(buf.end(), sub_post.begin(), sub_post.end());
              }
              buf.push_back(Utils::Token::ONEORMORE);
              while (tn--) {
                buf.push_back(Utils::Token::CONCAT);
              }
            }
          }
        }
      } break;
      case '[': {  // Charcter Class
        if (i + 1 < len and re[i + 1] == ']') {
          std::cerr << "Empty Character Class []" << std::endl;
          exit(1);
        }
        bool isNegated = (i + 1 < len and re[i + 1] == '^');
        if (isNegated) i++;
        if (re[i] == ']') {
          std::cerr << "Empty Character Class [^]" << std::endl;
          exit(1);
        }

        std::set<char> chars;
        char prev = 0;
        bool inRange = false;

        while (i + 1 < len and re[++i] != ']') {
          if (re[i] == '-' and prev != 0 and re[i + 1] != ']') {
            inRange = true;
            continue;
          }
          if (inRange) {
            for (char ch = prev; ch <= re[i]; ch++) chars.insert(ch);
            inRange = false;
          } else {
            chars.insert(re[i]);
            prev = re[i];
          }
        }

        if (i >= len or re[i] != ']') {
          std::cerr << "Undetermined Character Class" << std::endl;
          exit(1);
        }

        if (noOfAtoms > 1) {
          --noOfAtoms;
          buf.push_back(Utils::Token::CONCAT);
        }

        if (isNegated) {
          int count = -1;
          for (char ch = 0; ch < 127; ch++)
            if (not chars.count(ch)) {
              buf.push_back(ch);
              ++count;
            }
          while (count--) buf.push_back(Utils::Token::ALTERNATION);
        } else {
          int count = -1;
          for (char ch : chars) {
            buf.push_back(ch);
            count++;
          }
          while (count--) buf.push_back(Utils::Token::ALTERNATION);
        }

        noOfAtoms++;
      } break;
      default: {  // Noraml char
        if (noOfAtoms > 1) {
          --noOfAtoms;
          buf.push_back(Utils::Token::CONCAT);
        }
        buf.push_back(c);
        noOfAtoms++;
      } break;
    }
  }

  if (not stack.empty()) {
    std::cerr << "Invalid Expression, unbalanced ()" << std::endl;
    exit(1);
  }

  while (--noOfAtoms > 0) buf.push_back(Utils::Token::CONCAT);
  for (; nofAlternations > 0; nofAlternations--)
    buf.push_back(Utils::Token::ALTERNATION);

  return buf;
}