#include "SearchEngine.h"

#include <fstream>
#include <iostream>
#include <filesystem>

#include "FileIndex.h"
#include "utils.h"

SearchEngine::SearchEngine(
    const std::string& target_dir
) : index_file(index_file)
{
    std::filesystem::path dir(target_dir);

    std::string line;
    std::ifstream list_fs(dir / LIST_FILE_NAME);
    while (std::getline(list_fs, line)) {
        if (line.empty()) continue;
        file_list.push_back(line);
    }
    list_fs.close();

    uint32_t size;
    std::ifstream input(dir / INDEX_FILE_NAME, std::ios::binary);
    input.read(reinterpret_cast<char*>(&size), sizeof(size));
    for (uint32_t i = 0; i < size; i++) {
        std::string word;
        std::vector<uint32_t> docs;
        std::streampos entry_pos = input.tellg();
        FileIndex::read_entry(input, word, docs);
        words.insert({ word, static_cast<Offset>(entry_pos) });
    }
    input.close();
}

void SearchEngine::gen_index(const std::filesystem::path& target_dir) {
    std::filesystem::path prev = std::filesystem::current_path();
    std::filesystem::current_path(target_dir);
    std::vector<std::string> files = get_files(".");
    std::ofstream list_fs(LIST_FILE_NAME);
    for (auto& file : files) {
        list_fs << file << std::endl;
    }
    list_fs.close();

    FileIndex index;
    for (uint32_t i = 0; i < files.size(); i++) {
        index.add_file(files[i], i);
    }
    index.save(INDEX_FILE_NAME);
    std::filesystem::current_path(prev);
}

void SearchEngine::search(const std::string& query, std::ostream& output) const {
    std::string stemmed = stem_word(query);
    auto it = words.find(stemmed);
    if (it == words.end()) {
        output << "The query word is not in the index" << std::endl;
        return;
    }
    Offset entry_pos = it->second;
    std::string word;
    std::vector<uint32_t> docs;
    std::ifstream index_fs(index_file, std::ios::binary);
    index_fs.seekg(entry_pos);
    FileIndex::read_entry(index_fs, word, docs);
    index_fs.close();

    for (uint32_t doc_id : docs) {
        output << file_list[doc_id] << std::endl;
    }
}