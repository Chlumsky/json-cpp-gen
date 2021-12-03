
#include "AbsPath.h"

#include <cstdio>
#include <cstdlib>
#include <climits>

#ifdef _WIN32

#include <direct.h>
#define getcwd _getcwd
#define PATH_MAX _MAX_PATH
#define IS_PATH_SEPARATOR(c) ((c) == '/' || (c) == '\\')

static bool isAbsolutePath(const std::string &path) {
    if (path.empty())
        return false;
    if (IS_PATH_SEPARATOR(path.front()))
        return true;
    for (char c : path) {
        if (c == ':')
            return true;
        if (IS_PATH_SEPARATOR(c))
            break;
    }
    return false;
}

#else

#include <unistd.h>
#define IS_PATH_SEPARATOR(c) ((c) == '/')

static bool isAbsolutePath(const std::string &path) {
    if (path.empty())
        return false;
    if (IS_PATH_SEPARATOR(path.front()))
        return true;
    return false;
}

#endif

static std::string getWorkingDirectoryPath() {
    std::string path;
    char buffer[PATH_MAX+8];
    if (char *cwd = getcwd(buffer, sizeof(buffer)-2))
        path = std::string(cwd)+'/';
    return path;
}

static std::string normalizePath(const std::string &path) {
    std::string outPath;
    char prevPrevPrevChar = '\0';
    char prevPrevChar = '\0';
    char prevChar = '\0';
    for (char c : path) {
        if (IS_PATH_SEPARATOR(c)) {
            c = '/';
            if (prevChar == '.' && prevPrevChar == '.' && prevPrevPrevChar == '/') {
                int i = (int) outPath.size()-4;
                if (i >= 0) {
                    while (i >= 0 && outPath[i] != '/')
                        --i;
                    outPath.resize(i+1);
                }
            } else if (prevChar == '.' && prevPrevChar == '/') {
                outPath.pop_back();
            } else if (prevChar != '/')
                outPath.push_back(c);
        } else
            outPath.push_back(c);
        prevPrevPrevChar = prevPrevChar;
        prevPrevChar = prevChar;
        prevChar = c;
    }
    return outPath;
}

static int commonPathPrefix(const std::string &a, const std::string &b) {
    int commonLen = 0, i = 0;
    for (const char *ac = a.c_str(), *bc = b.c_str(); *ac && *bc; ++ac, ++bc, ++i) {
        if (IS_PATH_SEPARATOR(*ac) || IS_PATH_SEPARATOR(*bc)) {
            if (IS_PATH_SEPARATOR(*ac) && IS_PATH_SEPARATOR(*bc))
                commonLen = i+1;
            else
                break;
        }
    }
    return commonLen;
}

const AbsPath AbsPath::workingDirectory(getWorkingDirectoryPath());

AbsPath::AbsPath(const std::string &path) {
    if (isAbsolutePath(path))
        absPath = normalizePath(path);
    else {
        absPath = workingDirectory.absPath;
        while (!absPath.empty() && !IS_PATH_SEPARATOR(absPath.back()))
            absPath.pop_back();
        absPath = normalizePath(absPath+path);
    }
}

AbsPath AbsPath::operator+(const std::string &path) const {
    AbsPath outPath;
    if (isAbsolutePath(path))
        outPath.absPath = normalizePath(path);
    else {
        outPath.absPath = absPath;
        while (!outPath.absPath.empty() && !IS_PATH_SEPARATOR(outPath.absPath.back()))
            outPath.absPath.pop_back();
        outPath.absPath = normalizePath(outPath.absPath+path);
    }
    return outPath;
}

std::string AbsPath::operator-(const AbsPath &other) const {
    int prefixLen = commonPathPrefix(absPath, other.absPath);
    if (!prefixLen)
        return absPath;
    if (prefixLen == (int) other.absPath.size())
        return absPath.substr(prefixLen);
    std::string relPath;
    for (const char *c = other.absPath.c_str()+prefixLen; *c; ++c) {
        if (IS_PATH_SEPARATOR(*c))
            relPath += "../";
    }
    relPath += absPath.substr(prefixLen);
    return relPath;
}

const std::string & AbsPath::string() const {
    return absPath;
}

const char * AbsPath::cStr() const {
    return absPath.c_str();
}
