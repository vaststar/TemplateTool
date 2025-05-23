/*
 * WARNING: do not edit!
 * Generated by Makefile from include/openssl/opensslconf.h.in
 *
 * Copyright 2016-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef HEADER_OPENSSLCONF_H
#define HEADER_OPENSSLCONF_H

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef OPENSSL_ALGORITHM_DEFINES
# error OPENSSL_ALGORITHM_DEFINES no longer supported
#endif

/*
 * OpenSSL was configured with the following options:
 */

#ifndef OPENSSL_SYS_MACOSX
# define OPENSSL_SYS_MACOSX 1
#endif
#ifndef OPENSSL_NO_ARIA
# define OPENSSL_NO_ARIA
#endif
#ifndef OPENSSL_NO_CAMELLIA
# define OPENSSL_NO_CAMELLIA
#endif
#ifndef OPENSSL_NO_DEPRECATED_PSB_DH_SAFE_PRIMES
# define OPENSSL_NO_DEPRECATED_PSB_DH_SAFE_PRIMES
#endif
#ifndef OPENSSL_NO_EAP_FAST_CIPHERS
# define OPENSSL_NO_EAP_FAST_CIPHERS
#endif
#ifndef OPENSSL_NO_ENTROPY_MIXING
# define OPENSSL_NO_ENTROPY_MIXING
#endif
#ifndef OPENSSL_NO_MD2
# define OPENSSL_NO_MD2
#endif
#ifndef OPENSSL_NO_MUST_NOT_PSB_CIPHERS
# define OPENSSL_NO_MUST_NOT_PSB_CIPHERS
#endif
#ifndef OPENSSL_NO_RC4
# define OPENSSL_NO_RC4
#endif
#ifndef OPENSSL_NO_RC5
# define OPENSSL_NO_RC5
#endif
#ifndef OPENSSL_NO_SEED
# define OPENSSL_NO_SEED
#endif
#ifndef OPENSSL_NO_SHOULD_NOT_PSB_CIPHERS
# define OPENSSL_NO_SHOULD_NOT_PSB_CIPHERS
#endif
#ifndef OPENSSL_NO_SM2
# define OPENSSL_NO_SM2
#endif
#ifndef OPENSSL_NO_SM3
# define OPENSSL_NO_SM3
#endif
#ifndef OPENSSL_NO_SM4
# define OPENSSL_NO_SM4
#endif
#ifndef OPENSSL_NO_T_EC_PSB_CIPHERS
# define OPENSSL_NO_T_EC_PSB_CIPHERS
#endif
#ifndef OPENSSL_THREADS
# define OPENSSL_THREADS
#endif
#ifndef OPENSSL_RAND_SEED_OS
# define OPENSSL_RAND_SEED_OS
#endif
#ifndef OPENSSL_FIPS
# define OPENSSL_FIPS
#endif
#ifndef OPENSSL_NO_AFALGENG
# define OPENSSL_NO_AFALGENG
#endif
#ifndef OPENSSL_NO_ASAN
# define OPENSSL_NO_ASAN
#endif
#ifndef OPENSSL_NO_CRYPTO_MDEBUG
# define OPENSSL_NO_CRYPTO_MDEBUG
#endif
#ifndef OPENSSL_NO_CRYPTO_MDEBUG_BACKTRACE
# define OPENSSL_NO_CRYPTO_MDEBUG_BACKTRACE
#endif
#ifndef OPENSSL_NO_DEVCRYPTOENG
# define OPENSSL_NO_DEVCRYPTOENG
#endif
#ifndef OPENSSL_NO_EARLY_DATA
# define OPENSSL_NO_EARLY_DATA
#endif
#ifndef OPENSSL_NO_EC_NISTP_64_GCC_128
# define OPENSSL_NO_EC_NISTP_64_GCC_128
#endif
#ifndef OPENSSL_NO_EGD
# define OPENSSL_NO_EGD
#endif
#ifndef OPENSSL_NO_EXTERNAL_TESTS
# define OPENSSL_NO_EXTERNAL_TESTS
#endif
#ifndef OPENSSL_NO_FUZZ
# define OPENSSL_NO_FUZZ
#endif
#ifndef OPENSSL_NO_FUZZ_AFL
# define OPENSSL_NO_FUZZ_AFL
#endif
#ifndef OPENSSL_NO_FUZZ_LIBFUZZER
# define OPENSSL_NO_FUZZ_LIBFUZZER
#endif
#ifndef OPENSSL_NO_GOST
# define OPENSSL_NO_GOST
#endif
#ifndef OPENSSL_NO_HEARTBEATS
# define OPENSSL_NO_HEARTBEATS
#endif
#ifndef OPENSSL_NO_JENT
# define OPENSSL_NO_JENT
#endif
#ifndef OPENSSL_NO_MSAN
# define OPENSSL_NO_MSAN
#endif
#ifndef OPENSSL_NO_SCTP
# define OPENSSL_NO_SCTP
#endif
#ifndef OPENSSL_NO_SSL_TRACE
# define OPENSSL_NO_SSL_TRACE
#endif
#ifndef OPENSSL_NO_SSL3
# define OPENSSL_NO_SSL3
#endif
#ifndef OPENSSL_NO_SSL3_METHOD
# define OPENSSL_NO_SSL3_METHOD
#endif
#ifndef OPENSSL_NO_UBSAN
# define OPENSSL_NO_UBSAN
#endif
#ifndef OPENSSL_NO_UNIT_TEST
# define OPENSSL_NO_UNIT_TEST
#endif
#ifndef OPENSSL_NO_WEAK_CURVES
# define OPENSSL_NO_WEAK_CURVES
#endif
#ifndef OPENSSL_NO_WEAK_SSL_CIPHERS
# define OPENSSL_NO_WEAK_SSL_CIPHERS
#endif
#ifndef OPENSSL_NO_DYNAMIC_ENGINE
# define OPENSSL_NO_DYNAMIC_ENGINE
#endif


/*
 * Sometimes OPENSSSL_NO_xxx ends up with an empty file and some compilers
 * don't like that.  This will hopefully silence them.
 */
#define NON_EMPTY_TRANSLATION_UNIT static void *dummy = &dummy;

/*
 * Applications should use -DOPENSSL_API_COMPAT=<version> to suppress the
 * declarations of functions deprecated in or before <version>. Otherwise, they
 * still won't see them if the library has been built to disable deprecated
 * functions.
 */
#ifndef DECLARE_DEPRECATED
# define DECLARE_DEPRECATED(f)   f;
# ifdef __GNUC__
#  if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ > 0)
#   undef DECLARE_DEPRECATED
#   define DECLARE_DEPRECATED(f)    f __attribute__ ((deprecated));
#  endif
# elif defined(__SUNPRO_C)
#  if (__SUNPRO_C >= 0x5130)
#   undef DECLARE_DEPRECATED
#   define DECLARE_DEPRECATED(f)    f __attribute__ ((deprecated));
#  endif
# endif
#endif

#ifndef OPENSSL_FILE
# ifdef OPENSSL_NO_FILENAMES
#  define OPENSSL_FILE ""
#  define OPENSSL_LINE 0
# else
#  define OPENSSL_FILE __FILE__
#  define OPENSSL_LINE __LINE__
# endif
#endif

#ifndef OPENSSL_MIN_API
# define OPENSSL_MIN_API 0
#endif

#if !defined(OPENSSL_API_COMPAT) || OPENSSL_API_COMPAT < OPENSSL_MIN_API
# undef OPENSSL_API_COMPAT
# define OPENSSL_API_COMPAT OPENSSL_MIN_API
#endif

/*
 * Do not deprecate things to be deprecated in version 1.2.0 before the
 * OpenSSL version number matches.
 */
#if OPENSSL_VERSION_NUMBER < 0x10200000L
# define DEPRECATEDIN_1_2_0(f)   f;
#elif OPENSSL_API_COMPAT < 0x10200000L
# define DEPRECATEDIN_1_2_0(f)   DECLARE_DEPRECATED(f)
#else
# define DEPRECATEDIN_1_2_0(f)
#endif

#if OPENSSL_API_COMPAT < 0x10100000L
# define DEPRECATEDIN_1_1_0(f)   DECLARE_DEPRECATED(f)
#else
# define DEPRECATEDIN_1_1_0(f)
#endif

#if OPENSSL_API_COMPAT < 0x10000000L
# define DEPRECATEDIN_1_0_0(f)   DECLARE_DEPRECATED(f)
#else
# define DEPRECATEDIN_1_0_0(f)
#endif

#if OPENSSL_API_COMPAT < 0x00908000L
# define DEPRECATEDIN_0_9_8(f)   DECLARE_DEPRECATED(f)
#else
# define DEPRECATEDIN_0_9_8(f)
#endif

/* Generate 80386 code? */
#undef I386_ONLY

#undef OPENSSL_UNISTD
#define OPENSSL_UNISTD <unistd.h>

#undef OPENSSL_EXPORT_VAR_AS_FUNCTION

/*
 * The following are cipher-specific, but are part of the public API.
 */
#if !defined(OPENSSL_SYS_UEFI)
# undef BN_LLONG
/* Only one for the following should be defined */
# define SIXTY_FOUR_BIT_LONG
# undef SIXTY_FOUR_BIT
# undef THIRTY_TWO_BIT
#endif

#define RC4_INT unsigned int

#ifdef  __cplusplus
}
#endif

#include <openssl/opensslv.h>
#endif //HEADER_OPENSSLCONF_H
