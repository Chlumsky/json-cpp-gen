
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

bool writeTextFile(const std::string &fileContents, const char *filename) {
    if (FILE *f = fopen(filename, "wb")) {
        bool success = fwrite(fileContents.c_str(), 1, fileContents.size(), f) == fileContents.size();
        fclose(f);
        return success;
    }
    return false;
}
