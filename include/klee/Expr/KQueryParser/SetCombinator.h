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
    // List of sets, where each set at the given index is the 
    // set of characters the index can take. 
    SetList setList;
    // Mapping index to list of sets, as restrained by different constraints.
    // Each constraint forms a separate set.
    // setList is the intersection of all the sets for each index.
    SetMap setMap;
    int parserLength;
    std::string identifier;
    

  public: 
    /** 
      * Forms setList by taking intersections of all the constraints from setMap.
      * 
      * @return setList 
    */
    SetList getSetList();

    SetMap getSetMap() { return setMap; }
    std::string getName() { return identifier; }
    int getLength() { return parserLength; }

    /** 
      * Updates setMap accordingly, based on whether set is constructed from 
      * new constraint or continuing from old one. 
      * Ex) (Eq X 97) is new constraint
      * Ex) (Eq False (Eq X 97)). Eq False is old constraint. 
      * 
      * @param index Index into setMap and ultimately setList which the constraints apply to
      * @param oldSet Existing allowed characters before current constraint
      * @param newSet Allowed characters after current constraint
    */
    void setElement(int index, std::set<char> oldSet, std::set<char> newSet);

    /** 
      * Updates setMap accordingly, taking complement of set for current constraint. 
      * Modifies oldSet to be its own complement. 
      * 
      * @param index Index into setMap and ultimately setList which the constraints apply to
      * @param oldSet Existing allowed characters before current constraint
    */
    void complementSet(int index, std::set<char> &oldSet);

    /** 
      * Method called to form setList from setMap by taking the intersection
      * of the list of constraints for each index. 
      * 
      * @param index Index into setList 
      * @param otherSet Set to intersect current set in setList[index]
    */
    void intersectSets(int index, std::set<char> otherSet);
    
    // unionSets not being used.
    void unionSets(int index, std::set<char> otherSet);
    
    /** 
      * Prints setList nicely. Calls getSetList() to form setList. 
    */
    void printChRange(); 

    /** 
      * Outputs parser combinator in file. Calls getSetList() to form setList. 
      * 
      * @return HParser*, which is a pointer to the formed parser, but
      *   value is not well supported. 
      *   TODO: figure out variadic functions to form the parser?
    */
    HParser* outputCombinator(); 
    

    SetCombinator();
    SetCombinator(std::string identifier, int parserLength);
};

#endif // KLEE_SETCOMBINATOR_H