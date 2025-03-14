
#pragma once

#include <string>
#include "Configuration.h"

class LineEndingStats {

public:
    void process(const std::string &text);
    Settings::LineEndingStyle majorityStyle() const;

private:
    int lfCount = 0, crlfCount = 0;

};
