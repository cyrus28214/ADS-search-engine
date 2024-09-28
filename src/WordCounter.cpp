#include "WordCounter.h"

#include <string>
#include <vector>
#include <algorithm>

/**
 * @brief Add a word to the counter.
 * @param word The word to be added to the counter.
 *
 * This method increments the count of the specified word.
 */
void WordCounter::add_word(const std::string& word) {
    word_count[word]++;
}


/**
 * @brief Print the word count result.
 * @param output The output stream to print the results.
 *
 * This method prints the words and their corresponding counts in
 * descending order of frequency.
 */
void WordCounter::print(std::ostream& output) {
    std::vector<Entry> word_list(word_count.begin(), word_count.end());

    // Sort the words by frequency in descending order
    std::sort(word_list.begin(), word_list.end(), [](const Entry& a, const Entry& b) {
        return b.second < a.second;
        });

    // Print each word and its count
    for (const auto& entry : word_list) {
        output << entry.first << ": " << entry.second << std::endl;
    }
}