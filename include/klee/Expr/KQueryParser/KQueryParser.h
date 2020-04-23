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
      Type getType() const { return Index; }
      IndexElem(int index) {elem = index;}
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
      SetCombinator comb;
      std::map<ref<Expr>, KQueryElem*> parsed;

    public: 
      SetCombinator getCombinator() { return comb; }

      void initializeParser(const Array* arr);
      bool parseQueryCommand(const Query &QC, HParser** fullParser);
      KQueryElem* parseConstraint(ref<Expr> constraint, int* width_out);
      KQueryElem* parseConstraintActual(ref<Expr> constraint, int* width_out);


      KQueryParser();

  };
}


#endif /* KLEE_KQUERYPARSER_H */