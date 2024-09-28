#include <iostream>
#include <cstring>

using namespace std;

#define CLI_NAME "ADS_search_engine"

void print_help() {
    cout << "Usage:" << endl;
    cout << "  " CLI_NAME " help" << endl;
    cout << "  " CLI_NAME " count <input_file> [-o,--output <output_file>]" << endl;
    cout << "  " CLI_NAME " index <target_dir>" << endl;
    cout << "  " CLI_NAME " search <target_dir> # interactive mode" << endl;
    cout << "  " CLI_NAME " search <target_dir> [-s,--search <query>]" << endl;
}

int main(int argc, char* argv[]) {
    std::string input, output, query;
    if (argc == 1) {
        print_help();
    }
    else if (strcmp(argv[1], "help") == 0) {
        print_help();
    }
    else if (strcmp(argv[1], "count") == 0) {

    }
    return 0;
}