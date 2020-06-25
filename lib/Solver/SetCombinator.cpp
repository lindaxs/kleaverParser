#include "klee/Expr/KQueryParser/SetCombinator.h"

#define MAXVAL 128

// Assumptions: index will always be < parserLength

void SetCombinator::setElement(int index, std::set<char> newSet) {
  // check to see if any value has been 
  setList[index] = newSet;
}

void SetCombinator::unionSets(int index, std::set<char> otherSet) {
  for (char ch : otherSet) {
    setList[index].insert(ch);
  }
}

void SetCombinator::intersectSets(int index, std::set<char> otherSet) {
  std::set<char> intSet;
  for (char ch : otherSet) {
    if (setList[index].find(ch) != setList[index].end()) {
      intSet.insert(ch);
    }
  }
  setList[index] = intSet;
}

void SetCombinator::complementSet(int index) {
  // Return complement of set
  std::set<char> compSet;
  for (int i  = 0; i < MAXVAL; i++) {
    if (setList[index].find(i) == setList[index].end()) {
      compSet.insert((char) i);
    }
  }
  setList[index] = compSet;
}
 
void SetCombinator::printChRange() {
  for (size_t i = 0; i < setList.size(); i++) {
    std::cout << "Position " << i << ':'; 
    bool first = true;
    char beg; 
    char prevCh; 

    for (char ch : setList[i]) {
      if (first) {
        first = false;
        beg = ch;
      } else if ((int) ch - (int) prevCh != 1) {
        if (beg != prevCh) {
          std::cout << (int) beg << '-' << (int) prevCh << std::endl;
        } else {
          std::cout << (int) beg << std::endl;
        } 
        beg = ch;
      }
      prevCh = ch;
    }
    if (beg != prevCh) {
      std::cout << (int) beg << '-' << (int) prevCh << std::endl;
    } else {
      std::cout << (int) beg << std::endl;
    }
  }
}

template <typename HParser, std::size_t N, std::size_t... I>
HParser* combineRangeChoice(HParser* (&data)[N], std::index_sequence<I...>)
{
  if (N == 1) {
    return data[0];
  } else {
    return h_choice(data[I]..., NULL);
  }
} 

template <typename HParser, std::size_t N, std::size_t... I>
HParser* combineRangeSeq(HParser* (&data)[N], std::index_sequence<I...>)
{
  if (N == 1) {
    return data[0];
  } else {
    return h_sequence(data[I]..., NULL); 
  }
} 

template <typename HParser, std::size_t N>
HParser* combineRange(HParser* (&data)[N], bool isChoice=true)
{
  if (isChoice) {
    return combineRangeChoice(data, std::make_index_sequence<N>{});
  } else {
    return combineRangeSeq(data, std::make_index_sequence<N>{});
  }
} 

HParser* outputSeqElem(std::vector<HParser*> combinatorElem, int count) {
  // Copy into array to input into variadic function
  constexpr int size = 2;
  HParser* arr[size];
  // std::array<HParser*, size> arr;
  // std::cout << "unit size " << combinatorElem.size() << std::endl;
  std::copy(combinatorElem.begin(), combinatorElem.end(), arr);

  // std::cout << "times " << count << std::endl;
  // Combine the combinatorElem with option
  HParser* elem = combineRange(arr);

  // Repeat count number of times
  if (count == 1) {
    return elem;
  } else {
    return h_repeat_n(elem, count);
  }
}

std::string combineRangeStr(std::vector<std::string> combinatorElemStr, bool isChoice=true) {
  if (combinatorElemStr.size() == 1) {
    return combinatorElemStr[0];
  } else {
    std::string concatElemStr;

    if (isChoice) {
      concatElemStr = "h_choice(";
    } else {
      concatElemStr = "h_sequence(";
    }
    for (size_t i = 0; i < combinatorElemStr.size(); i++) {
      concatElemStr += combinatorElemStr[i] + ", ";
    }
    concatElemStr += "NULL)";
    return concatElemStr;
  }
  
}

std::string outputSeqElemStr(std::vector<std::string> combinatorElemStr, int count) {
  // Copy into array to input into variadic function
  std::string elem = combineRangeStr(combinatorElemStr);

  // Repeat count number of times
  if (count == 1) {
    return elem;
  } else {
    return "h_repeat_n(" + elem + ", " + std::to_string(count) + ")";
  }
}

HParser* SetCombinator::outputCombinator() {
  int count = 0;
  std::set<char> prevSet = setList[0];
  std::set<char> currSet;

  std::vector<HParser*> fullCombinator;
  std::vector<HParser*> combinatorElem;
  HParser* elemUnion;

  std::vector<std::string> fullCombinatorStr;
  std::vector<std::string> combinatorElemStr;
  std::string elemStr;

  for (size_t i = 0; i < setList.size(); i++) {

    currSet = setList[i];

    if (currSet != prevSet) {
      elemUnion = outputSeqElem(combinatorElem, count);
      elemStr = outputSeqElemStr(combinatorElemStr, count);
      fullCombinator.push_back(elemUnion);
      fullCombinatorStr.push_back(elemStr);
      count = 1;
    } else {  // currSet == prevSet
      count += 1;
      if (i != 0) {
        continue;
      }
    }

    combinatorElem = std::vector<HParser*>();
    combinatorElemStr = std::vector<std::string>();

    /* Form the character set for one element in parser. */
    bool first = true;
    char beg; 
    char prevCh; 
    
    for (char ch : currSet) {
      if (first) {
        first = false;
        beg = ch;
      } else if ((int) ch - (int) prevCh != 1) {
        std::stringstream s;
        if (beg != prevCh) {
          combinatorElem.push_back(h_ch_range(beg, prevCh));
          s << "h_ch_range(" << (int) beg << ", " << (int) prevCh << ")"; 
          combinatorElemStr.push_back(s.str());
          // combinatorElemStr.push_back("h_ch_range(" + beg + ", " + prevCh + ")");
          // std::cout << (int) beg << '-' << (int) prevCh << std::endl;
        } else {
          combinatorElem.push_back(h_ch(beg));
          s << "h_ch(" << (int) beg << ")"; 
          combinatorElemStr.push_back(s.str());
          // std::cout << (int) beg << std::endl;
        } 
        beg = ch;
      }
      prevCh = ch;
    }
    std::stringstream st;
    if (beg != prevCh) {
      combinatorElem.push_back(h_ch_range(beg, prevCh));
      st << "h_ch_range(" << (int) beg << ", " << (int) prevCh << ")"; 
      combinatorElemStr.push_back(st.str());
      // std::cout << (int) beg << '-' << (int) prevCh << std::endl;
    } else {
      combinatorElem.push_back(h_ch(beg));
      st << "h_ch(" << (int) beg << ")"; 
      combinatorElemStr.push_back(st.str());
      // std::cout << (int) beg << std::endl;
    }

    prevSet = currSet;
  }

  // Ending sequence
  // If allow any ch, no need to specify
  // TODO: make optional?
  if (currSet.size() != MAXVAL || combinatorElemStr.size() == 1) { 
    elemUnion = outputSeqElem(combinatorElem, count);
    elemStr = outputSeqElemStr(combinatorElemStr, count);
    fullCombinator.push_back(elemUnion);
    fullCombinatorStr.push_back(elemStr);
  } 
  // else {
  //   elemUnion = outputSeqElem(combinatorElem, 0);
  // }
  

  
  // Copy into array to input into variadic function
  constexpr int size = 1;
  HParser* combArr[size];
  // std::array<HParser*, size> arr;
  // std::cout << "full size " << fullCombinator.size() << std::endl;
  std::copy(fullCombinator.begin(), fullCombinator.end(), combArr);

  HParser *fullParser = combineRange(combArr, false);
  std::string fullParserStr = combineRangeStr(fullCombinatorStr, false);


  /* Output to file */
  std::ofstream outfile;

  std::stringstream fileName;
  fileName << "constraint" << identifier << ".c";
  outfile.open(fileName.str());
  // TODO: change this to be based on the input constraint file name
  outfile << "#include <hammer/hammer.h>" << "\n";
  outfile << "#include <stdio.h>" << "\n\n";
  outfile << "int main(int argc, char *argv[]) {\n";
  outfile << "\tuint8_t input[1024];\n";
  outfile << "\tsize_t inputsize;\n";

  outfile << "\t// Parser for " << identifier << "\n";
  outfile << "\tHParser *fullParser = ";
  outfile << fullParserStr << ";\n\n";

  outfile << "\tinputsize = fread(input, 1, sizeof(input), stdin);" << "\n\n";

  outfile << "\tHParseResult *result = h_parse(fullParser, input, inputsize);\n";
  outfile << "\tif (result) {\n";
  outfile << "\t\tprintf(\"yay!\\n\");\n";
  outfile << "\t} else {\n";
  outfile << "\t\tprintf(\"boo!\\n\");\n";
  outfile << "\t}\n";
  outfile << "}\n";
  outfile.close();

  std::cout << fullParserStr << std::endl;

  return fullParser;
}

SetCombinator::SetCombinator() {
  this->parserLength = 0;
  this->identifier = "";
}

SetCombinator::SetCombinator(std::string identifier, int parserLength) { 
  this->parserLength = parserLength;
  this->identifier = identifier;
  // Initialize to allow all characters
  for (int i = 0; i < parserLength; i++) {
    std::set<char> fullSet;
    for (int j = 0; j < MAXVAL; j++) {
      fullSet.insert((char) j); 
    } 
    setList.push_back(fullSet);
  }
}