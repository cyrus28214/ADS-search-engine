#include <fstream>
#include <cassert>

#include "FileIndex.h"
#include "tests.h"

int build_and_print_index_test() {
    FileIndex index;
    index.add_dir("shakespeare/merchant");
    std::ofstream file("output/build_and_print_index_test.txt");
    index.print(file);
    file.close();
    return 0;
}

int save_and_read_index_test() {
    FileIndex index;
    index.add_dir("shakespeare/cymbeline");
    index.save("output/save_and_read_index_test.dat");
    std::ofstream file1("../");
    index.print("output/save_and_read_index_test1.txt");
    FileIndex index2 = FileIndex::read("output/save_and_read_index_test.dat");
    index2.print("output/save_and_read_index_test2.txt");
    assert(files_identical("output/save_and_read_index_test1.txt", "output/save_and_read_index_test2.txt"));
    return 0;
}

int merge_and_print_index_file_test() {
    std::string prefix = "output/merge_and_print_index_file_test";
    FileIndex index;
    uint32_t id_curr;

    // save index separately
    id_curr = 0;
    id_curr += index.add_dir("shakespeare/richardii", id_curr);
    index.save(prefix + "1.dat");
    index.clear();
    id_curr += index.add_dir("shakespeare/richardiii", id_curr);
    index.save(prefix + "2.dat");
    index.clear();

    // save index together
    id_curr = 0;
    id_curr += index.add_dir("shakespeare/richardii", id_curr);
    id_curr += index.add_dir("shakespeare/richardiii", id_curr);
    index.save(prefix + "_direct.dat");
    index.clear();

    // merge index
    FileIndex::merge_files(prefix + "1.dat", prefix + "2.dat", prefix + "_merged.dat");

    // compare merged index with direct index
    assert(files_identical(prefix + "_direct.dat", prefix + "_merged.dat"));
    return 0;
}