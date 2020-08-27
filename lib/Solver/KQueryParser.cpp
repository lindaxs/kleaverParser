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

bool KQueryParser::parseArrayDecl(const Array* arr) {
  arrayName = arr->name;
  arrayMap[arrayName] = arr;
  return true;
}

bool KQueryParser::parseQueryCommand(const Query& query, HParser** fullParser) {
  // Parses every constraint
  for (const ref<Expr> &constraint : query.constraints) {
    int width; 
    KQueryParser::parseConstraint(constraint, &width);
  }
  // Outputs parser for each specified symbolic argument 
  for (auto &combElem : combMap) {
    combElem.second.printChRange();
    *fullParser = combElem.second.outputCombinator();
  }
  // TODO: Have it return false when fails
  // What is a fail?
  return true;
}

KQueryElem* KQueryParser::parseConstraint(ref<Expr> e, int *width_out) {
  if (isa<ConstantExpr>(e)) {
    return parseConstraintActual(e, width_out);
  } else {
    // TODO: Keep track of parsed constraints, so we can 
    // look up in dictionary if we see same line. 

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
      
      return new NullElem();
    }


    /* Comparison: Evaluate which candidate elements satisfy the conditions. */
    case Expr::Eq: {
      EqExpr *ee = cast<EqExpr>(e);
      KQueryElem *left = parseConstraint(ee->left, width_out);
      KQueryElem *right = parseConstraint(ee->right, width_out);

      // Items are NULL if failed to parse or are constraints on 
      // an symbolic argument not listed in Objects.
      if (left->getType() == KQueryElem::Null 
          || right->getType() == KQueryElem::Null) {
        return left;
      }

      // Index on left
      if (left->getType() == KQueryElem::Index) {
        IndexElem* iElem = (IndexElem*) left;
        std::set<char> currSet = iElem->getSet();
        if (right->getType() == KQueryElem::Char) {
          std::set<char> s;
          for (char ch : currSet) {
            if (iElem->evalCond(ch) == right->elem) {
              s.insert(ch);
            }
          }
          iElem->setIndexSet(s);
          combMap[combName].setElement(left->elem, currSet, s);
        } else if (right->getType() == KQueryElem::Bool) {
          if (!right->elem) {
            combMap[combName].complementSet(iElem->elem, currSet);
            iElem->setIndexSet(currSet);
          }
          
        }
        return iElem;
      } // Index on right
      else if (right->getType() == KQueryElem::Index) {
        IndexElem* iElem = (IndexElem*) right;
        std::set<char> currSet = iElem->getSet();
        if (left->getType() == KQueryElem::Char) {
          std::set<char> s;
          for (char ch : currSet) {
            if (iElem->evalCond(ch) == left->elem) {
              s.insert(ch);
            }
          }
          iElem->setIndexSet(s);
          combMap[combName].setElement(iElem->elem, currSet, s);
        } else if (left->getType() == KQueryElem::Bool) {
          if (!left->elem) {
            combMap[combName].complementSet(iElem->elem, currSet);
            iElem->setIndexSet(currSet);
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

      // Items are NULL if failed to parse or are constraints on 
      // an symbolic argument not listed in Objects.
      if (left->getType() == KQueryElem::Null 
          || right->getType() == KQueryElem::Null) {
        return left;
      }

      // Index on left
      if (left->getType() == KQueryElem::Index) {
        IndexElem* iElem = (IndexElem*) left;
        if (right->getType() == KQueryElem::Char) {
          std::set<char> s;
          std::set<char> currSet = iElem->getSet();
          for (char ch : currSet) {
            if (iElem->evalCond(ch) < right->elem) {
              s.insert(ch);
            }
          }
          iElem->setIndexSet(s);
          combMap[combName].setElement(iElem->elem, currSet, s);
        }
        return iElem;
      } // Index on right
      else if (right->getType() == KQueryElem::Index) {
        IndexElem* iElem = (IndexElem*) right;
        if (left->getType() == KQueryElem::Char) {
          std::set<char> s;
          std::set<char> currSet = iElem->getSet();
          for (char ch : currSet) {
            if (iElem->evalCond(ch) > left->elem) {
              s.insert(ch);
            }
          }
          iElem->setIndexSet(s);
          combMap[combName].setElement(iElem->elem, currSet, s);
        } 
        return iElem;
      } 

      return new NullElem();
    }

    case Expr::Sle: {
      SleExpr *se = cast<SleExpr>(e);
      KQueryElem *left = parseConstraint(se->left, width_out);
      KQueryElem *right = parseConstraint(se->right, width_out);

      // Items are NULL if failed to parse or are constraints on 
      // an symbolic argument not listed in Objects.
      if (left->getType() == KQueryElem::Null 
          || right->getType() == KQueryElem::Null) {
        return left;
      }

      // Index on left
      if (left->getType() == KQueryElem::Index) {
        IndexElem* iElem = (IndexElem*) left;
        if (right->getType() == KQueryElem::Char) {
          std::set<char> s;
          std::set<char> currSet = iElem->getSet();
          for (char ch : currSet) {
            if (iElem->evalCond(ch) <= right->elem) {
              s.insert(ch);
            }
          }
          iElem->setIndexSet(s);
          combMap[combName].setElement(iElem->elem, currSet, s);
        }
        return iElem;
      } // Index on right
      else if (right->getType() == KQueryElem::Index) {
        IndexElem* iElem = (IndexElem*) right;
        if (left->getType() == KQueryElem::Char) {
          std::set<char> s;
          std::set<char> currSet = iElem->getSet();
          for (char ch : currSet) {
            if (iElem->evalCond(ch) >= left->elem) {
              s.insert(ch);
            }
          }
          iElem->setIndexSet(s);
          combMap[combName].setElement(right->elem, currSet, s);
        } 
        return iElem;
      } 

      return new NullElem();
    }

    case Expr::Ult: {
      UltExpr *ue = cast<UltExpr>(e);
      KQueryElem *left = parseConstraint(ue->left, width_out);
      KQueryElem *right = parseConstraint(ue->right, width_out);

      // Items are NULL if failed to parse or are constraints on 
      // an symbolic argument not listed in Objects.
      if (left->getType() == KQueryElem::Null 
          || right->getType() == KQueryElem::Null) {
        return left;
      }

      // Index on left
      if (left->getType() == KQueryElem::Index) {
        IndexElem* iElem = (IndexElem*) left;
        if (right->getType() == KQueryElem::Char) {
          std::set<char> s;
          std::set<char> currSet = iElem->getSet();
          for (char ch : currSet) {
            if ((unsigned int) iElem->evalCond(ch) < (unsigned int) right->elem) {
              s.insert(ch);
            }
          }
          iElem->setIndexSet(s);
          combMap[combName].setElement(iElem->elem, currSet, s);
        }
        return iElem;
      } // Index on right
      else if (right->getType() == KQueryElem::Index) {
        IndexElem* iElem = (IndexElem*) right;
        if (left->getType() == KQueryElem::Char) {
          std::set<char> s;
          std::set<char> currSet = iElem->getSet();
          for (char ch : currSet) {
            if ((unsigned int) iElem->evalCond(ch) > (unsigned int) left->elem) {
              s.insert(ch);
            }
          }
          iElem->setIndexSet(s);
          combMap[combName].setElement(iElem->elem, currSet, s);
        } 
        return iElem;
      } 

      return new NullElem();
    }

    case Expr::Ule: {
      UltExpr *ue = cast<UltExpr>(e);
      KQueryElem *left = parseConstraint(ue->left, width_out);
      KQueryElem *right = parseConstraint(ue->right, width_out);

      // Items are NULL if failed to parse or are constraints on 
      // an symbolic argument not listed in Objects.
      if (left->getType() == KQueryElem::Null 
          || right->getType() == KQueryElem::Null) {
        return left;
      }

      // Index on left
      if (left->getType() == KQueryElem::Index) {
        IndexElem* iElem = (IndexElem*) left;
        if (right->getType() == KQueryElem::Char) {
          std::set<char> s;
          std::set<char> currSet = iElem->getSet();
          for (char ch : currSet) {
            if ((unsigned int) iElem->evalCond(ch) <= (unsigned int) right->elem) {
              s.insert(ch);
            }
          }
          iElem->setIndexSet(s);
          combMap[combName].setElement(iElem->elem, currSet, s);
        }
        return iElem;
      } // Index on right
      else if (right->getType() == KQueryElem::Index) {
        IndexElem* iElem = (IndexElem*) right;
        if (left->getType() == KQueryElem::Char) {
          std::set<char> s;
          std::set<char> currSet = iElem->getSet();
          for (char ch : currSet) {
            if ((unsigned int) iElem->evalCond(ch) >= (unsigned int) left->elem) {
              s.insert(ch);
            }
          }
          iElem->setIndexSet(s);
          combMap[combName].setElement(iElem->elem, currSet, s);
        } 
        return iElem;
      } 
      return new NullElem();
    }


    /* Special: Reading arrays */
    case Expr::Read: {
      ReadExpr *re = cast<ReadExpr>(e);
      assert(re && re->updates.root);
      *width_out = re->updates.root->getRange();

      // Symbolic array
      if (combMap.find(re->updates.root->name) != combMap.end()) {
        combName = re->updates.root->name;
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

    
    /* Casting: Cast to spceified number of bits*/ 
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

    /* Arithmetic */ 
    case Expr::Add: {
      AddExpr *ae = cast<AddExpr>(e);
      KQueryElem *left = parseConstraint(ae->left, width_out);
      KQueryElem *right = parseConstraint(ae->right, width_out);

      // Items are NULL if failed to parse or are constraints on 
      // an symbolic argument not listed in Objects.
      if (left->getType() == KQueryElem::Null 
          || right->getType() == KQueryElem::Null) {
        return left;
      }

      if (left->getType() == KQueryElem::Index) {
        IndexElem *iElem = (IndexElem*) left;
        if (right->getType() == KQueryElem::Char) {
          std::cout << "Add " << right->elem << std::endl;
          Cond *cond = new Cond(Cond::Add, right->elem);
          iElem->addCond(cond);
          return iElem;
        }
      }
      else if (right->getType() == KQueryElem::Index) {
        IndexElem* iElem = (IndexElem*) right;
        if (left->getType() == KQueryElem::Char) {
          std::cout << "Add " << left->elem << std::endl;
          Cond *cond = new Cond(Cond::Add, left->elem);
          iElem->addCond(cond);
          return iElem;
        }
      }
    }

    case Expr::Mul: {
      MulExpr *me = cast<MulExpr>(e);
      KQueryElem *left = parseConstraint(me->left, width_out);
      KQueryElem *right = parseConstraint(me->right, width_out);

      // Items are NULL if failed to parse or are constraints on 
      // an symbolic argument not listed in Objects.
      if (left->getType() == KQueryElem::Null 
          || right->getType() == KQueryElem::Null) {
        return left;
      }

      if (left->getType() == KQueryElem::Index) {
        IndexElem *iElem = (IndexElem*) left;
        if (right->getType() == KQueryElem::Char) {
          Cond *cond = new Cond(Cond::Mul, right->elem);
          iElem->addCond(cond);
          return iElem;
        }
      }
      else if (right->getType() == KQueryElem::Index) {
        IndexElem *iElem = (IndexElem*) right;
        if (left->getType() == KQueryElem::Char) {
          Cond *cond = new Cond(Cond::Mul, left->elem);
          iElem->addCond(cond);
          return iElem;
        }
      }
    }
    /* Bitwise operations */ 
    case Expr::And: {
      AndExpr *ae = cast<AndExpr>(e);
      KQueryElem *left = parseConstraint(ae->left, width_out);
      KQueryElem *right = parseConstraint(ae->right, width_out);

      // Items are NULL if failed to parse or are constraints on 
      // an symbolic argument not listed in Objects.
      if (left->getType() == KQueryElem::Null 
          || right->getType() == KQueryElem::Null) {
        return left;
      }

      if (left->getType() == KQueryElem::Index) {
        IndexElem *iElem = (IndexElem*) left;
        if (right->getType() == KQueryElem::Char) {
          Cond *cond = new Cond(Cond::And, right->elem);
          iElem->addCond(cond);
          return iElem;
        }
      }
      else if (right->getType() == KQueryElem::Index) {
        IndexElem* iElem = (IndexElem*) right;
        if (left->getType() == KQueryElem::Char) {
          Cond *cond = new Cond(Cond::And, left->elem);
          iElem->addCond(cond);
          return iElem;
        }
      }
    }
    default: {
      return new NullElem();
      break;
    }
  }
}


KQueryParser::KQueryParser() {;}