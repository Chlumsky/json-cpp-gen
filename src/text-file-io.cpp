
#include "text-file-io.h"

#include <cstdio>

bool readTextFile(std::string &fileContents, const char *filename) {
    if (FILE *f = fopen(filename, "rb")) {
        fseek(f, 0, SEEK_END);
        size_t fileSize = ftell(f);
        rewind(f);
        fileContents.resize(fileSize);
        bool success = fread(&fileContents[0], 1, fileSize, f) == fileSize;
        fclose(f);
        return success;
    }
    return false;
}

bool writeTextFile(const std::string &fileContents, const char *filename, Settings::LineEndingStyle lineEndingStyle) {
    bool success = false;
    if (FILE *f = fopen(filename, lineEndingStyle == Settings::LineEndingStyle::NATIVE ? "w" : "wb")) {
        if (lineEndingStyle == Settings::LineEndingStyle::CRLF) {
            std::string processedFileContents;
            processedFileContents.reserve(size_t(1.125*fileContents.size()));
            for (char c : fileContents) {
                if (c == '\n')
                    processedFileContents.push_back('\r');
                processedFileContents.push_back(c);
            }
            success = fwrite(processedFileContents.c_str(), 1, processedFileContents.size(), f) == processedFileContents.size();
        } else
            success = fwrite(fileContents.c_str(), 1, fileContents.size(), f) == fileContents.size();
        fclose(f);
    }
    return success;
}
