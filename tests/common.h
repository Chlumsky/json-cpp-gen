
#pragma once

#include <cstdio>
#include <typeinfo>

extern int testFailures;

#define CHECK(condition) do { if (!(condition)) { fprintf(stderr, "FAILURE: %s\n", #condition); ++testFailures; } } while (false)
#define CHECK_RESULT(err) do { if (auto e = (err)) { fprintf(stderr, "%s: %s\n", typeid(e).name(), err.typeString()); } } while (false)
