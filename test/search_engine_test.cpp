#include "SearchEngine.h"

#include <filesystem>
#include <cassert>

#include "utils.h"

namespace fs = std::filesystem;

int search_engine_gen_index_test() {
    fs::path dir = fs::current_path() / "shakespeare/asyoulikeit";

    if (fs::exists(dir / BASE_DIR)) {
        fs::remove_all(dir / BASE_DIR);
    }

    SearchEngine::gen_index(dir, nullptr, true);
    assert(fs::exists(dir / BASE_DIR / INDEX_FILE_NAME));
    assert(fs::exists(dir / BASE_DIR / LIST_FILE_NAME));
    return 0;
}

int search_engine_load_and_search_test() {
    fs::path dir = fs::current_path() / "shakespeare/asyoulikeit";
    SearchEngine se(dir);
    std::ofstream output(fs::current_path() / "output/search_engine_load_and_search.txt");
    output << "searching for 'allow'..." << std::endl;
    se.search("allow", output);
    output << "searching for 'love'..." << std::endl;
    se.search("love", output);
    return 0;
}