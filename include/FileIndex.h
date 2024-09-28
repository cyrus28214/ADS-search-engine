#pragma once

#include <map>
#include <vector>
#include <fstream>
#include <string>
#include <cstdint>
#include <iostream>
#include <filesystem>

#include "StopFilter.h"

/**
 * @class FileIndex
 * @brief This class implements an inverted file index teached in the ADS course for a mini search engine.
 *
 * The inverted index allows for efficient storage and retrieval of documents based on keywords.
 * It supports adding files and directories, serializing to and from binary files, and **merging
 * multiple indexes without loading all data into memory**, making it suitable for **large datasets**.
 */
class FileIndex {
public:
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
    void add_file(const std::filesystem::path& filename, uint32_t id, StopFilter* filter = nullptr);

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
    uint32_t add_dir(const std::filesystem::path& dir, uint32_t id_start = 0, StopFilter* filter = nullptr);

    /**
     * @brief Clears the index.
     *
     * This function resets the index by clearing all stored data, preparing the index for
     * new data to be added. It is essential for memory management, especially when handling
     * large datasets.
     */
    void clear();

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
    void serialize(std::ostream& output);

    /**
     * @brief Saves the serialized index to a file.
     *
     * This function opens a binary output file stream for the specified filename
     * and calls the serialize method to write the index to the file.
     * It ensures that the output file is properly closed after writing.
     *
     * @param filename The name of the file where the index will be saved.
     */
    void save(const std::filesystem::path& filename);

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
    static FileIndex deserialize(std::istream& input);

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
    static FileIndex read(const std::string& filename);

    /**
     * @brief Prints the index to an output stream.
     * This function iterates over the index and prints each word and its corresponding entry.
     * The entry is printed in the format "word(freq): doc1 doc2 ...", where "word" is the word, "freq" is the frequency of the word, and "doc1 doc2 ..." is the list of document IDs that contain the word.
     * @param output The output stream to which the index will be printed.
     */
    void print(std::ostream& output);

    /**
     * @overload
     * @brief Prints the index to a file.
     * This function opens a file stream for the specified filename and calls the print method to print the index to the file.
     * It ensures that the file is properly closed after printing.
     * @param filename The name of the file where the index will be printed.
     */
    void print(const std::filesystem::path& filename);

    /**
     * @brief Prints the index to a file.
     * This function opens a file stream for the specified filename and calls the print method to print the index to the file.
     * It ensures that the file is properly closed after printing.
     * @param filename The name of the file where the index will be printed.
     * @param output The output stream to which the index will be printed.
     */
    static void print_file(const std::string& filename, std::ostream& output);

    /**
     * @brief Merge two indexes from two streams into one stream.
     * Merge two indexes from two streams into one stream.
     * This function avoid read all the data into memory.
     * So it can handle large files.
     * @param input1 The first input stream.
     * @param input2 The second input stream.
     * @param output The output stream.
     */
    static void merge_files(
        const std::filesystem::path& filename1,
        const std::filesystem::path& filename2,
        const std::filesystem::path& output_filename
    );

    /**
     * @brief Read an entry from the input stream.
     * Read a word and an entry from the input stream.
     * @param input The input stream.
     * @param word The word of the entry.
     * @param entry The entry to be read.
     */
    struct Entry {
        uint32_t freq;
        std::vector<uint32_t> docs;
    };

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
    static bool read_entry(std::istream& input, std::string& word, Entry& entry);

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
    static void write_entry(std::ostream& output, const std::string& word, const Entry& entry);

    /**
     * @brief Merge two entries.
     * Merge two entries into one.
     * The merged entry has the sum of the frequencies and the union of the documents.
     * The documents are sorted in ascending order.
     * @param entry1 The first entry.
     * @param entry2 The second entry.
     * @return The merged entry.
     */
    static void print_entry(std::ostream& output, const std::string& word, const Entry& entry);

    // Helper function to merge two vectors of file IDs
    static Entry merge_entries(const Entry& entry1, const Entry& entry2);
private:
    std::map<std::string, Entry> index; ///< The index of words and their frequencies and documents.
};
