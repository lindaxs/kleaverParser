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

namespace klee {
  class KQueryElem {
    public: 
      int elem;
      enum Type {
        Null,
        Index, 
        Char, 
        Bool,
      };
      std::vector<std::pair<std::string, int>> conds;
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
      // Maintain conditions
      Type getType() const { return Index; }
      void addCond(std::pair<std::string, int> c) {
        conds.push_back(c);
      }
      IndexElem(int index) {elem = index;}
      IndexElem(int index, std::pair<std::string, int> c) {
        elem = index;
        conds.push_back(c);
      }
      int evalCond(int ch) {
        for (auto cond : conds) {
          ch = evalCondHelper(ch, cond);
        }
        return ch;  
      }
    
    private: 
      int evalCondHelper(int ch, std::pair<std::string, int> c) {
        if (c.first.compare("add") == 0) {
          return ch + c.second;
        } else if (c.first.compare("mul") == 0) {
          return ch * c.second;
        } else if (c.first.compare("sub") == 0) {
          return ch * c.second;
        } else {
          return ch;
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
      // Keep track of all set combinators by name
      std::map<std::string, SetCombinator> combMap; 
      // Keep track of particular combinator
      std::string combName;
      std::map<ref<Expr>, KQueryElem*> parsed;

      

    public: 
      // SetCombinator getCombinator() { return comb; }

      void initializeParser(const Array* arr);
      bool parseQueryCommand(const Query &QC, HParser** fullParser);
      KQueryElem* parseConstraint(ref<Expr> constraint, int* width_out);
      KQueryElem* parseConstraintActual(ref<Expr> constraint, int* width_out);


      KQueryParser();

  };
}


#endif /* KLEE_KQUERYPARSER_H */