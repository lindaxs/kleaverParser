#ifndef KLEE_SETCOMBINATOR_H
#define KLEE_SETCOMBINATOR_H

#include<cstdio>
#include<iostream>
#include<vector>
#include<set>
#include<algorithm>
#include<math.h>
#include<iterator>

// Assumptions: index will always be < parserLength
class SetCombinator {
  private: 
    std::vector<std::set<char> > setList;
    int parserLength;
    std::string identifier;
    

  public: 
    std::vector<std::set<char> > getSetList() {
      return setList;
    }

    void setElement(int index, std::set<char> newSet);
    void unionSets(int index, std::set<char> otherSet);
    void intersectSets(int index, std::set<char> otherSet);
    void complementSet(int index);

    void printChRange(); 
    


    SetCombinator();
    SetCombinator(std::string identifier, int parserLength);
};

#endif // KLEE_SETCOMBINATOR_H