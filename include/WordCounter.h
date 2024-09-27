#pragma once

#include <string>
#include <unordered_map>
#include <iostream>

class WordCounter {
public:
    using Entry = std::pair<std::string, int>;

    // Add a word to the counter
    void add_word(const std::string& word);

    // Print the word count result
    void print(std::ostream& output);
private:
    std::unordered_map<std::string, int> word_count;
};
