/*
 * Copyright 2015-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#define declare_dh_bn(x) \
    extern const BIGNUM _bignum_dh##x##_p;              \
    extern const BIGNUM _bignum_dh##x##_g;              \
    extern const BIGNUM _bignum_dh##x##_q;

declare_dh_bn(1024_160)
declare_dh_bn(2048_224)
declare_dh_bn(2048_256)

extern const BIGNUM _bignum_ffdhe2048_p;
extern const BIGNUM _bignum_ffdhe3072_p;
extern const BIGNUM _bignum_ffdhe4096_p;
extern const BIGNUM _bignum_ffdhe6144_p;
extern const BIGNUM _bignum_ffdhe8192_p;
extern const BIGNUM _bignum_ffdhe2048_q;
extern const BIGNUM _bignum_ffdhe3072_q;
extern const BIGNUM _bignum_ffdhe4096_q;
extern const BIGNUM _bignum_ffdhe6144_q;
extern const BIGNUM _bignum_ffdhe8192_q;
extern const BIGNUM _bignum_const_2;

extern const BIGNUM _bignum_modp_1536_p;
extern const BIGNUM _bignum_modp_2048_p;
extern const BIGNUM _bignum_modp_3072_p;
extern const BIGNUM _bignum_modp_4096_p;
extern const BIGNUM _bignum_modp_6144_p;
extern const BIGNUM _bignum_modp_8192_p;
extern const BIGNUM _bignum_modp_1536_q;
extern const BIGNUM _bignum_modp_2048_q;
extern const BIGNUM _bignum_modp_3072_q;
extern const BIGNUM _bignum_modp_4096_q;
extern const BIGNUM _bignum_modp_6144_q;
extern const BIGNUM _bignum_modp_8192_q;
