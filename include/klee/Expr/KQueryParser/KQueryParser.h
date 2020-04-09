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
        Index, 
        Char
      };
      virtual Type getType() const = 0;
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

  class KQueryParser {
    private: 
      SetCombinator comb;

    public: 
      SetCombinator getCombinator() { return comb; }

      void initializeParser(const Array* arr);
      bool parseQueryCommand(const Query &QC);
      KQueryElem* parseConstraint(ref<Expr> constraint, int* width_out);

      KQueryParser();

  };
}


#endif /* KLEE_KQUERYPARSER_H */