
#include "NameFormat.h"

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
                if (!isalnum(c)) {
                    if (!prevSpace)
                        formatted.push_back(separator);
                    prevSpace = true;
                } else {
                    if (islower(c))
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
                if (!isalnum(c)) {
                    if (!prevSpace)
                        formatted.push_back(separator);
                    prevSpace = true;
                } else {
                    if (isupper(c))
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
                if (!isalnum(c))
                    prevSpace = true;
                else {
                    if (prevSpace && islower(c))
                        c = (char) toupper(c);
                    formatted.push_back(c);
                    prevSpace = false;
                }
            }
            break;
    }
    return formatted;
}
