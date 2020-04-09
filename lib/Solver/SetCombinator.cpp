#include "klee/Expr/KQueryParser/SetCombinator.h"

// Assumptions: index will always be < parserLength

void SetCombinator::setElement(int index, std::set<char> newSet) {
  // check to see if any value has been 
  setList[index] = newSet;
}

void SetCombinator::unionSets(int index, std::set<char> otherSet) {
  for (char ch : otherSet) {
    setList[index].insert(ch);
  }
}

void SetCombinator::intersectSets(int index, std::set<char> otherSet) {
  std::set<char> intSet;
  for (char ch : otherSet) {
    if (setList[index].find(ch) != setList[index].end()) {
      intSet.insert(ch);
    }
  }
  setList[index] = intSet;
}

void SetCombinator::complementSet(int index) {
  // Return complement of set
  std::set<char> compSet;
  for (int i  = 0; i < 128; i++) {
    if (setList[index].find(i) != setList[index].end()) {
      compSet.insert((char) i);
    }
  }
  setList[index] = compSet;
}
 
void SetCombinator::printChRange() {
  for (int i = 0; i < setList.size(); i++) {
    std::cout << "Position " << i << ':'; 
    bool first = true;
    char beg; 
    char prevCh; 
    std::set<char>::iterator itr;
    for (char ch : setList[i]) {
      if (first) {
        first = false;
        beg = ch;
      } else if ((int) ch - (int) prevCh != 1) {
        if (beg != prevCh) {
          std::cout << (int) beg << '-' << (int) prevCh << std::endl;
        } else {
          std::cout << (int) beg << std::endl;
        } 
        beg = ch;
      }
      prevCh = ch;
    }
    if (beg != prevCh) {
      std::cout << (int) beg << '-' << (int) prevCh << std::endl;
    } else {
      std::cout << (int) beg << std::endl;
    }
  }
}

SetCombinator::SetCombinator() {
  this->parserLength = 0;
  this->identifier = "";
}

SetCombinator::SetCombinator(std::string identifier, int parserLength) { 
  this->parserLength = parserLength;
  this->identifier = identifier;
  // Initialize to allow all characters
  for (int i = 0; i < parserLength; i++) {
    std::set<char> fullSet;
    for (int j = 0; j < 128; j++) {
      fullSet.insert((char) j); 
    } 
    setList.push_back(fullSet);
  }
}