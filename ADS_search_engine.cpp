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
    cout << "  " CLI_NAME " index <target_dir> [-l,--large] [-s,--stop <stop_words_file>]" << endl;
    cout << "  "          " - Large mode can handle more very large amount of data in a limited memory. It will do the merge *on-disk*." << endl;
    cout << "  "          " - Normal mode is faster when memory is big enough." << endl;
    cout << "  "          " - You can pass a stop words file to ignore some words. An example is provided in test/stop_words.txt" << endl;
    cout << "  " CLI_NAME " search <target_dir> [-t,--threshold <threshold>] # No query is passed, start interactive mode" << endl;
    cout << "  " CLI_NAME " search <target_dir> [-q,--query <query>] [-t,--threshold <threshold>]" << endl;
    cout << "  "          " - Threshold is float number from 0.0 to 1.0" << endl;
    cout << "  "          " - When the threshold is passed, only the top <threshold>*100% not frequent input terms will be used to search" << endl;
}

int main(int argc, char* argv[]) {
    std::string output, target_dir;

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
        if (!filesystem::exists(target_dir)) {
            cout << "Error: target directory does not exist" << endl;
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
            filesystem::path output_path(output);
            filesystem::create_directories(output_path.parent_path());
            ofstream output_file(output);
            counter.print(output_file);
            output_file.close();
        }
        return 0;
    }

    // index command
    if (argc >= 3 && strcmp(argv[1], "index") == 0) {
        bool large_mode = false;
        StopFilter* stop_filter = nullptr;
        for (int i = 2; i < argc; i++) {
            if ((strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--large") == 0)) {
                large_mode = true;
            }
            else if ((strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--stop") == 0) && i + 1 < argc) {
                stop_filter = new StopFilter(argv[i + 1]);
                i++;
            }

            else {
                target_dir = argv[i];
            }
        }
        filesystem::path dir(target_dir);
        if (!filesystem::exists(dir)) {
            cout << "Error: target directory does not exist" << endl;
            return 1;
        }
        if (filesystem::exists(dir / BASE_DIR)) {
            cout << "Index exsits. Delete and rebuild? (y/N): " << flush;
            string ans;
            getline(cin, ans);
            if (!ans.empty() && (ans[0] == 'y' || ans[0] == 'Y')) {
                filesystem::remove_all(dir / BASE_DIR);
            }
            else {
                return 0;
            }
        }
        if (large_mode) SearchEngine::gen_index_large(target_dir, stop_filter);
        else SearchEngine::gen_index(target_dir, stop_filter);
        cout << "Index generated" << endl;
        return 0;
    }

    // search command
    if (argc >= 3 && strcmp(argv[1], "search") == 0) {
        string query;
        double threshold;
        for (int i = 2; i < argc; i++) {
            if ((strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--query") == 0)) {
                query = argv[i + 1];
                i++;
            }
            else if ((strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--threshold") == 0) && i + 1 < argc) {
                threshold = atof(argv[i + 1]);
                i++;
            }
            else {
                target_dir = argv[i];
            }
        }
        filesystem::path dir(target_dir);
        if (!filesystem::exists(dir)) {
            cout << "Error: target directory does not exist" << endl;
            return 1;
        }
        if (!filesystem::exists(dir / BASE_DIR)) {
            cout << "Error: no index found, please generate index first" << endl;
            return 1;
        }
        SearchEngine engine(dir);
        if (!query.empty()) {
            engine.search(query, cout, threshold);
            return 0;
        }
        else {
            // interactive mode
            while (true) {
                cout << "Enter query (or '/q' to quit): " << flush;
                string line;
                getline(cin, line);
                if (line.empty() || line == "/q") {
                    break;
                }
                engine.search(line, cout, threshold);
            }
            return 0;
        }
    }

    cout << "Unkown command" << endl;
    print_help();
    return 1;
}