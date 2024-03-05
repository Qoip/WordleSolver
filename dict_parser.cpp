#include <iostream>
#include <fstream>
#include <vector>

struct Word {
  size_t index;
  long double ipm;
  std::string word;
  std::string type;
};

int main() {
  std::ifstream fin("lemma.txt");
  std::vector<Word> words;
  while (fin.peek() != EOF) {
    words.push_back({});
    fin >> words.back().index >> words.back().ipm >> words.back().word >> words.back().type;
  }

  int count = 0;
  for (size_t i = 0; i < words.size(); ++i) {
    if (words[i].type == "noun" && words[i].word.size() > 0) {
      ++count;
      std::cout << words[i].word << '\n';
    }
  }
  std::cout << count;  
}