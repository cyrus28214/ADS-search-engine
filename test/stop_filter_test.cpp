#include <cassert>

#include "StopFilter.h"

int stop_filter_test() {
    StopFilter stop_filter("stop_words.txt");
    assert(stop_filter.is_stop("a"));
    assert(stop_filter.is_stop("the"));
    assert(stop_filter.is_stop("a"));
    assert(!stop_filter.is_stop("hello"));
    assert(!stop_filter.is_stop("world"));
    assert(!stop_filter.is_stop("stop"));
    return 0;
}