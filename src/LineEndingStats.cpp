
#include "LineEndingStats.h"

void LineEndingStats::process(const std::string &text) {
    bool prevCr = false;
    for (char c : text) {
        if (c == '\n') {
            if (prevCr)
                ++crlfCount;
            else
                ++lfCount;
        }
        prevCr = c == '\r';
    }
}

Settings::LineEndingStyle LineEndingStats::majorityStyle() const {
    return lfCount >= crlfCount ? Settings::LineEndingStyle::LF : Settings::LineEndingStyle::CRLF;
}
