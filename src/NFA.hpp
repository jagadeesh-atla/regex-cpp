#pragma once

#include <iostream>
#include <stack>
#include <vector>

#include "Utils.hpp"

static int noOfStates;

struct State {
  int c = -1;
  State* out = nullptr;
  State* out1 = nullptr;
  int lastListId = -1;

  State(int _c) : c(_c){};
  State(int _c, State* _out, State* _out1) : c(_c), out(_out), out1(_out1) {
    ++noOfStates;
  }
};

//  Points to list of states
union PtrList {
  PtrList* next;
  State* s;
};

// Transtitions
struct Frag {
  State* start;
  PtrList* out;

  Frag() : start(nullptr), out(nullptr) {}
  Frag(State* _start, PtrList* _out) : start(_start), out(_out) {}
};

// List of all states
struct List {
  State** s;
  int n;
};

class NFA {
 public:
  ~NFA() {
    if (l1.s) delete[] l1.s;
    if (l2.s) delete[] l2.s;
  }

  State* getNFA(std::vector<int> postFix);
  bool match(State* start, std::string s);

 private:
  int listId;
  List l1, l2;
  State matchState = State(Utils::Type::MATCH);  // like Final State

  void addState(List*, State*);
  void transition(List*, int, List*);
  List* startList(State* start, List* l);
  bool isMatch(List*);

  // make list of states to witch pointed
  PtrList* makeList(State** state) {
    PtrList* list;
    list = (PtrList*)state;
    list->next = nullptr;
    return list;
  }

  // point list of states to state
  void patch(PtrList* list, State* state) {
    PtrList* next;
    for (; list; list = next) {
      next = list->next;
      list->s = state;
    }
  }

  // Join two lists l1 and l2
  PtrList* append(PtrList* l1, PtrList* l2) {
    PtrList* oldl1;
    oldl1 = l1;  // move to last of l1
    while (l1->next) l1 = l1->next;
    l1->next = l2;
    return oldl1;
  }
};

State* NFA::getNFA(std::vector<int> polish) {
  std::stack<Frag> stack;
  Frag e1, e2, e;
  State* s;

  if (polish.size() == 0) {
    std::cerr << "Empty Regex" << std::endl;
    exit(1);
  }

  for (int p : polish) {
    switch (p) {
      default: {  // Matcher
        s = new State(p);
        stack.push({s, makeList(&s->out)});
      } break;
      case '.': {
        s = new State(Utils::Type::WILDCARDType);
        stack.push({s, makeList(&s->out)});
      } break;
      case Utils::Token::CONCAT: {
        e2 = stack.top();
        stack.pop();
        e1 = stack.top();
        stack.pop();
        patch(e1.out, e2.start);
        stack.push({e1.start, e2.out});
      } break;
      case Utils::Token::ALTERNATION: {
        e2 = stack.top();
        stack.pop();
        e1 = stack.top();
        stack.pop();
        s = new State(Utils::Type::SPLIT, e1.start, e2.start);
        stack.push({s, append(e1.out, e2.out)});
      } break;
      case Utils::Token::ZEROORONE: {
        e = stack.top();
        stack.pop();
        s = new State(Utils::Type::SPLIT, e.start, nullptr);
        stack.push({s, append(e.out, makeList(&s->out1))});
      } break;
      case Utils::Token::ZEROORMORE: {
        e = stack.top();
        stack.pop();
        s = new State(Utils::Type::SPLIT, e.start, nullptr);
        patch(e.out, s);
        stack.push({s, makeList(&s->out1)});
      } break;
      case Utils::Token::ONEORMORE: {
        e = stack.top();
        stack.pop();
        s = new State(Utils::Type::SPLIT, e.start, nullptr);
        patch(e.out, s);
        stack.push({e.start, makeList(&s->out1)});
      } break;
    }
  }

  e = stack.top();
  stack.pop();
  if (not stack.empty()) {
    Utils::printTokens(polish);
    std::cerr << "Invalid Regex" << std::endl;
    exit(1);
  }

  patch(e.out, &matchState);

  l1.s = new State*[noOfStates];
  l2.s = new State*[noOfStates];

  return e.start;
}

List* NFA::startList(State* start, List* l) {
  l->n = 0;
  listId++;
  addState(l, start);
  return l;
}

bool NFA::isMatch(List* l) {
  for (int i = 0; i < l->n; i++)
    if (l->s[i] == &matchState) return true;
  return false;
}

void NFA::addState(List* l, State* s) {
  if (s == nullptr or s->lastListId == listId) return;
  s->lastListId = listId;
  if (s->c == Utils::Type::SPLIT) {
    addState(l, s->out);
    addState(l, s->out1);

    return;
  }
  l->s[l->n++] = s;
  return;
}

void NFA::transition(List* clist, int c, List* nlist) {
  int i;
  State* s;
  listId++;
  nlist->n = 0;
  for (i = 0; i < clist->n; i++) {
    s = clist->s[i];
    if (s->c == c or s->c == Utils::Type::WILDCARDType) addState(nlist, s->out);
  }
}

/* Run NFA to determine whether it matches s. */
bool NFA::match(State* start, std::string s) {
  List *clist, *nlist;
  clist = startList(start, &l1);
  nlist = &l2;
  for (char c : s) {
    c = c & 0xFF;
    transition(clist, c, nlist);
    std::swap(clist, nlist);
  }
  return isMatch(clist);
}
