#include <string>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <filesystem>

#include "FileIndex.h"
#include "StopFilter.h"

class SearchEngine {
public:
    using Offset = uint32_t;

    SearchEngine(const std::filesystem::path& dir);

    void search(const std::string& query, std::ostream& output) const;
    FileIndex::Entry search_word(const std::string& word, std::ostream& output) const;

    static void gen_index(const std::filesystem::path& dir);
private:
    std::filesystem::path dir;
    std::vector<std::string> file_list;
    std::unordered_map <std::string, Offset> words;
};