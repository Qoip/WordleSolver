#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>

#pragma GCC opimize("Ofast")

// TODO:
// locale? ok // hell // chcp 1251
// last word guess
// first word always same
// iterate on only good masks
/* mask tree:
    -time on each
    +more is_availables
*/

class WordleSolver {
 private:
  struct Mask {
   private:
    struct LetterConstarains {
      char symbol;
      size_t min = 0;
      size_t max = SIZE_MAX;
    };
    std::vector<char> exact;
    std::vector<char> exactly_not;
    std::vector<LetterConstarains> letters_constarains;

   public:
    Mask(const std::string& query, const std::vector<uint8_t>& result) : exact(result.size(), '\0'), exactly_not(result.size(), '\0') {
      size_t query_size = result.size();
      for (size_t i = 0; i < query_size; ++i) {
        if (result[i] == 2) {
          exact[i] = query[i];
        } else {
          exactly_not[i] = query[i];
        }
      }
      std::vector<std::pair<char, uint8_t>> full_query(query_size);
      for (size_t i = 0; i < query_size; ++i) {
        full_query[i] = std::make_pair(query[i], result[i]);
      }
      std::sort(full_query.begin(), full_query.end(), [](auto& a, auto& b) {
        if (a.first == b.first) {
          return a.second > b.second;
        }
        return a.first < b.first;
      });

      for (size_t i = 0; i < query_size; ++i) {
        if (full_query[i].second > 0) {
          if (letters_constarains.size() > 0 && letters_constarains.back().symbol == full_query[i].first) {
            ++letters_constarains.back().min;
          } else {
            letters_constarains.push_back({.symbol = full_query[i].first, .min = 1});
          }
        } else {
          if (letters_constarains.size() == 0 || letters_constarains.back().symbol != full_query[i].first) {
            letters_constarains.push_back({.symbol = full_query[i].first});
          }
          letters_constarains.back().max = letters_constarains.back().min;
        }
      }
    }

    bool IsSatisfy(std::string& query) {
      size_t query_size = query.size();
      for (size_t i = 0; i < query_size; ++i) { /// make "exact" as list of needed
        if (exact[i] != '\0' && exact[i] != query[i]) {
          return false;
        }
        if (exactly_not[i] == query[i]) {
          return false;
        }
      }
      for (size_t i = 0; i < letters_constarains.size(); ++i) { /// 2 pointers
        size_t count = std::count(query.begin(), query.end(), letters_constarains[i].symbol);
        if (count < letters_constarains[i].min || count > letters_constarains[i].max) {
          return false;
        }
      }
      return true;
    }
  };

  // std::vector<bool> is_available;
  std::vector<std::string> available_words;
  std::vector<std::string> sorted_words;
  std::vector<char> alphabet; /// not needed

  const size_t top_amount = 20; // NextWords

  // std::vector<bool> CopyQueryResolve(const std::string& query, const std::vector<uint8_t>& result) {
  //   std::vector<bool> copy_is_available = is_available;
  //   Mask mask(query, result);
  //   for (size_t i = 0; i < sorted_words.size(); ++i) {
  //     if (copy_is_available[i] && !mask.IsSatisfy(sorted_words[i])) {
  //       copy_is_available[i] = false;
  //     }
  //   }
  //   return copy_is_available;
  // }

  size_t QueryRemoves(const std::string& query, const std::vector<uint8_t>& result) {
    Mask mask(query, result);
    size_t removed = 0;
    for (size_t i = 0; i < available_words.size(); ++i) {
      if (!mask.IsSatisfy(available_words[i])) {
        ++removed;
      }
    }
    return removed;
  }

 public:
  const size_t word_size;
  bool only_satisfying_words = false;

  WordleSolver(size_t word_size, const std::vector<std::string>& words) : word_size(word_size) {
    // filter words
    for (auto& word : words) {
      if (word.size() == word_size) {
        sorted_words.push_back(word);
      }
    }
    std::sort(sorted_words.begin(), sorted_words.end());
    // is_available.resize(sorted_words.size(), true);
    available_words = sorted_words;

    // get alphabet
    for (const auto& word : sorted_words) {
      for (char symbol : word) {
        if (std::find(alphabet.begin(), alphabet.end(), symbol) == alphabet.end()) {
          alphabet.push_back(symbol);
        }
      }
    }
    std::sort(alphabet.begin(), alphabet.end());

    // debug data
    std::cout << sorted_words.size() << " words of length " << word_size << " loaded" << std::endl;
    std::cout << "Alphabet: ";
    for (char elem : alphabet) {
      std::cout << elem << " ";
    }
    std::cout << std::endl;
  }

  void QueryResolve(const std::string& query, const std::vector<uint8_t>& result) {
    /*
      Result format:
        0 - no letter
        1 - letter in and not in place
        2 - letter in place
    */
    Mask mask(query, result);
    std::vector<std::string> new_available_words;
    for (size_t i = 0; i < available_words.size(); ++i) {
      if (mask.IsSatisfy(available_words[i])) {
        new_available_words.push_back(available_words[i]);
      }
    }
    std::swap(available_words, new_available_words);
    std::cout << "Words left: " << available_words.size() << std::endl;
  }

  const std::string& GetNextWord() {
    if (available_words.size() == 1) {
      std::cout << "Guessed word: " << available_words[0] << std::endl;
      return available_words[0];
    }
    std::vector<std::pair<std::string*, long double>> top_words(top_amount, std::make_pair(nullptr, -1));
    std::vector<size_t> unreliable_query_word_indexes;
    auto& words_to_check = (only_satisfying_words ? available_words : sorted_words);
    for (size_t word_index = 0; word_index < words_to_check.size(); ++word_index) {
      if ((word_index + 1) % 100 == 0) { // make percentage
        std::cout << word_index + 1 << "th word checking" << std::endl;
      }
      long double expected_removed_percent = 0;
      for (uint64_t i = 0; i < uint64_t(std::pow(3, word_size) + 0.5); ++i) {
        uint64_t result_value = i;
        std::vector<uint8_t> result(word_size); /// move up -> faster // not matters
        for (size_t digit = 0; digit < word_size; ++digit) {
          result[digit] = result_value % 3;
          result_value /= 3;
        }
        bool skip = false; // validate for same masks ("aa" : 01 <=> 10)
        for (size_t e1 = 0; e1 < word_size; ++e1) {
          for (size_t e2 = e1 + 1; e2 < word_size; ++e2) {
            if (words_to_check[word_index][e1] == words_to_check[word_index][e2] && result[e1] == 0 && result[e2] == 1) {
              skip = true;
            }
          }
        }
        if (skip) {
          continue;
        }
        /// auto new_is_available = CopyQueryResolve(word, result);
        /// size_t removed = words_left - count(new_is_available.begin(), new_is_available.end(), true);
        size_t removed = QueryRemoves(words_to_check[word_index], result);
        if (removed == 0 || removed == available_words.size()) {
          continue;
        }
        long double removed_percent = (long double)removed / available_words.size();
        long double probability = 1 - removed_percent;
        ///  std::cout << removed << " " << words_left << " " << probability << " "; for (auto i : result) std::cout << int(i); std::cout << std::endl;
        /// std::cout << "msk: "; for(auto i : (new Mask(word, result))->letters_constarains) std::cout << i.min << " " << i.max << " " << i.symbol << '\n';
        expected_removed_percent += removed_percent * probability;
      }

      if (expected_removed_percent == 0) {
        unreliable_query_word_indexes.push_back(word_index);
      }

      for (size_t place = 0; place < top_amount; ++place) { // top words refresh
        if (expected_removed_percent > top_words[place].second) {
          for (size_t move = top_amount - 1; move > place; --move) {
            top_words[place].first = top_words[place - 1].first;
            top_words[place].second = top_words[place - 1].second;
          }
          top_words[place].first = &words_to_check[word_index];
          top_words[place].second = expected_removed_percent;
          break;
        }
      }
      // std::cout << "Expected removing rate of \"" << word << "\" is " << expected_removed_percent << '\n';
    }

    std::vector<std::string> new_sorted_words;
    size_t deleting_index = 0;
    for (int i = 0; i < sorted_words.size(); ++i) {
      if (deleting_index >= unreliable_query_word_indexes.size() || i != unreliable_query_word_indexes[deleting_index]) {
        new_sorted_words.push_back(sorted_words[i]);
      } else {
        ++deleting_index;
      }
    }
    std::swap(sorted_words, new_sorted_words);

    // debug data
    if (top_words[0].first == nullptr) {
      std::cout << "No words working" << std::endl;
    } else {
      for (size_t i = 0; i < top_amount; ++i) {
        if ((i > 0 && top_words[i].first == top_words[i - 1].first) || top_words[i].first == nullptr) {
          break;
        }
        std::cout << "Top " << i + 1 << " word is \"" << *top_words[i].first << "\" with expected removing rate " << top_words[i].second
                  << " (" << available_words.size() * top_words[i].second << " words)" << std::endl;
      }
    }
    std::cout << "Erased " << unreliable_query_word_indexes.size() << " query words for inefficiency" << std::endl;
    return *top_words[0].first; /// return all todo
  }

  void Erase(std::string word) {
    bool is_found = false;
    for (size_t i = 0; i < available_words.size(); ++i) {
      if (available_words[i] == word) {
        available_words.erase(available_words.begin() + i);
        is_found = true;
        std::cout << "Word \"" << word << "\" erased from available words" << std::endl;
      }
    }
    for (size_t i = 0; i < sorted_words.size(); ++i) {
      if (sorted_words[i] == word) {
        sorted_words.erase(sorted_words.begin() + i);
        is_found = true;
        std::cout << "Word \"" << word << "\" erased from query wordlist" << std::endl;
      }
    }
    if (!is_found) {
      std::cout << "Word \"" << word << "\" was not found" << std::endl;
    }
  }

  std::vector<std::string> GetAvailable() {
    std::cout << "Available words:" << std::endl;
    for (auto& word : available_words) {
      std::cout << "> " << word << '\n';
    }
    std::cout << available_words.size() << " words total" << std::endl;
    return available_words;
  }
};

WordleSolver WordleFromFile(std::string filename, size_t word_size) {
  std::ifstream fin(filename);
  std::vector<std::string> words;
  std::string line;
  while (std::getline(fin, line)) {
    words.push_back(line);
  }
  WordleSolver solver(word_size, words);
  return solver;
}

int main() {
  WordleSolver solver = WordleFromFile("dict/ru.txt", 5);
  std::string command;
  while (command != "exit") {
    std::cin >> command;

    auto begin = std::chrono::steady_clock::now();
    if (command == "get") {
      solver.GetNextWord();
    } else if (command == "q") {
      std::string query;
      std::cin >> query;
      std::vector<uint8_t> result(solver.word_size);
      for (auto& element : result) {
        char symbol;
        std::cin >> symbol;
        element = symbol - '0';
      }
      solver.QueryResolve(query, result);
    } else if (command == "list") {
      solver.GetAvailable();
    } else if (command == "erase") {
      std::string word;
      std::cin >> word;
      solver.Erase(word);
    }
    auto end = std::chrono::steady_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
    std::cout << "Querying time: " << elapsed_ms.count() / 1000.0 << "s" << std::endl;
  }
}