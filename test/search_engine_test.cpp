#include "SearchEngine.h"

#include <filesystem>
#include <cassert>

#include "utils.h"

int search_engine_gen_index_test() {
    std::filesystem::path target("shakespeare/asyoulikeit");

    if (std::filesystem::exists(target / INDEX_FILE_NAME)) {
        std::filesystem::remove(target / INDEX_FILE_NAME) == 0;
    }
    if (std::filesystem::exists(target / LIST_FILE_NAME)) {
        std::filesystem::remove(target / LIST_FILE_NAME) == 0;
    }

    SearchEngine::gen_index(target);
    assert(std::filesystem::exists(target / INDEX_FILE_NAME));
    assert(std::filesystem::exists(target / LIST_FILE_NAME));
    return 0;
}