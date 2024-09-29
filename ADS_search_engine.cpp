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
 * @brief Print help message
 */
void print_help() {
    cout << "This program can count, index, and search all the .html files in a directory." << endl;
    cout << "Usage:" << endl;
    cout << "  " CLI_NAME " help" << endl;
    cout << "  " CLI_NAME " count <target_dir> [-o,--output <output_file>]" << endl;
    cout << "  " CLI_NAME " index <target_dir> [-l,--large] [-s,--stop <stop_words_file>]" << endl;
    cout << "  "          " - Large mode can handle larger amounts of data, performing merges on-disk." << endl;
    cout << "  "          " - Normal mode is faster when enough memory is available." << endl;
    cout << "  "          " - You can pass a stop words file to ignore certain words. An example is provided in test/stop_words.txt." << endl;
    cout << "  " CLI_NAME " search <target_dir> [-t,--threshold <threshold>] # Start interactive mode if no query is passed." << endl;
    cout << "  " CLI_NAME " search <target_dir> [-q,--query <query>] [-t,--threshold <threshold>]" << endl;
    cout << "  "          " - Threshold is a float number from 0.0 to 1.0." << endl;
    cout << "  "          " - When the threshold is passed, only the top <threshold>*100% of infrequent input terms will be used to search." << endl;
}

/**
 * @brief Main function for CLI
 * @param argc Number of arguments
 * @param argv Argument array
 * @return 0 on success, 1 on error
 */
int main(int argc, char* argv[]) {
    std::string output, target_dir;

    // Print help message if no arguments are provided
    if (argc == 1) {
        print_help();
        return 0;
    }

    // Print help message if the first argument is "help"
    if (argc == 2 && strcmp(argv[1], "help") == 0) {
        print_help();
        return 0;
    }

    // Handle count command
    if (argc >= 2 && strcmp(argv[1], "count") == 0) {
        for (int i = 2; i < argc; i++) {
            // If input is -o or --output, get the output path
            if ((strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) && i + 1 < argc) {
                output = argv[i + 1]; // Get from the next argument
                i++; // Skip the next argument
            }
            else {
                target_dir = argv[i]; // Treat others as target directory
            }
        }

        WordCounter counter; // Create a WordCounter object

        // Print error if no target directory specified
        if (target_dir.empty()) {
            cout << "Error: No target directory specified" << endl;
            return 1;
        }

        // Print error if target directory does not exist
        if (!filesystem::exists(target_dir)) {
            cout << "Error: Target directory does not exist" << endl;
            return 1;
        }

        vector<string> files = get_files(target_dir); // Get all .html files in the target directory
        for (const string& file : files) { // Iterate over all files
            ifstream input(file); // Open the file
            if (!input.is_open()) { // Handle error if file cannot be opened
                cout << "Warning: Cannot open file " << file << ", ignored" << endl;
                continue;
            }
            while (input) { // Iterate over all words in the file
                string word = tokenize(input);
                if (word.empty()) { // Handle empty word
                    continue;
                }
                counter.add_word(word); // Add word to the counter
            }
            input.close(); // Close the file
        }

        // Print results based on whether an output file is specified
        if (output.empty()) {
            counter.print(cout); // Print to console
        }
        else {
            filesystem::path output_path(output);
            filesystem::create_directories(output_path.parent_path()); // Create output directory
            ofstream output_file(output); // Open output file
            counter.print(output_file); // Print to file
            output_file.close(); // Close output file
        }
        return 0;
    }

    // Handle index command
    if (argc >= 3 && strcmp(argv[1], "index") == 0) {
        bool large_mode = false; // Default to not using large mode
        StopFilter* stop_filter = nullptr; // Pointer for stop word filter
        for (int i = 2; i < argc; i++) {
            if ((strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--large") == 0)) {
                large_mode = true; // Set to large mode
            }
            else if ((strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--stop") == 0) && i + 1 < argc) {
                stop_filter = new StopFilter(argv[i + 1]); // Create stop word filter
                i++;
            }
            else {
                target_dir = argv[i]; // Treat others as target directory
            }
        }

        filesystem::path dir(target_dir);
        if (!filesystem::exists(dir)) { // Check if target directory exists
            cout << "Error: Target directory does not exist" << endl;
            return 1;
        }

        // Check if index already exists
        if (filesystem::exists(dir / BASE_DIR)) {
            cout << "Index exists. Delete and rebuild? (y/N): " << flush;
            string ans;
            getline(cin, ans);
            if (!ans.empty() && (ans[0] == 'y' || ans[0] == 'Y')) {
                filesystem::remove_all(dir / BASE_DIR); // Remove existing index
            }
            else {
                return 0; // User chose not to rebuild
            }
        }

        // Generate index based on mode
        if (large_mode) SearchEngine::gen_index_large(target_dir, stop_filter);
        else SearchEngine::gen_index(target_dir, stop_filter);
        cout << "Index generated" << endl;
        return 0;
    }

    // Handle search command
    if (argc >= 3 && strcmp(argv[1], "search") == 0) {
        string query; // Store query string
        double threshold = 1.0; // Default threshold is 1.0
        for (int i = 2; i < argc; i++) {
            if ((strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--query") == 0)) {
                query = argv[i + 1]; // Get query string
                i++;
            }
            else if ((strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--threshold") == 0) && i + 1 < argc) {
                threshold = atof(argv[i + 1]); // Get threshold value
                i++;
            }
            else {
                target_dir = argv[i]; // Treat others as target directory
            }
        }

        filesystem::path dir(target_dir);
        if (!filesystem::exists(dir)) { // Check if target directory exists
            cout << "Error: Target directory does not exist" << endl;
            return 1;
        }

        // Check if index exists
        if (!filesystem::exists(dir / BASE_DIR)) {
            cout << "Error: No index found, please generate index first" << endl;
            return 1;
        }

        SearchEngine engine(dir); // Create SearchEngine object
        if (!query.empty()) {
            engine.search(query, cout, threshold); // Search based on query string
            return 0;
        }
        else {
            // Interactive mode
            while (true) {
                cout << "Enter query (or '/q' to quit): " << flush;
                string line;
                getline(cin, line);
                if (line.empty() || line == "/q") {
                    break; // User chose to exit
                }
                engine.search(line, cout, threshold); // Perform search
            }
            return 0;
        }
    }

    cout << "Unknown command" << endl; // Handle unknown command
    print_help(); // Print help message
    return 1; // Return error code
}
