
#pragma once

#include <string>
#include "Configuration.h"

bool readTextFile(std::string &fileContents, const char *filename);
bool writeTextFile(const std::string &fileContents, const char *filename, Settings::LineEndingStyle lineEndingStyle);
