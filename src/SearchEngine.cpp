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

/**
 * @brief Construct a new Search Engine:: Search Engine object
 * @param dir The target directory to search in.
 *
 * This directory should contain a index folder built using SearchEngine::gen_index(_large).
 * The index folder's name is specified by macro BASE_DIR in utils.h.
 */
SearchEngine::SearchEngine(const std::filesystem::path& dir) {
    this->dir = dir;
    std::string line;
    std::ifstream list_fs(dir / BASE_DIR / LIST_FILE_NAME); // this file contains a list of indexed files
    while (std::getline(list_fs, line)) {
        if (line.empty()) continue;
        file_list.push_back(line);
    }
    list_fs.close();

    if (fs::exists(dir / BASE_DIR / STOP_FILE_NAME)) {
        this->stop_filter = new StopFilter(dir / BASE_DIR / STOP_FILE_NAME); // load stop words list from file
    }
    else {
        this->stop_filter = nullptr; // fix bug on 9.29, if not initialized to nullptr, it will crash
    }

    uint32_t size;
    std::ifstream input(dir / BASE_DIR / INDEX_FILE_NAME, std::ios::binary);
    input.read(reinterpret_cast<char*>(&size), sizeof(size)); // read the number of indexed words
    for (uint32_t i = 0; i < size; i++) {
        std::string word;
        FileIndex::Entry entry;
        std::streampos entry_pos = input.tellg(); // record the offset of the entry in the index file
        FileIndex::read_entry(input, word, entry);
        words.insert({ word, static_cast<Offset>(entry_pos) }); // insert the word and its offset into the map
    }
    input.close();
}

/**
 * @brief Generate an index for the target directory.
 * @param dir The target directory to index.
 * @param stop_filter The stop filter to use. nullptr if no stop filter is needed.
 * @param quiet If true, do not print any output to stdout.
 */
void SearchEngine::gen_index(const std::filesystem::path& dir, StopFilter* stop_filter, bool quiet) {
    fs::path prev = fs::current_path(); // store the current working directory
    fs::current_path(dir); // change to the target directory
    fs::create_directory(BASE_DIR); // make sure the index folder exists
    fs::path base(BASE_DIR);
    std::vector<std::string> files = get_files("."); // get all files in the directory
    std::ofstream list_fs(base / LIST_FILE_NAME);
    for (auto& file : files) {
        list_fs << file << std::endl;
    }
    list_fs.close();

    if (stop_filter) {
        std::ofstream stop_fs(base / STOP_FILE_NAME);
        stop_filter->print(stop_fs); // print stop words list to file
        stop_fs.close();
    }

    FileIndex index;
    for (uint32_t i = 0; i < files.size(); i++) {
        if (!quiet) std::cout << "Indexing " << fs::canonical(files[i]) << std::endl;
        // canonical() returns the absolute path of the file. For prettier printing.
        index.add_file(files[i], i, stop_filter);
    }
    index.save(base / INDEX_FILE_NAME); // save the index to file
    fs::current_path(prev); // return to the original directory
}

/**
 * @brief BNUNS: Generate an index for the target directory, but do most operations on dick to prevent running out of memory.
 * @param dir The target directory to index.
 * @param stop_filter The stop filter to use. nullptr if no stop filter is needed.
 * @param quiet If true, do not print any output to stdout.
 */
void SearchEngine::gen_index_large(const std::filesystem::path& dir, StopFilter* stop_filter, bool quiet) {
    fs::path prev = fs::current_path(); // store the current working directory
    fs::current_path(dir); // change to the target directory
    fs::create_directory(BASE_DIR); // make sure the index folder exists
    fs::path base(BASE_DIR);
    std::vector<std::string> files = get_files("."); // get all files in the directory
    std::ofstream list_fs(base / LIST_FILE_NAME); // write file list to file
    for (auto& file : files) {
        list_fs << file << std::endl;
    }
    list_fs.close();

    FileIndex index;
    for (uint32_t i = 0; i < files.size(); i++) {
        if (!quiet) std::cout << "Indexing " << fs::canonical(files[i]) << std::endl;
        // canonical() returns the absolute path of the file. For prettier printing.
        index.add_file(files[i], i, stop_filter); // add file to index
        std::string name;
        name = std::string("index_part_") + std::to_string(i) + std::string("to") + std::to_string(i) + ".tmp"; // generate file name
        // e.g. index_part_3to3.tmp
        index.save(base / name);
        index.clear();
    }
    merge_index(dir, 0, files.size() - 1, quiet); // merge all the .tmp files *on disk*
    std::string name = std::string("index_part_") + std::to_string(0) + std::string("to") + std::to_string(files.size() - 1) + std::string(".tmp"); // generate file name
    std::filesystem::rename(base / name, base / INDEX_FILE_NAME); // rename the merged file to index
    fs::current_path(prev); // return to the original directory
}

/**
 * @brief Merge the index files generated by gen_index_large.
 * @param dir The target directory to index.
 * @param l The left index of the range to merge.
 * @param r The right index of the range to merge.
 * @param quiet If true, do not print any output to stdout.
 *
 * Merge a series of index file to one. The algorithm is similar to merge sort.
 * It uses recursion to split the range into two halves and merge them.
 */
void SearchEngine::merge_index(const std::filesystem::path& dir, std::size_t l, std::size_t r, bool quiet) {
    if (l == r) return;
    std::size_t m = (l + r) / 2; // find the middle index
    merge_index(dir, l, m, quiet); // merge the left half
    merge_index(dir, m + 1, r, quiet); // merge the right half
    std::string name1 = std::string("index_part_") + std::to_string(l) + std::string("to") + std::to_string(m) + std::string(".tmp");
    std::string name2 = std::string("index_part_") + std::to_string(m + 1) + std::string("to") + std::to_string(r) + std::string(".tmp");
    std::string name3 = std::string("index_part_") + std::to_string(l) + std::string("to") + std::to_string(r) + std::string(".tmp");
    fs::path base(BASE_DIR);
    FileIndex::merge_files(base / name1, base / name2, base / name3); // do the actual merge
    if (!quiet) std::cout << "Merging " << name1 << " and " << name2 << " into " << name3 << std::endl; // print the merge operation
    std::filesystem::remove(base / name1); // remove the temporary files
    std::filesystem::remove(base / name2); // remove the temporary files
}

/**
 * @brief Search for a word in the index.
 * @param word The word to search for.
 * @param output The output stream to write the result to.
 * @param threshold The threshold for the search result.
 *
 * Threshold is a ratio from 0.0 to 1.0. It represents the percentage of terms that should be used in searching. Default value is 1.0
 * For example, if threshold is 0.8, only the top 80% less frequent terms will be used in searching.
 */
void SearchEngine::search(const std::string& query, std::ostream& output, double threshold) const {
    std::stringstream ss(query);
    std::vector<std::string> words;
    std::string token;

    while (ss) {
        token = tokenize(ss); // tokenize the query
        if (token.empty()) continue; // ignore empty tokens
        token = stem_word(token); // stem the word
        if (stop_filter && stop_filter->is_stop(token)) { // if stop word filter is enabled, ignore stop words
            output << "Stop word \"" << token << "\" is ignored." << std::endl;
            continue;
        }
        words.push_back(token);
    }
    std::vector<std::pair<std::string, FileIndex::Entry>> entries;

    // search each word separetely and then intersect the results
    for (auto& word : words) {
        FileIndex::Entry entry = search_word(word, output); //  search the word
        entries.push_back({ word, entry });
    }
    std::sort(entries.begin(), entries.end(), [](
        const std::pair<std::string, FileIndex::Entry>& e1,
        const std::pair<std::string, FileIndex::Entry>& e2
        ) {
            return e1.second.freq < e2.second.freq;
        }); // sort by frequency, *ascending*. It is for the querry thresholding pollicy
    std::vector<uint32_t> res;
    bool first = true; // true if it is processing the first set of results.
    for (std::size_t i = 0; i < entries.size(); i++) {
        auto& entry = entries[i];
        if (i > entries.size() * threshold) { // if the threshold is reached, ignore the rest of the words
            output << "\"" << entry.first << "\" is ignored due to threshold." << std::endl;
        }
        else {
            if (first) { // if it is the first set of results, just assign it to the result
                res = entry.second.docs;
                first = false; // set the flag to false
            }
            else {
                res = intersect(res, entry.second.docs); // intersect the results
            }
        }
    }
    if (res.empty()) { // if the result is empty, print "No results found."
        output << "No results found." << std::endl;
    }
    else for (auto& doc : res) {
        output << file_list[doc] << std::endl; // print the result
    }
}

/**
 * @brief Search for a word in the index.
 * @param word The word to search for.
 * @param output The output stream to write the result to.
 * @return The entry of the word in the index. Retrived from the file.
 */
FileIndex::Entry SearchEngine::search_word(const std::string& word, std::ostream& output) const {
    auto it = words.find(word); // find the word in the index
    if (it == words.end()) return FileIndex::Entry(); // if the word is not found, return an empty entry
    Offset offset = it->second; // get the offset of the word in the index

    std::ifstream index(dir / BASE_DIR / INDEX_FILE_NAME, std::ios::binary);
    index.seekg(offset); // seek to the offset of the word in the index
    FileIndex::Entry entry; // create an entry to store the result
    std::string index_word; // create a string to store the word in the index
    FileIndex::read_entry(index, index_word, entry); // read the entry from the index
    index.close();

    return entry;
}