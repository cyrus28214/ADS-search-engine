#include "StopFilter.h"

#include <fstream>
#include <iostream>
#include <string>

StopFilter::StopFilter(const std::string& stop_words_file) {
    std::ifstream input(stop_words_file);
    std::string word;
    while (input >> word) {
        if (!word.empty()) {
            stop_words.insert(word);
        }
    }
}

bool StopFilter::is_stop(const std::string& word) const {
    if (word.size() < 3) return true;
    return stop_words.find(word) != stop_words.end();
}
