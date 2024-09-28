#include <iostream>
#include <cstring>
#include <vector>
#include <fstream>
#include <filesystem>

#include "WordCounter.h"
#include "utils.h"
#include "SearchEngine.h"

using namespace std;

#define CLI_NAME "ADS_search_engine"

/**
 * @brief print help message
 */
void print_help() {
    cout << "This program can count, index and search all the .html files in a dirctory." << endl;
    cout << "Usage:" << endl;
    cout << "  " CLI_NAME " help" << endl;
    cout << "  " CLI_NAME " count <target_dir> [-o,--output <output_file>]" << endl;
    cout << "  " CLI_NAME " index <target_dir>" << endl;
    cout << "  " CLI_NAME " search <target_dir> # interactive mode" << endl;
    cout << "  " CLI_NAME " search <target_dir> [-s,--search <query>]" << endl;
}

int main(int argc, char* argv[]) {
    std::string output, target_dir, query;

    // print help message if no arguments are provided
    if (argc == 1) {
        print_help();
        return 0;
    }

    // print help message if the first argument is "help"
    if (argc == 2 && strcmp(argv[1], "help") == 0) {
        print_help();
        return 0;
    }

    // count command
    if (argc >= 2 && strcmp(argv[1], "count") == 0) {
        for (int i = 2; i < argc; i++) {
            if ((strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) && i + 1 < argc) {
                output = argv[i + 1];
                i++;
            }
            else {
                target_dir = argv[i];
            }
        }
        WordCounter counter;
        if (target_dir.empty()) {
            cout << "Error: no target directory specified" << endl;
            return 1;
        }
        vector<string> files = get_files(target_dir);
        for (const string& file : files) {
            ifstream input(file);
            if (!input.is_open()) {
                cout << "Warning: cannot open file " << file << ", ignored" << endl;
                continue;
            }
            while (input) {
                string word = tokenize(input);
                if (word.empty()) {
                    continue;
                }
                counter.add_word(word);
            }
            input.close();
        }
        if (output.empty()) {
            counter.print(cout);
        }
        else {
            std::filesystem::path output_path(output);
            std::filesystem::create_directories(output_path.parent_path());
            ofstream output_file(output);
            counter.print(output_file);
            output_file.close();
        }
        return 0;
    }

    // index command
    if (argc == 3 && strcmp(argv[1], "index") == 0) {
        target_dir = argv[2];
        SearchEngine::gen_index(target_dir);
        cout << "Index generated" << endl;
        return 0;
    }

    cout << "Unkown command" << endl;
    print_help();
    return 1;
}