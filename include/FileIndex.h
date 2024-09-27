#pragma once

#include <map>
#include <vector>
#include <fstream>
#include <string>
#include <cstdint>
#include <iostream>
#include <filesystem>

#include "StopFilter.h"

class FileIndex {
public:
    // Add file content to the index
    void add_file(const std::string& filename, uint32_t id, StopFilter* filter = nullptr);

    // Add a directory to the index
    uint32_t add_dir(const std::string& dir, uint32_t id_start = 0, StopFilter* filter = nullptr);

    // Clear the index
    void clear();

    // Serialize the index to a stream (binary)
    void serialize(std::ostream& output);

    // Serialize the index to a file (binary)
    void save(const std::filesystem::path& filename);

    // Deserialize an index from a stream
    static FileIndex deserialize(std::istream& input);

    // Deserialize an index from a file
    static FileIndex read(const std::string& filename);

    // Print the index
    void print(std::ostream& output);
    void print(const std::string& filename);

    // Print file index from an input stream
    static void print_file(const std::string& filename, std::ostream& output);

    // Merge two index files into one
    static void merge_files(
        const std::string& filename1,
        const std::string& filename2,
        const std::string& output_filename
    );

    // Helper function to read an entry from an input stream
    static bool read_entry(std::istream& input, std::string& word, std::vector<uint32_t>& docs);

    // Helper function to write an entry to an output stream
    static void write_entry(std::ostream& output, const std::string& word, const std::vector<uint32_t>& docs);

    // Helper function to merge two vectors of file IDs
    static std::vector<uint32_t> merge_lists(const std::vector<uint32_t>& files1, const std::vector<uint32_t>& files2);
private:
    std::map<std::string, std::vector<uint32_t>> index;

    struct Entry {
        uint32_t freq;
        std::vector<uint32_t> docs;
    };
};
