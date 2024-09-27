#include "WordCounter.h"

#include <string>
#include <vector>
#include <algorithm>

void WordCounter::add_word(const std::string& word) {
    word_count[word]++;
}

void WordCounter::print(std::ostream& output) {
    std::vector<Entry> word_list(word_count.begin(), word_count.end());

    sort(word_list.begin(), word_list.end(), [](const Entry& a, const Entry& b) {
        return b.second < a.second;
        }); // sort by frequency in descending order

    for (const auto& entry : word_list) {
        output << entry.first << ": " << entry.second << std::endl;
    }
}