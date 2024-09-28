#pragma once

#include <unordered_set>
#include <string>

/**
 * @class StopFilter
 * @brief A class to filter out stop words from a text.
 *
 * The StopFilter class provides functionality to load a list of stop words
 * from a file and check whether a given word is a stop word.
 */
class StopFilter {
private:
    std::unordered_set<std::string> stop_words; ///< A set to store stop words.

public:
    /**
     * @brief Constructor to load stop words from a file.
     * @param stop_words_file The path to the file containing stop words.
     *
     * This constructor reads the stop words from the specified file and
     * stores them in a set for quick lookup.
     */
    StopFilter(const std::string& stop_words_file);

    /**
     * @brief Check if a word is a stop word.
     * @param word The word to check.
     * @return true if the word is a stop word, false otherwise.
     *
     * This method returns true if the word is found in the stop words set
     * or if the word's length is less than 3 characters.
     */
    bool is_stop(const std::string& word) const;
};
