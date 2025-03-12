
#include "NameFormat.h"

#include <cctype>

static bool isAlphanumeric(char c) {
    return (c&0x80) || isalnum(c);
}

std::string formatName(const std::string &name, NameFormat format) {
    std::string formatted;
    formatted.reserve(name.size());
    bool prevSpace = false;
    char separator = '-';
    switch (format) {
        case NameFormat::ANY:
            formatted = name;
            break;
        case NameFormat::UPPERCASE_UNDERSCORE:
            separator = '_';
            // fallthrough
        case NameFormat::UPERCASE_DASH:
            for (char c : name) {
                if (!isAlphanumeric(c)) {
                    if (!prevSpace)
                        formatted.push_back(separator);
                    prevSpace = true;
                } else {
                    if (!(c&0x80) && islower(c))
                        c = (char) toupper(c);
                    formatted.push_back(c);
                    prevSpace = false;
                }
            }
            break;
        case NameFormat::LOWERCASE_UNDERSCORE:
            separator = '_';
            // fallthrough
        case NameFormat::LOWERCASE_DASH:
            for (char c : name) {
                if (!isAlphanumeric(c)) {
                    if (!prevSpace)
                        formatted.push_back(separator);
                    prevSpace = true;
                } else {
                    if (!(c&0x80) && isupper(c))
                        c = (char) tolower(c);
                    formatted.push_back(c);
                    prevSpace = false;
                }
            }
            break;
        case NameFormat::CAMELCASE_CAPITAL:
            prevSpace = true;
            // fallthrough
        case NameFormat::CAMELCASE:
            for (char c : name) {
                if (!isAlphanumeric(c))
                    prevSpace = true;
                else {
                    if (prevSpace && !(c&0x80) && islower(c))
                        c = (char) toupper(c);
                    formatted.push_back(c);
                    prevSpace = false;
                }
            }
            break;
    }
    return formatted;
}
