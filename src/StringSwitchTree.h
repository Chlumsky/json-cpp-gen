
#pragma once

#include <string>
#include <map>
#include <memory>

// Optimizes finding string in a predefined set of strings

struct StringSwitchTree {
    static constexpr int LEAF_NODE_MARKER = -2;
    static constexpr int LENGTH_SWITCH_MARKER = -1;

    int position;
    std::string label;
    std::map<int, std::unique_ptr<StringSwitchTree> > branches;

    static std::unique_ptr<StringSwitchTree> build(const std::string *labels, size_t count);
};
