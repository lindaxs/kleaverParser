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

// TODO: Have it return false when fails
// What is a fail?
bool KQueryParser::parseQueryCommand(const Query& query, HParser** fullParser) {
  for (const ref<Expr> &constraint : query.constraints) {
    int width; 
    KQueryParser::parseConstraint(constraint, &width);
  }
  // comb.printChRange();
  *fullParser = comb.outputCombinator();
  return true;
}

KQueryElem* KQueryParser::parseConstraint(ref<Expr> e, int *width_out) {
  if (isa<ConstantExpr>(e)) {
    return parseConstraintActual(e, width_out);
  } else {
    // if (parsed.find(e) != parsed.end()) {
    //   return parsed.find(e);
    // }
    // parsed.insert(e);
    return parseConstraintActual(e, width_out);
    // map< std::pair<ExprHandle, unsigned> >::iterator it = 
    //   constructed.find(e);
    // if (it!=constructed.end()) {
    //   if (width_out)
    //     *width_out = it->second.second;
    //   return it->second.first;
    // } else {
    //   int width;
    //   if (!width_out) width_out = &width;
    //   ExprHandle res = constructActual(e, width_out);
    //   constructed.insert(std::make_pair(e, std::make_pair(res, *width_out)));
    //   return res;
    // }
  }
}

KQueryElem* KQueryParser::parseConstraintActual(ref<Expr> e, int* width_out) {
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

    // Index on left
    if (left->getType() == KQueryElem::Index) {
      if (right->getType() == KQueryElem::Char) {
        std::set<char> s;
        std::set<char> currSet = comb.getSetList()[left->elem];
        for (char ch : currSet) {
          if (ch == right->elem) {
            s.insert(ch);
          }
        }
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
        std::set<char> s;
        std::set<char> currSet = comb.getSetList()[right->elem];
        for (char ch : currSet) {
          if (ch == left->elem) {
            s.insert(ch);
          }
        }
        comb.setElement(right->elem, s);
      } else if (left->getType() == KQueryElem::Bool) {
        if (!left->elem) {
          comb.complementSet(right->elem);
        }
      }
      return right;
    } 

    return new NullElem();
  }
  case Expr::Slt: {
    SltExpr *se = cast<SltExpr>(e);
    KQueryElem *left = parseConstraint(se->left, width_out);
    KQueryElem *right = parseConstraint(se->right, width_out);

    // Index on left
    if (left->getType() == KQueryElem::Index) {
      if (right->getType() == KQueryElem::Char) {
        std::set<char> s;
        std::set<char> currSet = comb.getSetList()[left->elem];
        for (char ch : currSet) {
          if (ch < right->elem) {
            s.insert(ch);
          }
        }
        comb.setElement(left->elem, s);
      }
      return left;
    } // Index on right
    else if (right->getType() == KQueryElem::Index) {
      if (left->getType() == KQueryElem::Char) {
        std::set<char> s;
        std::set<char> currSet = comb.getSetList()[right->elem];
        for (char ch : currSet) {
          if (ch < left->elem) {
            s.insert(ch);
          }
        }
        comb.setElement(right->elem, s);
      } 
      return right;
    } 

    return new NullElem();
  }
  case Expr::Sle: {
    SleExpr *se = cast<SleExpr>(e);
    KQueryElem *left = parseConstraint(se->left, width_out);
    KQueryElem *right = parseConstraint(se->right, width_out);

    // Index on left
    if (left->getType() == KQueryElem::Index) {
      if (right->getType() == KQueryElem::Char) {
        std::set<char> s;
        std::set<char> currSet = comb.getSetList()[left->elem];
        for (char ch : currSet) {
          if (ch <= right->elem) {
            s.insert(ch);
          }
        }
        comb.setElement(left->elem, s);
      }
      return left;
    } // Index on right
    else if (right->getType() == KQueryElem::Index) {
      if (left->getType() == KQueryElem::Char) {
        std::set<char> s;
        std::set<char> currSet = comb.getSetList()[right->elem];
        for (char ch : currSet) {
          if (ch <= left->elem) {
            s.insert(ch);
          }
        }
        comb.setElement(right->elem, s);
      } 
      return right;
    } 

    return new NullElem();
  }
  case Expr::Read: {
    ReadExpr *re = cast<ReadExpr>(e);
    assert(re && re->updates.root);
    *width_out = re->updates.root->getRange();

    if ((comb.getName()).compare(re->updates.root->name) == 0) {
      KQueryElem *chElem = parseConstraint(re->index, width_out);
      return new IndexElem(chElem->elem);
    } else {
      return new NullElem();
    }
    
  }
  case Expr::Extract: {
    ExtractExpr *ee = cast<ExtractExpr>(e);
    return parseConstraint(ee->expr, width_out);  

    // TODO: add offset  
  }
  
  // Casting
  case Expr::ZExt: {
    int srcWidth;
    CastExpr *ce = cast<CastExpr>(e);
    return parseConstraint(ce->src, &srcWidth);
  }
  default: {
    // std::cout << "Unhandled Expr type" << std::endl;
    return new NullElem();
    break;
  }
  }
}


KQueryParser::KQueryParser() {;}