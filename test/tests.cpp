#include <filesystem>
#include <fstream>

#include "tests.h"



int main(int argc, char* argv[]) {
    if (argc == 1) {
        std::cerr << "No test specified.\n\nUseage: tests <test_case_name>\n - see `CMakeList.txt for the test case names`" << std::endl;
        return 1;
    }

    std::filesystem::path testdir = std::filesystem::current_path() / "../test";
    std::filesystem::current_path(testdir);

    std::string testname = argv[1];

    if (testname == "word_count") {
        return word_counting_test();
    }
    else if (testname == "stop_filter") {
        return stop_filter_test();
    }
    else if (testname == "build_and_print_index") {
        return build_and_print_index_test();
    }
    else if (testname == "save_and_read_index") {
        return save_and_read_index_test();
    }
    else if (testname == "merge_and_print_index_file") {
        return merge_and_print_index_file_test();
    }
    else if (testname == "search_engine_gen_index") {
        return search_engine_gen_index_test();
    }
    else if (testname == "search_engine_load_and_search") {
        return search_engine_load_and_search_test();
    }

    std::cerr << "Unknown test: " << testname << std::endl;
    return 1;
}

// Utils
bool files_identical(const std::string& file1, const std::string& file2) {
    std::ifstream f1(file1, std::ios::binary);
    std::ifstream f2(file2, std::ios::binary);
    char ch1, ch2;
    bool has1 = static_cast<bool>(f1.get(ch1));
    bool has2 = static_cast<bool>(f2.get(ch2));
    while (has1 && has2) {
        if (ch1 != ch2) {
            return false;
        }
        has1 = static_cast<bool>(f1.get(ch1));
        has2 = static_cast<bool>(f2.get(ch2));
    }
    return !has1 && !has2;
}