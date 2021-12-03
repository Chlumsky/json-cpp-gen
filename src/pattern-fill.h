
#pragma once

#include <string>

#define ARRAY_LENGTH(a) (sizeof(a)/sizeof(*(a)))

struct Replacer {
    char key;
    const char *value;
};

std::string fillPattern(const std::string &pattern, const Replacer *replacers, int count);
