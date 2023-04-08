#ifndef INCLUDE_BINDINGS_H
#define INCLUDE_BINDINGS_H

#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>

extern "C" {

char *encrypt_bjj(const char *point, const char *scalar);

void free_str(char *s);

} // extern "C"

#endif // INCLUDE_BINDINGS_H
