#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>

int word_counting_test();
int stop_filter_test();
int build_and_print_index_test();
int save_and_read_index_test();
int merge_and_print_index_file_test();
int search_engine_gen_index_test();
int search_engine_load_and_search_test();
bool files_identical(const std::string& file1, const std::string& file2);