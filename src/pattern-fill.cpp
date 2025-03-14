
#include "pattern-fill.h"

static bool isIndentChar(char c) {
    return c == ' ' || c == '\t';
}

static bool isMultiline(const char *str) {
    for (; *str; ++str) {
        if (*str == '\n')
            return true;
    }
    return false;
}

std::string fillPattern(const std::string &pattern, const Replacer *replacers, int count, const std::string &indent) {
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
                    const char *src = replacers[i].value;
                    // Indent multiline value
                    if (isMultiline(src)) {
                        size_t indentLen = 0;
                        while (indentLen < str.size() && isIndentChar(str[str.size()-indentLen-1]))
                            ++indentLen;
                        if (!(indentLen >= str.size() || str[str.size()-indentLen-1] == '\n'))
                            indentLen = 0;
                        if (indentLen == 0 && !str.empty()) {
                            // Multiline value is in the middle of a line - make it single-line
                            while (*src) {
                                if (src[0] == '\r' && src[1] == '\n')
                                    ++src;
                                if (*src == '\n') {
                                    while (isIndentChar(*++src));
                                    str.push_back(' ');
                                    continue;
                                }
                                str.push_back(*src++);
                            }
                        } else {
                            std::string replacerIndent = indentLen >= str.size() ? indent : str.substr(str.size()-indentLen);
                            for (; *src; ++src) {
                                str.push_back(*src);
                                if (*src == '\n')
                                    str += replacerIndent;
                            }
                        }
                    } else
                        str += src;
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
            if (*cur == '\n')
                str += indent;
        }
        ++cur;
    }
    return str;
}
