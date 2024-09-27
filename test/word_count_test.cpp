#include <iostream>
#include <fstream>

#include "WordCounter.h"
#include "utils.h"

using namespace std;

int word_counting_test() {
    WordCounter counter;
    ifstream input("../test/shakespeare/allswell/allswell.1.1.html");
    while (input) {
        string token = tokenize(input);
        if (token.empty()) break;
        counter.add_word(token);
    }
    input.close();
    ofstream output("../test/output/word_count_test.txt");
    counter.print(output);
    return 0;
}