#pragma once

#include <fstream>

#define assert(X, ...) \
  do { \
    if (!(X)) { \
      printf("assert failed in %s:%d: ", __FILE__, __LINE__); \
      printf(__VA_ARGS__); \
    } \
  } while (0)

#define die(msg) do { puts(msg); exit(1); } while (0)

