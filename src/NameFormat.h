
#pragma once

#include <string>

enum class NameFormat {
    /// Any format is valid, no conversion takes place.
    ANY,
    /// UPPERCASE_SEPARATED_BY_UNDERSCORE
    UPPERCASE_UNDERSCORE,
    /// lowercase_separated_by_underscore
    LOWERCASE_UNDERSCORE,
    /// UPPERCASE-SEPARATED-BY-DASH
    UPERCASE_DASH,
    /// lowercase-separated-by-dash
    LOWERCASE_DASH,
    /// firstLetterOfEachWordExceptTheFirstCapitalized
    CAMELCASE,
    /// FirstLetterOfEachWordCapitalized
    CAMELCASE_CAPITAL
};

std::string formatName(const std::string &name, NameFormat format);
