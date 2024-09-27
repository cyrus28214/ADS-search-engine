#include <string>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <filesystem>

#include "StopFilter.h"

class SearchEngine {
public:
    using Offset = uint32_t;

    SearchEngine(const std::string& target_dir);

    void search(const std::string& query, std::ostream& output) const;

    static void gen_index(const std::filesystem::path& target_dir);
private:
    std::string index_file;
    std::vector<std::string> file_list;
    std::unordered_map <std::string, Offset> words;
};