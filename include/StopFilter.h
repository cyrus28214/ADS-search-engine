#pragma once

#include <unordered_set>
#include <string>

class StopFilter {
private:
    std::unordered_set<std::string> stop_words;
public:
    // Constructor to load stop words from a file
    StopFilter(const std::string& stop_words_file);

    // Check if a word is a stop word
    bool is_stop(const std::string& word) const;
};