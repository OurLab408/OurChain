#ifndef INCLUDE_BINDINGS_H
#define INCLUDE_BINDINGS_H

#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>

extern "C" {

int32_t rust_add(int32_t a, int32_t b);

} // extern "C"

#endif // INCLUDE_BINDINGS_H
