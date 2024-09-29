#include "utils.h"

#include <filesystem>
#include <iostream>

extern "C" {
#include "stmr.h" // Include the stemmer header
}

/**
 * @brief Get all files from a specified directory with a given extension.
 *
 * This function retrieves all files with the specified extension from
 * the provided directory and its subdirectories. If the directory
 * does not exist, it prints an error message and returns an empty vector.
 *
 * @param directory The path to the directory to search.
 * @param extension The file extension to filter by.
 * @return A vector containing the paths of the found files.
 */
std::vector<std::string> get_files(
    const std::filesystem::path& directory,
    const std::string& extension
) {
    std::vector<std::string> files;
    if (!std::filesystem::exists(directory)) {
        std::cerr << "目录不存在: " << directory << std::endl; // Directory does not exist
        return {};
    }
    for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == extension) {
            files.push_back(entry.path().string()); // Add file path to vector
        }
    }
    return files;
}

/**
 * @brief Stem a word to its base form.
 *
 * This function takes a word as input, converts it to lowercase,
 * and applies the stemming algorithm.
 *
 * @param word The word to stem.
 * @return The stemmed version of the word.
 */
std::string stem_word(const std::string& word) {
    std::string s = word;
    for (char& p : s) {
        p = tolower(p); // Convert to lowercase
    }
    stem(s.data(), 0, static_cast<int>(s.size() - 1)); // Apply stemming algorithm
    return s;
}

/**
 * @brief Tokenize a string into words.
 *
 * This function reads input from a stream and extracts a single
 * token (word) while skipping over HTML tags. It returns the
 * token in stemmed form.
 *
 * @param input The input stream to read from.
 * @return The tokenized word in stemmed form.
 */
std::string tokenize(std::istream& input) {
    std::string token;
    char ch;

    while (input.get(ch)) {
        if (isalnum(ch)) {
            token += ch; // Start of a token
            break;
        }
        else if (ch == '<') { // Skip HTML tags
            while (input.get(ch)) {
                if (ch == '>') {
                    break; // End of the HTML tag
                }
            }
        }
    }

    while (input.get(ch)) {
        if (!isalnum(ch)) {
            break; // End of token
        }
        token += ch; // Continue adding characters to token
    }

    return stem_word(token); // Return the stemmed token
}

/**
 * @brief Intersect two **aescending** vectors of unsigned 32-bit integers.
 *
 * This function finds the intersection of two **aescending** vectors of unsigned
 * 32-bit integers and returns the result as a new vector.
 *
 * @param vec1 The first vector to intersect, must be **aescending**.
 * @param vec2 The second vector to intersect, must be **aescending**.
 * @return A new aescending vector containing the intersection of vec1 and vec2.
 */
std::vector<uint32_t> intersect(const std::vector<uint32_t>& vec1, const std::vector<uint32_t>& vec2) {
    std::vector<uint32_t> result;
    auto it1 = vec1.begin();
    auto it2 = vec2.begin();

    while (it1 != vec1.end() && it2 != vec2.end()) {
        if (*it1 == *it2) {
            result.push_back(*it1); // Add common element to result
            ++it1;
            ++it2;
        }
        else if (*it1 < *it2) {
            ++it1;
        }
        else {
            ++it2;
        }
    }

    return result;
}