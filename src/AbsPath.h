
#pragma once

#include <string>

class AbsPath {

public:
    static const AbsPath workingDirectory;

    inline AbsPath() { }
    // Directory must end with path separator
    explicit AbsPath(const std::string &path);
    AbsPath operator+(const std::string &path) const;
    std::string operator-(const AbsPath &other) const;
    const std::string & string() const;
    const char * cStr() const;

private:
    std::string absPath;

};
