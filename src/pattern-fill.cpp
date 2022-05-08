
#include "pattern-fill.h"

std::string fillPattern(const std::string &pattern, const Replacer *replacers, int count) {
    std::string str;
    str.reserve(2*pattern.size());
    const char *cur = pattern.c_str();
    const char *end = cur+pattern.size();
    bool closedAngleBracket = false;
    while (cur < end) {
        if (*cur == '$') {
            ++cur;
            for (int i = 0; i < count; ++i) {
                if (replacers[i].key == *cur) {
                    str += replacers[i].value;
                    if (!str.empty() && str.back() == '>')
                        closedAngleBracket = true;
                    break;
                }
            }
        } else {
            if (closedAngleBracket) {
                if (*cur == '>')
                    str.push_back(' ');
                closedAngleBracket = false;
            }
            str.push_back(*cur);
        }
        ++cur;
    }
    return str;
}
