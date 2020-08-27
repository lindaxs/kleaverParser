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
#include<map>

#define MAXVAL 128

extern "C" {
  #include<hammer/hammer.h>
}

typedef std::vector< std::set<char> > SetList;
typedef std::map< int, SetList > SetMap;

// Assumptions: index will always be < parserLength
class SetCombinator {
  private: 
    SetList setList;
    // Mapping index to list of sets, as restrained by different constraints.
    // setList is the intersection of all the sets for each index.
    SetMap setMap;
    int parserLength;
    std::string identifier;
    

  public: 
    SetList getSetList();
    SetMap getSetMap() { return setMap; }

    std::string getName() { return identifier; }
    int getLength() { return parserLength; }

    // Need to compare old and new set. 
    void setElement(int index, std::set<char> oldSet, std::set<char> newSet);
    void unionSets(int index, std::set<char> otherSet);
    void intersectSets(int index, std::set<char> otherSet);
    void complementSet(int index, std::set<char> &oldSet);

    void printChRange(); 
    HParser* outputCombinator(); 
    

    SetCombinator();
    SetCombinator(std::string identifier, int parserLength);
};

#endif // KLEE_SETCOMBINATOR_H