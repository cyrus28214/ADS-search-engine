#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <filesystem>

#define INDEX_FILE_NAME ("ADS_Search_index.dat")
#define LIST_FILE_NAME ("ADS_Search_list.txt")

// Get all files from a directory
std::vector<std::string> get_files(
    const std::filesystem::path& directory,
    const std::string& extension = ".html"
);

// Stem word function
std::string stem_word(const std::string& word);

// Tokenize a string into words
std::string tokenize(std::istream& input);