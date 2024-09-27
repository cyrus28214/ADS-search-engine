#include "utils.h"

#include <filesystem>
#include <iostream>

extern "C" {
#include "stmr.h"
}

std::vector<std::string> get_files(
    const std::filesystem::path& directory,
    const std::string& extension
) {
    std::vector<std::string> files;
    if (!std::filesystem::exists(directory)) {
        std::cerr << "目录不存在: " << directory << std::endl;
        return {};
    }
    for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == extension) {
            files.push_back(entry.path().string());
        }
    }
    return files;
}

std::string stem_word(const std::string& word) {
    std::string s = word;
    for (char& p : s) {
        p = tolower(p);
    }
    stem(s.data(), 0, s.size() - 1);
    return s;
}

std::string tokenize(std::istream& input) {
    std::string token;
    char ch;

    while (input.get(ch)) {
        if (isalnum(ch)) {
            token += ch;
            break;
        }
        else if (ch == '<') { // skip html tags
            while (input.get(ch)) {
                if (ch == '>') {
                    break;
                }
            }
        }
    }

    while (input.get(ch)) {
        if (!isalnum(ch)) {
            break;
        }
        token += ch;
    }

    return stem_word(token);
}