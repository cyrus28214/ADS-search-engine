#include "FileIndex.h"
#include "StopFilter.h"
#include "utils.h"

#include <iostream>
#include <fstream>

using namespace std;

void FileIndex::add_file(const std::string& filename, uint32_t id, StopFilter* filter) {
    ifstream file(filename);
    string token;
    while (file) {
        token = tokenize(file);
        if (token.empty()) {
            break;
        }
        if (filter && filter->is_stop(token)) {
            continue;
        }
        auto& indexed = index[token];
        if (indexed.empty() || indexed.back() != id) {
            indexed.push_back(id);
        }
    }
}

uint32_t FileIndex::add_dir(const std::string& dir, uint32_t id_start, StopFilter* filter) {
    vector<string> files = get_files(dir);
    for (size_t i = 0; i < files.size(); i++) {
        this->add_file(files[i], id_start + i, filter);
    }
    return files.size() + id_start;
}

void FileIndex::clear() {
    index.clear();
}

/**
 * Serialize the index to a stream (binary). The binary format is as follows:
 * SIZE (4 bytes)
 * ENTRY[SIZE]:
 *      WORD_LEN (4 bytes)
 *      WORD[WORD_LEN] (WORD_LEN bytes)
 *      NUM_DOC (4 bytes)
 *      DOC_ID[NUM_FILE] (NUM_FILE * 4 bytes)
 */
void FileIndex::serialize(ostream& output) {
    uint32_t size = index.size();
    output.write(reinterpret_cast<const char*>(&size), sizeof(size));
    for (const auto& [word, docs] : index) {
        write_entry(output, word, docs);
    }
}

void FileIndex::save(const std::filesystem::path& filename) {
    ofstream output(filename, ios::binary);
    serialize(output);
    output.close();
}

FileIndex FileIndex::deserialize(istream& input) {
    FileIndex index;
    uint32_t size;
    input.read(reinterpret_cast<char*>(&size), sizeof(size));
    for (uint32_t i = 0; i < size; i++) {
        string word;
        vector<uint32_t> docs;
        read_entry(input, word, docs);
        index.index[word] = docs;
    }
    return index;
}

FileIndex FileIndex::read(const std::string& filename) {
    FileIndex index;
    ifstream input(filename, ios::binary);
    index = deserialize(input);
    input.close();
    return index;
}

void FileIndex::print(ostream& output) {
    for (const auto& [word, docs] : index) {
        output << word << ":";
        for (uint32_t doc : docs) {
            output << " " << doc;
        }
        output << endl;
    }
}

void FileIndex::print(const std::string& filename) {
    ofstream output(filename);
    print(output);
    output.close();
}

void FileIndex::print_file(const std::string& filename, std::ostream& output) {
    uint32_t size;
    ifstream input(filename, ios::binary);
    input.read(reinterpret_cast<char*>(&size), sizeof(size));
    for (uint32_t i = 0; i < size; i++) {
        string word;
        vector<uint32_t> docs;
        read_entry(input, word, docs);
        output << word << ":";
        for (uint32_t doc : docs) {
            output << " " << doc;
        }
        output << endl;
    }
}

/**
 * Merge two indexes from two streams into one stream.
 * This function avoid read all the data into memory.
 * So it can handle large files.
 */
void FileIndex::merge_files(
    const std::string& filename1,
    const std::string& filename2,
    const std::string& output_filename
) {
    ifstream input1(filename1, ios::binary);
    ifstream input2(filename2, ios::binary);
    ofstream output(output_filename, ios::binary);

    uint32_t size1, size2;
    input1.read(reinterpret_cast<char*>(&size1), sizeof(size1));
    input2.read(reinterpret_cast<char*>(&size2), sizeof(size2));

    uint32_t word_len1, word_len2;
    string word1, word2;
    vector<uint32_t> docs1, docs2;
    if (size1 > 0) {
        read_entry(input1, word1, docs1);
    }
    if (size2 > 0) {
        read_entry(input2, word2, docs2);
    }

    uint32_t size_merged = 0;
    std::streampos pos_size = output.tellp();
    output.write(reinterpret_cast<const char*>(&size_merged), sizeof(size_merged)); // placeholder for size, update later

    while (size1 > 0 || size2 > 0) {
        if (size1 > 0 && (size2 == 0 || word1 < word2)) {
            write_entry(output, word1, docs1);
            size_merged++;
            read_entry(input1, word1, docs1);
            size1--;
        }
        else if (size2 > 0 && (size1 == 0 || word2 < word1)) {
            write_entry(output, word2, docs2);
            size_merged++;
            read_entry(input2, word2, docs2);
            size2--;
        }
        else { // both not empty and have same word
            vector<uint32_t> merged_docs = merge_lists(docs1, docs2);
            write_entry(output, word1, merged_docs);
            size_merged++;
            read_entry(input1, word1, docs1);
            read_entry(input2, word2, docs2);
            size1--;
            size2--;
        }
    }

    output.seekp(pos_size);
    output.write(reinterpret_cast<const char*>(&size_merged), sizeof(size_merged)); // update size
}

bool FileIndex::read_entry(istream& input, string& word, vector<uint32_t>& docs) {
    uint32_t word_len;
    if (!input.read(reinterpret_cast<char*>(&word_len), sizeof(word_len))) {
        return false;
    }

    word.resize(word_len);
    input.read(&word[0], word_len);

    uint32_t num_doc;
    input.read(reinterpret_cast<char*>(&num_doc), sizeof(num_doc));
    docs.resize(num_doc);
    input.read(reinterpret_cast<char*>(docs.data()), num_doc * sizeof(uint32_t));
    return true;
}

void FileIndex::write_entry(ostream& output, const string& word, const vector<uint32_t>& docs) {
    uint32_t word_len = word.size();
    output.write(reinterpret_cast<const char*>(&word_len), sizeof(word_len));
    output.write(word.c_str(), word_len);
    uint32_t num_doc = docs.size();
    output.write(reinterpret_cast<const char*>(&num_doc), sizeof(num_doc));
    output.write(reinterpret_cast<const char*>(docs.data()), num_doc * sizeof(uint32_t));
}

vector<uint32_t> FileIndex::merge_lists(const vector<uint32_t>& files1, const vector<uint32_t>& files2) {
    vector<uint32_t> merged;
    size_t i = 0, j = 0;
    while (i < files1.size() || j < files2.size()) {
        if (i < files1.size() && (j >= files2.size() || files1[i] < files2[j])) {
            merged.push_back(files1[i]);
            i++;
        }
        else if (j < files2.size() && (i >= files1.size() || files2[j] < files1[i])) {
            merged.push_back(files2[j]);
            j++;
        }
        else {
            merged.push_back(files1[i]);
            i++;
            j++;
        }
    }
    return merged;
}