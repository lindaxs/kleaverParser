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
  combMap[name] = SetCombinator(name, size);;
}

// TODO: Have it return false when fails
// What is a fail?
bool KQueryParser::parseQueryCommand(const Query& query, HParser** fullParser) {
  for (const ref<Expr> &constraint : query.constraints) {
    int width; 
    KQueryParser::parseConstraint(constraint, &width);
  }
  // comb.printChRange();
  for (auto &combElem : combMap) {

    combElem.second.printChRange();
    *fullParser = combElem.second.outputCombinator();
  }
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
      IndexElem* iElem = (IndexElem*) left;
      if (right->getType() == KQueryElem::Char) {
        std::set<char> s;
        std::set<char> currSet = combMap[combName].getSetList()[left->elem];
        for (char ch : currSet) {
          if (iElem->evalCond(ch) == right->elem) {
            s.insert(ch);
          }
        }
        combMap[combName].setElement(left->elem, s);
      } else if (right->getType() == KQueryElem::Bool) {
        if (!right->elem) {
          combMap[combName].complementSet(left->elem);
        }
      }
      return iElem;
    } // Index on right
    else if (right->getType() == KQueryElem::Index) {
      IndexElem* iElem = (IndexElem*) right;
      if (left->getType() == KQueryElem::Char) {
        std::set<char> s;
        std::set<char> currSet = combMap[combName].getSetList()[iElem->elem];
        for (char ch : currSet) {
          if (iElem->evalCond(ch) == left->elem) {
            s.insert(ch);
          }
        }
        combMap[combName].setElement(iElem->elem, s);
      } else if (left->getType() == KQueryElem::Bool) {
        if (!left->elem) {
          combMap[combName].complementSet(iElem->elem);
        }
      }
      return iElem;
    } 

    return new NullElem();
  }
  case Expr::Slt: {
    SltExpr *se = cast<SltExpr>(e);
    KQueryElem *left = parseConstraint(se->left, width_out);
    KQueryElem *right = parseConstraint(se->right, width_out);

    // Index on left
    if (left->getType() == KQueryElem::Index) {
      IndexElem* iElem = (IndexElem*) left;
      if (right->getType() == KQueryElem::Char) {
        std::set<char> s;
        std::set<char> currSet = combMap[combName].getSetList()[iElem->elem];
        for (char ch : currSet) {
          if (iElem->evalCond(ch) < right->elem) {
            s.insert(ch);
          }
        }
        combMap[combName].setElement(iElem->elem, s);
      }
      return iElem;
    } // Index on right
    else if (right->getType() == KQueryElem::Index) {
      IndexElem* iElem = (IndexElem*) right;
      if (left->getType() == KQueryElem::Char) {
        std::set<char> s;
        std::set<char> currSet = combMap[combName].getSetList()[iElem->elem];
        for (char ch : currSet) {
          if (iElem->evalCond(ch) > left->elem) {
            s.insert(ch);
          }
        }
        combMap[combName].setElement(iElem->elem, s);
      } 
      return iElem;
    } 

    return new NullElem();
  }
  case Expr::Sle: {
    SleExpr *se = cast<SleExpr>(e);
    KQueryElem *left = parseConstraint(se->left, width_out);
    KQueryElem *right = parseConstraint(se->right, width_out);

    // Index on left
    if (left->getType() == KQueryElem::Index) {
      IndexElem* iElem = (IndexElem*) left;
      if (right->getType() == KQueryElem::Char) {
        std::set<char> s;
        std::set<char> currSet = combMap[combName].getSetList()[iElem->elem];
        for (char ch : currSet) {
          if (iElem->evalCond(ch) <= right->elem) {
            s.insert(ch);
          }
        }
        combMap[combName].setElement(iElem->elem, s);
      }
      return iElem;
    } // Index on right
    else if (right->getType() == KQueryElem::Index) {
      IndexElem* iElem = (IndexElem*) right;
      if (left->getType() == KQueryElem::Char) {
        std::set<char> s;
        std::set<char> currSet = combMap[combName].getSetList()[iElem->elem];
        for (char ch : currSet) {
          if (iElem->evalCond(ch) >= left->elem) {
            s.insert(ch);
          }
        }
        combMap[combName].setElement(right->elem, s);
      } 
      return iElem;
    } 

    return new NullElem();
  }
  case Expr::Ult: {
    UltExpr *ue = cast<UltExpr>(e);
    KQueryElem *left = parseConstraint(ue->left, width_out);
    KQueryElem *right = parseConstraint(ue->right, width_out);

    // Index on left
    if (left->getType() == KQueryElem::Index) {
      IndexElem* iElem = (IndexElem*) left;
      if (right->getType() == KQueryElem::Char) {
        std::set<char> s;
        std::set<char> currSet = combMap[combName].getSetList()[iElem->elem];
        for (char ch : currSet) {
          if ((unsigned int) iElem->evalCond(ch) < (unsigned int) right->elem) {
            s.insert(ch);
          }
        }
        combMap[combName].setElement(iElem->elem, s);
      }
      return iElem;
    } // Index on right
    else if (right->getType() == KQueryElem::Index) {
      IndexElem* iElem = (IndexElem*) right;
      if (left->getType() == KQueryElem::Char) {
        std::set<char> s;
        std::set<char> currSet = combMap[combName].getSetList()[iElem->elem];
        for (char ch : currSet) {
          if ((unsigned int) iElem->evalCond(ch) > (unsigned int) left->elem) {
            s.insert(ch);
          }
        }
        combMap[combName].setElement(iElem->elem, s);
      } 
      return iElem;
    } 

    return new NullElem();
  }
  case Expr::Ule: {
    UltExpr *ue = cast<UltExpr>(e);
    KQueryElem *left = parseConstraint(ue->left, width_out);
    KQueryElem *right = parseConstraint(ue->right, width_out);

    // Index on left
    if (left->getType() == KQueryElem::Index) {
      IndexElem* iElem = (IndexElem*) left;
      if (right->getType() == KQueryElem::Char) {
        std::set<char> s;
        std::set<char> currSet = combMap[combName].getSetList()[iElem->elem];
        for (char ch : currSet) {
          if ((unsigned int) iElem->evalCond(ch) <= (unsigned int) right->elem) {
            s.insert(ch);
          }
        }
        combMap[combName].setElement(iElem->elem, s);
      }
      return iElem;
    } // Index on right
    else if (right->getType() == KQueryElem::Index) {
      IndexElem* iElem = (IndexElem*) right;
      if (left->getType() == KQueryElem::Char) {
        std::set<char> s;
        std::set<char> currSet = combMap[combName].getSetList()[iElem->elem];
        for (char ch : currSet) {
          if ((unsigned int) iElem->evalCond(ch) >= (unsigned int) left->elem) {
            s.insert(ch);
          }
        }
        combMap[combName].setElement(iElem->elem, s);
      } 
      return iElem;
    } 
    return new NullElem();
  }
  case Expr::Read: {
    ReadExpr *re = cast<ReadExpr>(e);
    assert(re && re->updates.root);
    *width_out = re->updates.root->getRange();

    if (combMap.find(re->updates.root->name) != combMap.end()) {
      combName = re->updates.root->name;
      // comb = combMap[re->updates.root->name];
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
  // case eMacroKind_ReadLSB: {
    
  // }
  
  // Casting
  case Expr::ZExt: {
    int srcWidth;
    CastExpr *ce = cast<CastExpr>(e);
    return parseConstraint(ce->src, &srcWidth);
  }
  case Expr::SExt: {
    int srcWidth;
    CastExpr *ce = cast<CastExpr>(e);
    return parseConstraint(ce->src, &srcWidth);
  }
  // Arithmetic

  case Expr::Add: {
    AddExpr *ae = cast<AddExpr>(e);
    KQueryElem *left = parseConstraint(ae->left, width_out);
    KQueryElem *right = parseConstraint(ae->right, width_out);

    if (left->getType() == KQueryElem::Index) {
      IndexElem *iElem = (IndexElem*) left;
      if (right->getType() == KQueryElem::Char) {
        std::pair <std::string, int> cond ("add", right->elem);
        iElem->addCond(cond);
        return iElem;
      }
    }
    else if (right->getType() == KQueryElem::Index) {
      IndexElem* iElem = (IndexElem*) right;
      if (left->getType() == KQueryElem::Char) {
        std::pair <std::string, int> cond ("add", left->elem);
        iElem->addCond(cond);
        return iElem;
      }
    }
  }
  case Expr::Mul: {
    MulExpr *me = cast<MulExpr>(e);
    KQueryElem *left = parseConstraint(me->left, width_out);
    KQueryElem *right = parseConstraint(me->right, width_out);

    if (left->getType() == KQueryElem::Index) {
      IndexElem *iElem = (IndexElem*) left;
      if (right->getType() == KQueryElem::Char) {
        std::pair <std::string, int> cond ("mul", right->elem);
        iElem->addCond(cond);
        return iElem;
      }
    }
    else if (right->getType() == KQueryElem::Index) {
      IndexElem *iElem = (IndexElem*) right;
      if (left->getType() == KQueryElem::Char) {
        std::pair <std::string, int> cond ("mul", left->elem);
        iElem->addCond(cond);
        return iElem;
      }
    }
  } 
  default: {
    // std::cout << "Unhandled Expr type" << std::endl;
    return new NullElem();
    break;
  }
  }
}


KQueryParser::KQueryParser() {;}