#pragma once

#include <cstdio>

#define die(...) do { fprintf(stderr, __VA_ARGS__); exit(1); } while (0)

