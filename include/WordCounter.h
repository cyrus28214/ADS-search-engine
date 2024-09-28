#pragma once

#include <string>
#include <unordered_map>
#include <iostream>

/**
 * @class WordCounter
 * @brief A class to count occurrences of words.
 *
 * The WordCounter class provides methods to add words and print their counts.
 */
class WordCounter {
public:
    using Entry = std::pair<std::string, int>; ///< Type alias for a word and its count.

    /**
     * @brief Add a word to the counter.
     * @param word The word to be added to the counter.
     *
     * This method increments the count of the specified word.
     */
    void add_word(const std::string& word);


    /**
     * @brief Print the word count result.
     * @param output The output stream to print the results.
     *
     * This method prints the words and their corresponding counts in
     * descending order of frequency.
     */
    void print(std::ostream& output);
private:
    std::unordered_map<std::string, int> word_count;  ///< A map to store word counts.
};
