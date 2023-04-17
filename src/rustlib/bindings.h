#ifndef INCLUDE_BINDINGS_H
#define INCLUDE_BINDINGS_H

#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>

extern "C" {

char *get_zk_proof(const char *folder, const char *params);

char *encrypt_bjj(const char *point, const char *scalar);

/// private key (64 * 4bits) + public key (64 * 4bits) in hex
char *random_bjj();

///
char *ym_encode_0(uint64_t n);

///
char *ym_encode_1(uint64_t n);

/// call this to char* returned from rust to free the space.
void free_str(char *s);

} // extern "C"

#endif // INCLUDE_BINDINGS_H
