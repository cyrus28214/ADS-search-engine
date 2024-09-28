# ADS Project 1: Search Engine

This project implements a basic search engine that indexes documents and allows users to perform keyword searches. It includes functionalities for filtering stop words and counting word occurrences.

## Features

- The source code is well commented and documented, over 30%.
- Indexing documents and building a searchable index.
- Stop word filtering to enhance search results.
- Word counting functionality to track occurrences.
- Unit tests for all major components.
- **BONUS**: This program use **on-disk index merging** to avoid excessive memory usage. It can handle large datasets.

## Project Structure

```bash
.
├── ADS_search_engine.cpp       # Main entry point for the search engine application
├── CMakeLists.txt              # CMake configuration file
├── include/                    # Header files
│   ├── FileIndex.h             # Header for file indexing
│   ├── SearchEngine.h          # Header for search engine class
│   ├── StopFilter.h            # Header for filtering stop words
│   ├── WordCounter.h           # Header for counting word frequencies
│   └── utils.h                 # Miscellaneous utility functions
├── src/                        # Source files
│   ├── FileIndex.cpp           # File indexing implementation
│   ├── SearchEngine.cpp        # Search engine implementation
│   ├── StopFilter.cpp          # Stop words filter implementation
│   ├── WordCounter.cpp         # Word counting implementation
│   └── utils.cpp               # Utility functions implementation
├── stmr/                       # External stemming library (third-party)
│   ├── stmr.c                  # Stemmer C source
│   ├── stmr.h                  # Stemmer header
├── test/                       # Test files
│   ├── file_index_test.cpp     # Test for file indexing
│   ├── search_engine_test.cpp  # Test for search engine
│   ├── stop_filter_test.cpp    # Test for stop word filter
│   ├── word_count_test.cpp     # Test for word counting
│   └── tests.cpp               # Main test runner
└── word_count.txt              # Sample text for word counting
```

## Build Instructions

This project uses CMake for building and managing dependencies. The `stmr` library is included as a static library and linked to the main search engine executable.

### Prerequisites

- CMake 3.10 or later
- A C++17-compatible compiler (e.g., GCC, Clang)

### Steps to Build

1. Clone the repository and navigate to the root project directory.

2. Create a build directory:

   ```bash
   mkdir build && cd build
   ```

3. Run CMake to configure the project:

   ```bash
   cmake .. # don't forget the double dots
   ```

4. Build the project:

   ```bash
   make
   ```

5. Run the program:

   ```bash
   ./ADS_search_engine help
   ```

This will produce two executables: `ADS_search_engine` and `tests`.

## Running the Search Engine

After building the project, you can run the search engine by executing the `ADS_search_engine` binary:

```bash
./ADS_search_engine help
```
### Examples

If you are in the `build` directory (`cd build`), you can run the program like this:

1. Count Words:

   ```bash
   ./ADS_search_engine count ../test/shakespeare/allswell # print to stdout
   ./ADS_search_engine count ../test/shakespeare/allswell -o ../example/allswell_count.txt # print to file
   ```

2. Index Files in a Directory:

   ```bash
   ./ADS_search_engine index ../test/shakespeare/macbeth
   ls -a ../test/shakespeare/macbeth # you should see ".ADS_search_engine/" directory, that is the index directory
   ```

## Testing

The project includes various tests to ensure that all components (e.g., word counting, stop word filtering, file indexing) work as expected. To run the tests:

```bash
ctest
```

Alternatively, you can directly execute the `tests` binary:

```bash
./tests <test_case_name>
```

See `CMakeLists.txt` to checkout the test case names.

## Note

When testing, the index will be generated to shakespeare example data. To delete them all, use:

```bash
find test/shakespeare -name ".ADS_search_engine" -type d -exec rm -rf {} + # in linux
```

## Reference List

- The [stmr](https://github.com/wooorm/stmr.c) library is used for word stemming.
- [The Complete Works of William Shakespeare](http://shakespeare.mit.edu/)