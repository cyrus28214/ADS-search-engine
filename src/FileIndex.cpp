#include "FileIndex.h"
#include "StopFilter.h"
#include "utils.h"

#include <iostream>
#include <fstream>

using namespace std;

/**
 * @brief Adds the content of a file to the index.
 *
 * This function reads tokens from the specified file and updates the index accordingly.
 * It increments the frequency count of each token and records the document ID in which
 * the token appears.
 *
 * @param filename The name of the file to be added to the index.
 * @param id The unique identifier for the document being indexed.
 * @param filter An optional pointer to a StopFilter instance to filter out stop words.
 */
void FileIndex::add_file(const std::filesystem::path& filename, uint32_t id, StopFilter* filter) {
    ifstream file(filename);
    string token;
    while (file) {
        token = tokenize(file);
        if (token.empty()) {
            continue;
        }
        // Skip stop words
        if (filter && filter->is_stop(token)) {
            continue;
        }
        auto& entry = index[token];
        // Add document ID to the list of documents containing the token
        if (entry.docs.empty() || entry.docs.back() != id) {
            entry.docs.push_back(id);
        }
        entry.freq++;
    }
}

/**
 * @brief Adds all files from a specified directory to the index.
 *
 * This function retrieves all files in the specified directory and adds each file's content
 * to the index using a unique document ID starting from id_start. It supports large datasets
 * by processing files sequentially instead of loading all file contents into memory.
 *
 * @param dir The directory containing files to be indexed.
 * @param id_start The starting unique identifier for documents. Defaults to 0.
 * @param filter An optional pointer to a StopFilter instance to filter out stop words.
 * @return The next available document ID after indexing the files in the directory.
 */
uint32_t FileIndex::add_dir(const std::filesystem::path& dir, uint32_t id_start, StopFilter* filter) {
    vector<string> files = get_files(dir);
    for (uint32_t i = 0; i < files.size(); i++) {
        // Process files sequentially
        this->add_file(files[i], id_start + i, filter);
    }
    // Return the next available document ID
    return static_cast<uint32_t>(files.size()) + id_start;
}

/**
 * @brief Clears the index.
 *
 * This function resets the index by clearing all stored data, preparing the index for
 * new data to be added. It is essential for memory management, especially when handling
 * large datasets.
 */
void FileIndex::clear() {
    index.clear();
}

/**
 * @brief Serializes the index to a binary output stream.
 *
 * This function converts the in-memory index into a binary format for storage.
 * The binary format consists of a size header followed by entries for each word.
 * Each entry includes the frequency of the word, the length of the word,
 * the word itself, the number of documents that contain the word,
 * and the list of document IDs.
 *
 * The binary format of the index is as follows:
 * - uint32_t size: The number of entries in the index.
 * - Entry[] entries: An array of Entry structures, each containing the serialized data for a word.
 * - For the binary format of Entry, see the `write_entry` function.
 *
 * @param output The output stream to write the serialized index to.
 */
void FileIndex::serialize(ostream& output) {
    uint32_t size = static_cast<uint32_t>(index.size()); // Get the size of the index
    output.write(reinterpret_cast<const char*>(&size), sizeof(size)); // Write size header
    for (const auto& [word, entry] : index) {
        write_entry(output, word, entry); // Serialize each entry
    }
}

/**
 * @brief Saves the serialized index to a file.
 *
 * This function opens a binary output file stream for the specified filename
 * and calls the serialize method to write the index to the file.
 * It ensures that the output file is properly closed after writing.
 *
 * @param filename The name of the file where the index will be saved.
 */
void FileIndex::save(const std::filesystem::path& filename) {
    ofstream output(filename, ios::binary); // Open output file in binary mode
    serialize(output); // Serialize the index to the file
    output.close(); // Close the file
}

/**
 * @brief Reads a serialized index from a file.
 *
 * This function opens a binary input file stream for the specified filename,
 * calls the deserialize method to read and reconstruct the index,
 * and ensures the input file is properly closed after reading.
 *
 * @param filename The name of the file from which the index will be read.
 * @return The FileIndex object reconstructed from the file.
 */
FileIndex FileIndex::deserialize(istream& input) {
    FileIndex index;
    uint32_t size; // Variable to store the number of entries in the index
    input.read(reinterpret_cast<char*>(&size), sizeof(size)); // Read size header
    for (uint32_t i = 0; i < size; i++) {
        string word;
        Entry entry;
        read_entry(input, word, entry); // Deserialize each entry
        index.index[word] = entry;
    }
    return index;
}

/**
 * @brief Reads a serialized index from a file.
 *
 * This function opens a binary input file stream for the specified filename,
 * calls the deserialize method to read and reconstruct the index,
 * and ensures the input file is properly closed after reading.
 *
 * @param filename The name of the file from which the index will be read.
 * @return The FileIndex object reconstructed from the file.
 */
FileIndex FileIndex::read(const std::string& filename) {
    FileIndex index;
    ifstream input(filename, ios::binary);
    index = deserialize(input); // Deserialize the index from the file
    input.close();
    return index;
}

/**
 * @brief Prints the index to an output stream.
 * This function iterates over the index and prints each word and its corresponding entry.
 * The entry is printed in the format "word(freq): doc1 doc2 ...", where "word" is the word, "freq" is the frequency of the word, and "doc1 doc2 ..." is the list of document IDs that contain the word.
 * @param output The output stream to which the index will be printed.
 */
void FileIndex::print(ostream& output) {
    for (const auto& [word, entry] : index) {
        print_entry(output, word, entry);
    }
}

/**
 * @overload
 * @brief Prints the index to a file.
 * This function opens a file stream for the specified filename and calls the print method to print the index to the file.
 * It ensures that the file is properly closed after printing.
 * @param filename The name of the file where the index will be printed.
 */
void FileIndex::print(const std::filesystem::path& filename) {
    ofstream output(filename);
    print(output);
    output.close();
}

/**
 * @brief Prints the index to a file.
 * This function opens a file stream for the specified filename and calls the print method to print the index to the file.
 * It ensures that the file is properly closed after printing.
 * @param filename The name of the file where the index will be printed.
 * @param output The output stream to which the index will be printed.
 */
void FileIndex::print_file(const std::string& filename, std::ostream& output) {
    uint32_t size;
    ifstream input(filename, ios::binary);
    input.read(reinterpret_cast<char*>(&size), sizeof(size)); // Read size header
    for (uint32_t i = 0; i < size; i++) {
        string word;
        Entry entry;
        read_entry(input, word, entry); // Deserialize each entry
        output << word << ":";
        for (uint32_t doc : entry.docs) {
            output << " " << doc;
        }
        output << endl;
    }
}

/**
 * @brief Merge two indexes from two streams into one stream.
 * Merge two indexes from two streams into one stream.
 * This function avoid read all the data into memory.
 * So it can handle large files.
 * @param input1 The first input stream.
 * @param input2 The second input stream.
 * @param output The output stream.
 */
void FileIndex::merge_files(
    const std::filesystem::path& filename1,
    const std::filesystem::path& filename2,
    const std::filesystem::path& output_filename
) {
    ifstream input1(filename1, ios::binary);
    ifstream input2(filename2, ios::binary);
    ofstream output(output_filename, ios::binary);

    uint32_t size1, size2; // size of index
    input1.read(reinterpret_cast<char*>(&size1), sizeof(size1));
    input2.read(reinterpret_cast<char*>(&size2), sizeof(size2));

    string word1, word2;
    Entry entry1, entry2;
    if (size1 > 0) { // if index1 is not empty
        read_entry(input1, word1, entry1);
    }
    if (size2 > 0) { // if index2 is not empty
        read_entry(input2, word2, entry2);
    }

    uint32_t size_merged = 0;
    std::streampos pos_size = output.tellp(); // save position for size
    output.write(reinterpret_cast<const char*>(&size_merged), sizeof(size_merged)); // placeholder for size, update later

    while (size1 > 0 || size2 > 0) { // merge until one of the index is empty
        if (size1 > 0 && (size2 == 0 || word1 < word2)) {
            // if index1 is not empty and index2 is empty or word1 < word2, write word1 to output
            write_entry(output, word1, entry1);
            size_merged++;
            read_entry(input1, word1, entry1); // read next word
            size1--;
        }
        else if (size2 > 0 && (size1 == 0 || word2 < word1)) {
            // if index2 is not empty and index1 is empty or word2 < word1, write word2 to output
            write_entry(output, word2, entry2);
            size_merged++;
            read_entry(input2, word2, entry2); // read next word
            size2--;
        }
        else { // both not empty and have same word
            Entry merged = merge_entries(entry1, entry2);
            write_entry(output, word1, merged);
            size_merged++;
            read_entry(input1, word1, entry1); // read next word
            read_entry(input2, word2, entry2); // read next word
            size1--;
            size2--;
        }
    }

    output.seekp(pos_size); // go back to position for size
    output.write(reinterpret_cast<const char*>(&size_merged), sizeof(size_merged)); // update size
}

/**
 * @brief Read an entry from the input stream.
 * Read a word and an entry from the input stream.
 * @param input The input stream.
 * @param word The word of the entry.
 * @param entry The entry to be read.
 */
bool FileIndex::read_entry(istream& input, string& word, Entry& entry) {
    uint32_t word_len; // length of word
    if (!input.read(reinterpret_cast<char*>(&word_len), sizeof(word_len))) {
        return false;
    }

    word.resize(word_len); // resize word to word_len
    input.read(&word[0], word_len);

    uint32_t freq;
    input.read(reinterpret_cast<char*>(&freq), sizeof(freq));
    entry.freq = freq;

    uint32_t num_doc;
    input.read(reinterpret_cast<char*>(&num_doc), sizeof(num_doc)); // read number of docs
    entry.docs.resize(num_doc); // resize docs to num_doc
    input.read(reinterpret_cast<char*>(entry.docs.data()), num_doc * sizeof(uint32_t));
    return true;
}

/**
 * @brief Write an entry to the output stream.
 * Write a word and an entry to the output stream.
 * The binary format is:
 * - word_len (uint32_t): length of word
 * - word (char[word_len]): the word
 * - freq (uint32_t): frequency of the word
 * - num_doc (uint32_t): number of documents
 * - docs (uint32_t[num_doc]): the documents
 */
void FileIndex::write_entry(ostream& output, const string& word, const Entry& entry) {
    // write word
    size_t word_len = word.size();
    output.write(reinterpret_cast<const char*>(&word_len), sizeof(word_len));
    output.write(word.c_str(), word_len);

    // write freq
    uint32_t freq = entry.freq;
    output.write(reinterpret_cast<const char*>(&freq), sizeof(freq));

    // write docs
    size_t num_doc = entry.docs.size();
    output.write(reinterpret_cast<const char*>(&num_doc), sizeof(num_doc));
    output.write(reinterpret_cast<const char*>(entry.docs.data()), num_doc * sizeof(uint32_t));
}

/**
 * @brief Print an entry to the output stream.
 * Print a word and an entry to the output stream.
 * The format is:
 * word(freq): doc1 doc2 ...
 * For example:
 * hello(3): 1 2 3
 * hello is the word, 3 is the frequency, 1 2 3 are the documents.
 *
 * @param output The output stream.
 * @param word The word of the entry.
 * @param entry The entry to be printed.
 */
void FileIndex::print_entry(ostream& output, const string& word, const Entry& entry) {
    output << word << "(" << entry.freq << ")" << ":";
    for (auto& doc : entry.docs) {
        output << " " << doc;
    }
    output << endl;
}

/**
 * @brief Merge two entries.
 * Merge two entries into one.
 * The merged entry has the sum of the frequencies and the union of the documents.
 * The documents are sorted in ascending order.
 * @param entry1 The first entry.
 * @param entry2 The second entry.
 * @return The merged entry.
 */
FileIndex::Entry FileIndex::merge_entries(const Entry& entry1, const Entry& entry2) {
    Entry merged;
    merged.freq = entry1.freq + entry2.freq; // sum of frequencies
    auto& docs1 = entry1.docs;
    auto& docs2 = entry2.docs;
    size_t i = 0, j = 0;
    while (i < docs1.size() || j < docs2.size()) { // merge docs1 and docs2
        if (i < docs1.size() && (j >= docs2.size() || docs1[i] < docs2[j])) {
            // if docs1[i] < docs2[j], add docs1[i] to merged
            merged.docs.push_back(docs1[i]);
            i++;
        }
        else if (j < docs2.size() && (i >= docs1.size() || docs2[j] < docs1[i])) {
            // if docs2[j] < docs1[i], add docs2[j] to merged
            merged.docs.push_back(docs2[j]);
            j++;
        }
        else { // docs1[i] == docs2[j]
            merged.docs.push_back(docs1[i]);
            i++;
            j++;
        }
    }
    return merged;
}