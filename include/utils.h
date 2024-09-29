#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <filesystem>

// Define constants for directory and file names
#define BASE_DIR (".ADS_search_engine") ///< Base directory for the search engine, e.g. the index for `target_dir` will be stored in `target_dir/<BASE_DIR>`
#define INDEX_FILE_NAME ("index.dat")   ///< Index file name
#define LIST_FILE_NAME ("list.txt")     ///< List file name
#define STOP_FILE_NAME ("stop_wrods.txt") ///< Stop words file name

/**
 * @brief Get all files from a specified directory with a given extension.
 *
 * This function retrieves all files with the specified extension from
 * the provided directory and its subdirectories.
 *
 * @param directory The path to the directory to search.
 * @param extension The file extension to filter by (default is ".html").
 * @return A vector containing the paths of the found files.
 */
std::vector<std::string> get_files(
    const std::filesystem::path& directory,
    const std::string& extension = ".html"
);

/**
 * @brief Stem a word to its base form.
 *
 * This function takes a word as input and converts it to its
 * lowercase stemmed form.
 * It uses the Porter stemming algorithm to achieve this.
 *
 * @param word The word to stem.
 * @return The stemmed version of the word.
 */
std::string stem_word(const std::string& word);

/**
 * @brief Tokenize a string into words.
 *
 * This function reads input from a stream and extracts a single
 * token (word) while skipping over HTML tags.
 *
 * @param input The input stream to read from.
 * @return The tokenized word in stemmed form.
 */
std::string tokenize(std::istream& input);

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
std::vector<uint32_t> intersect(const std::vector<uint32_t>& vec1, const std::vector<uint32_t>& vec2);