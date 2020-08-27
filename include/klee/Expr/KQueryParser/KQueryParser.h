#ifndef KLEE_KQUERYPARSER_H
#define KLEE_KQUERYPARSER_H

#include "klee/Expr/Expr.h"
#include "klee/Expr/Parser/Parser.h"
#include "klee/Expr/Constraints.h"
#include "klee/Solver/Solver.h"

#include "SetCombinator.h"

#include <cassert>
#include <map>
#include <sstream>
#include <vector>

#define MAXVAL 128 

namespace klee {

  class Cond {
    public:
      int number;
      enum Type {
        Add, 
        Mul, 
        Concat, 
        Index,
        And,
      };
      Type type;
      Type getType() { return type; }
      int getNum() { return number; }

      Cond(Type t, int n=-1) {
        type = t;
        number = n;
      }
  };

  class KQueryElem {
    public: 
      int elem;
      enum Type {
        Null,
        Index, 
        Char, 
        Bool,
      };
      std::vector<Cond*> conds;
      virtual Type getType() const = 0;
  };

  class NullElem : public KQueryElem {
    public: 
      Type type = Null;
      Type getType() const { return Null; }
      NullElem() {;}
  };

  class IndexElem : public KQueryElem {
    public:
      Type type = Index;
      std::set<char> indexSet;
      
      // Maintain conditions
      Type getType() const { return Index; }
      std::set<char> getSet() const { return indexSet; }
      void setIndexSet(std::set<char> indexSet) { this->indexSet = indexSet; }
      void addCond(Cond* c) {
        conds.push_back(c);
      }
      IndexElem(int index) {
        elem = index;
        for (int i = 0; i < MAXVAL; i++) {
          indexSet.insert((char) i);
        }
      }
      IndexElem(int index, Cond *c) {
        elem = index;
        for (int i = 0; i < MAXVAL; i++) {
          indexSet.insert((char) i);
        }
        conds.push_back(c);
      }
      int evalCond(int ch) {
        for (auto cond : conds) {
          ch = evalCondHelper(ch, cond);
        }
        return ch;  
      }
    
    private: 
      int evalCondHelper(int ch, Cond *c) {
        switch(c->getType()) {
          case Cond::Add: { return ch + c->getNum(); }
          case Cond::Mul: { return ch * c->getNum(); }
          case Cond::And: { return ch & c->getNum(); }
          default: { return ch; }
        }
      }
  };

  class CharElem : public KQueryElem {
    public:
      Type type = Char;
      Type getType() const { return Char; }
      CharElem(int ch) {elem = ch;}
  };

  class BoolElem : public KQueryElem {
    public: 
      Type type = Bool;
      Type getType() const { return Bool; }
      BoolElem(bool b) { elem = b;}
  };


  class KQueryParser {
    private: 
      // Keep track of all constant arrays by name
      std::map<std::string, const Array*> arrayMap;
      std::string arrayName;
      // Keep track of all set combinators by name
      std::map<std::string, SetCombinator> combMap; 
      // Keep track of particular combinator
      std::string combName;
      std::map<ref<Expr>, KQueryElem*> parsed;
      

      
    public: 
      // SetCombinator getCombinator() { return comb; }

      /**
       * Initialize symbolic argument parser. 
       * 
       * @param arr symbolic array with given name and size
      */
      void initializeParser(const Array* arr);

      /** 
        * Main method called to parse array declaration. 
        * 
        * @param arr constant array from array delcaration to parse
        * 
        * @return true or false, depending on success of parse
      */ 
      bool parseArrayDecl(const Array *arr);

      /** 
        * Main method called to parse Query
        * 
        * @param query to parse
        * @param fullParser pointer to Hammer Parser, which is modified to be the parser. 
        * fullParser currently broken because can't deal with variable size. 
        * 
        * @return true or false, depending on success of parse
      */ 
      bool parseQueryCommand(const Query &QC, HParser** fullParser);

      /** 
        * Parses one constraint.
        * 
        * @param e Constraint to parse
        * @param width_out Size of element being parsed
        * 
        * @return KQueryElem*, which can be type character, boolean or index
      */
      KQueryElem* parseConstraint(ref<Expr> constraint, int* width_out);

      /** 
        * The main code to parse one constraint. Switches based on kind of constraint.
        * 
        * @param e Constraint to parse
        * @param width_out Size of element being parsed
        * 
        * @return KQueryElem*, which can be type character, boolean or index
      */
      KQueryElem* parseConstraintActual(ref<Expr> constraint, int* width_out);


      KQueryParser();

  };
}


#endif /* KLEE_KQUERYPARSER_H */