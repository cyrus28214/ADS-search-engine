#include "SearchEngine.h"

#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <algorithm>
#include <cstdint>

#include "FileIndex.h"
#include "utils.h"

namespace fs = std::filesystem;

SearchEngine::SearchEngine(const std::filesystem::path& dir) {
    this->dir = dir;
    std::string line;
    std::ifstream list_fs(dir / BASE_DIR / LIST_FILE_NAME);
    while (std::getline(list_fs, line)) {
        if (line.empty()) continue;
        file_list.push_back(line);
    }
    list_fs.close();

    if (fs::exists(dir / BASE_DIR / STOP_FILE_NAME)) {
        this->stop_filter = new StopFilter(dir / BASE_DIR / STOP_FILE_NAME);
    }
    else {
        this->stop_filter = nullptr;
    }

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

void SearchEngine::gen_index(const std::filesystem::path& dir, StopFilter* stop_filter, bool quiet) {
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

    if (stop_filter) {
        std::ofstream stop_fs(base / STOP_FILE_NAME);
        stop_filter->print(stop_fs);
        stop_fs.close();
    }

    FileIndex index;
    for (uint32_t i = 0; i < files.size(); i++) {
        if (!quiet) std::cout << "Indexing " << fs::canonical(files[i]) << std::endl;
        index.add_file(files[i], i, stop_filter);
    }
    index.save(base / INDEX_FILE_NAME);
    fs::current_path(prev);
}

void SearchEngine::gen_index_large(const std::filesystem::path& dir, StopFilter* stop_filter, bool quiet) {
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
        index.add_file(files[i], i, stop_filter);
        std::string name;
        name = std::string("index_part_") + std::to_string(i) + std::string("to") + std::to_string(i) + ".tmp";
        index.save(base / name);
        index.clear();
    }
    merge_index(dir, 0, files.size() - 1, quiet);
    std::string name = std::string("index_part_") + std::to_string(0) + std::string("to") + std::to_string(files.size() - 1) + std::string(".tmp");
    std::filesystem::rename(base / name, base / INDEX_FILE_NAME);
    fs::current_path(prev);
}

void SearchEngine::merge_index(const std::filesystem::path& dir, std::size_t l, std::size_t r, bool quiet) {
    if (l == r) return;
    std::size_t m = (l + r) / 2;
    merge_index(dir, l, m, quiet);
    merge_index(dir, m + 1, r, quiet);
    std::string name1 = std::string("index_part_") + std::to_string(l) + std::string("to") + std::to_string(m) + std::string(".tmp");
    std::string name2 = std::string("index_part_") + std::to_string(m + 1) + std::string("to") + std::to_string(r) + std::string(".tmp");
    std::string name3 = std::string("index_part_") + std::to_string(l) + std::string("to") + std::to_string(r) + std::string(".tmp");
    fs::path base(BASE_DIR);
    FileIndex::merge_files(base / name1, base / name2, base / name3);
    if (!quiet) std::cout << "Merging " << name1 << " and " << name2 << " into " << name3 << std::endl;
    std::filesystem::remove(base / name1);
    std::filesystem::remove(base / name2);
}

void SearchEngine::search(const std::string& query, std::ostream& output, double threshold) const {
    std::stringstream ss(query);
    std::vector<std::string> words;
    std::string token;

    while (ss) {
        token = tokenize(ss);
        if (token.empty()) continue;
        token = stem_word(token);
        std::cout << "TEST2.5" << std::endl;
        if (stop_filter && stop_filter->is_stop(token)) {
            output << "Stop word \"" << token << "\" is ignored." << std::endl;
            continue;
        }
        std::cout << "TEST2.6" << std::endl;
        words.push_back(token);
    }
    std::vector<std::pair<std::string, FileIndex::Entry>> entries;

    for (auto& word : words) {
        FileIndex::Entry entry = search_word(word, output);
        entries.push_back({ word, entry });
    }
    std::sort(entries.begin(), entries.end(), [](
        const std::pair<std::string, FileIndex::Entry>& e1,
        const std::pair<std::string, FileIndex::Entry>& e2
        ) {
            return e1.second.freq < e2.second.freq;
        }); // sort by frequency, ascending
    std::vector<uint32_t> res;
    bool first = true;
    for (std::size_t i = 0; i < entries.size(); i++) {
        auto& entry = entries[i];
        if (i > entries.size() * threshold) {
            output << "\"" << entry.first << "\" is ignored due to threshold." << std::endl;
        }
        else {
            if (first) {
                res = entry.second.docs;
                first = false;
            }
            else {
                res = intersect(res, entry.second.docs);
            }
        }
    }
    for (auto& doc : res) {
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