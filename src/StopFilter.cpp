#include "StopFilter.h"

#include <fstream>
#include <iostream>
#include <string>

/**
 * @brief Constructor to load stop words from a file.
 * @param stop_words_file The path to the file containing stop words.
 *
 * This constructor reads the stop words from the specified file and inserts
 * them into the stop_words set for efficient checking.
 */
StopFilter::StopFilter(const std::string& stop_words_file) {
    std::ifstream input(stop_words_file);
    std::string word;

    // Read words from the file and insert them into the stop_words set
    while (input >> word) {
        if (!word.empty()) {
            stop_words.insert(word);
        }
    }
}

/**
 * @brief Check if a word is a stop word.
 * @param word The word to check.
 * @return true if the word is a stop word, false otherwise.
 *
 * This method checks if the word is present in the stop_words set.
 * It also considers words with less than 3 characters as stop words.
 */
bool StopFilter::is_stop(const std::string& word) const {
    if (word.size() < 3) return true; // Treat short words as stop words
    return stop_words.find(word) != stop_words.end(); // Check set for stop word
}

/**
 * @brief Print the stop words set.
 * @param output The output stream to print to.
 * This method prints the stop words set to the specified output stream.
 */
void StopFilter::print(std::ostream& output) const {
    for (const auto& word : stop_words) {
        output << word << std::endl;
    }
}