# src/contract/Makefile.am

# Common flags for both libraries in this directory
AM_CPPFLAGS = \
    -I$(top_srcdir)/src \
    -I$(top_builddir)/src \
    -I$(top_builddir)/src/config \
    -I$(top_srcdir)/src/secp256k1/include \
    -I$(top_srcdir)/src/univalue/include \
    $(BDB_CPPFLAGS) \
    $(BOOST_CPPFLAGS) \
    $(LEVELDB_CPPFLAGS) \
    $(CRYPTO_CFLAGS) \
    $(SSL_CFLAGS) \
    $(UNIVALUE_CFLAGS) \
    $(HARDENED_CPPFLAGS) \
    -fvisibility=hidden

AM_CXXFLAGS = $(PIE_FLAGS) $(HARDENED_CXXFLAGS) $(ERROR_CXXFLAGS)

# Define the TWO libraries to be built (not installed system-wide)
noinst_LIBRARIES = libcontract_core.a

# --- Sources for the Core Contract Library ---
# (Fundamental functions needed by bitcoind etc.)
libcontract_core_a_SOURCES = \
    processing.cpp \
    contractdb.cpp \
    dbWrapper.cpp \
    updatepolicy.cpp \
    contractserver.cpp \
    processing.h \
    contractdb.h \
    dbWrapper.h \
    updatepolicy.h \
    contractserver.h \
    threadsafe_queue.h \
    contract.h

bin_SCRIPTS = ourcontract-mkdll

# Distribute necessary non-source files
EXTRA_DIST = ourcontract-mkdll.in

CLEANFILES = $(noinst_LIBRARIES)