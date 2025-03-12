
#pragma once

#include <cstdio>
#include <typeinfo>

extern int testFailures;

#define CHECK(condition) do { if (!(condition)) { fprintf(stderr, "FAILURE: %s\n", #condition); ++testFailures; } } while (false)
#define CHECK_RESULT(err) do { if (auto e = (err)) { fprintf(stderr, "%s: %s\n", typeid(e).name(), err.typeString()); } } while (false)
#define AUTO_EQ(T) bool operator==(const T &) const = default; bool operator!=(const T &) const = default

#ifndef NDEBUG
#define DUMP_JSON(title, jsonStdString) fprintf(stderr, "\n%s JSON:\n%s\n", title, (jsonStdString).c_str())
#endif

#ifndef DUMP_JSON
#define DUMP_JSON(title, jsonStdString) do { } while (false)
#endif
