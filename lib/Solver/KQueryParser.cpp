#include "klee/Expr/KQueryParser/KQueryParser.h"

#include<cstdio>
#include<iostream>
#include<vector>
#include<set>
#include<algorithm>
#include<math.h>
#include<iterator>

using namespace klee;
void KQueryParser::initializeParser(const Array* arr) {
  // Initialize comb
  std::string name = arr->name;
  int size = arr->size;
  comb = SetCombinator(name, size);
}

bool KQueryParser::parseQueryCommand(const Query& query) {
  for (const ref<Expr> &constraint : query.constraints) {
    int width; 
    KQueryParser::parseConstraint(constraint, &width);
  }
  return true;
}

// KQueryElem::construct(ref<Expr> e, int *width_out) {

// }

KQueryElem* KQueryParser::parseConstraint(ref<Expr> e, int* width_out) {
  switch (e->getKind()) {
  case Expr::Constant: {
    ConstantExpr *CE = cast<ConstantExpr>(e);
    *width_out = CE->getWidth();

    // Coerce to bool if necessary.
    if (*width_out == 1)
      return new BoolElem(CE->isTrue());
    if (*width_out <= 32)
      return new CharElem(CE->getZExtValue(32));
    if (*width_out <= 64)
      return new CharElem(CE->getZExtValue());
    
    return new CharElem(100);
  }
  case Expr::Eq: {
    EqExpr *ee = cast<EqExpr>(e);
    KQueryElem *left = parseConstraint(ee->left, width_out);
    KQueryElem *right = parseConstraint(ee->right, width_out);

    std::cout << left->getType() << std::endl;
    std::cout << right->getType() << std::endl;
    // Index on left
    if (left->getType() == KQueryElem::Index) {
      if (right->getType() == KQueryElem::Char) {
        std::set<char> s {(char) right->elem};
        comb.setElement(left->elem, s);
      } else if (right->getType() == KQueryElem::Bool) {
        if (!right->elem) {
          comb.complementSet(left->elem);
        }
      }
      return left;
    } // Index on right
    else if (right->getType() == KQueryElem::Index) {
      if (left->getType() == KQueryElem::Char) {
        std::set<char> s {(char) left->elem};
        comb.setElement(right->elem, s);
      } else if (left->getType() == KQueryElem::Bool) {
        if (!left->elem) {
          comb.complementSet(right->elem);
        }
      }
      return right;
    } 
    // TODO CHANGE
    return new CharElem(100);
  }
  case Expr::Read: {
    ReadExpr *re = cast<ReadExpr>(e);
    assert(re && re->updates.root);
    *width_out = re->updates.root->getRange();
    KQueryElem *chElem = parseConstraint(re->index, width_out);
    return new IndexElem(chElem->elem);
  }
  default: {
    std::cout << "Unhandled Expr type" << std::endl;
    return new CharElem(100);
    break;
  }
  }
}


KQueryParser::KQueryParser() {;}