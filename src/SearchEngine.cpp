#include "SearchEngine.h"

#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>

#include "FileIndex.h"
#include "utils.h"

namespace fs = std::filesystem;

SearchEngine::SearchEngine(const std::filesystem::path& dir)
{
    this->dir = dir;
    std::string line;
    std::ifstream list_fs(dir / BASE_DIR / LIST_FILE_NAME);
    while (std::getline(list_fs, line)) {
        if (line.empty()) continue;
        file_list.push_back(line);
    }
    list_fs.close();

    uint32_t size;
    std::ifstream input(dir / BASE_DIR / INDEX_FILE_NAME, std::ios::binary);
    input.read(reinterpret_cast<char*>(&size), sizeof(size));
    for (uint32_t i = 0; i < size; i++) {
        std::string word;
        FileIndex::Entry entry;
        std::streampos entry_pos = input.tellg();
        FileIndex::read_entry(input, word, entry);
        words.insert({ word, static_cast<Offset>(entry_pos) });
    }
    input.close();
}

void SearchEngine::gen_index(const std::filesystem::path& dir, bool quiet) {
    fs::path prev = fs::current_path();
    fs::current_path(dir);
    fs::create_directory(BASE_DIR);
    fs::path base(BASE_DIR);
    std::vector<std::string> files = get_files(".");
    std::ofstream list_fs(base / LIST_FILE_NAME);
    for (auto& file : files) {
        list_fs << file << std::endl;
    }
    list_fs.close();

    FileIndex index;
    for (uint32_t i = 0; i < files.size(); i++) {
        if (!quiet) std::cout << "Indexing " << fs::canonical(files[i]) << std::endl;
        index.add_file(files[i], i);
    }
    index.save(base / INDEX_FILE_NAME);
    fs::current_path(prev);
}

void SearchEngine::search(const std::string& query, std::ostream& output) const {
    std::stringstream ss(query);
    std::vector<std::string> words;
    std::string token;

    while (ss) {
        token = tokenize(ss);
        if (token.empty()) continue;
        token = stem_word(token);
        words.push_back(token);
    }

    // TODO: handle multiple words

    std::vector<uint32_t> docs;
    for (auto& word : words) {
        FileIndex::Entry entry = search_word(word, output);
        docs = intersect(docs, entry.docs);
    }

    for (auto& doc : docs) {
        output << file_list[doc] << std::endl;
    }
}

FileIndex::Entry SearchEngine::search_word(const std::string& word, std::ostream& output) const {
    auto it = words.find(word);
    if (it == words.end()) return FileIndex::Entry();
    Offset offset = it->second;

    std::ifstream index(dir / BASE_DIR / INDEX_FILE_NAME, std::ios::binary);
    index.seekg(offset);
    FileIndex::Entry entry;
    std::string index_word;
    FileIndex::read_entry(index, index_word, entry);
    index.close();

    return entry;
}