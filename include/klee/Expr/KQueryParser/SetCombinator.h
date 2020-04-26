#ifndef KLEE_SETCOMBINATOR_H
#define KLEE_SETCOMBINATOR_H

#include<cstdio>
#include<iostream>
#include<vector>
#include<set>
#include<algorithm>
#include<math.h>
#include<iterator>
#include<sstream>
#include<fstream>

extern "C" {
  #include<hammer/hammer.h>
}

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

    std::string getName() { return identifier; }
    int getLength() { return parserLength; }

    void setElement(int index, std::set<char> newSet);
    void unionSets(int index, std::set<char> otherSet);
    void intersectSets(int index, std::set<char> otherSet);
    void complementSet(int index);

    void printChRange(); 
    HParser* outputCombinator(); 
    

    SetCombinator();
    SetCombinator(std::string identifier, int parserLength);
};

// template <std::size_t... I>
// class index_sequence {};

// template <std::size_t N, std::size_t ...I>
// struct make_index_sequence : make_index_sequence<N-1, N-1,I...> {};

// template <std::size_t ...I>
// struct make_index_sequence<0,I...> : index_sequence<I...> {};

#endif // KLEE_SETCOMBINATOR_H