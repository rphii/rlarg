#include "utf8.h"
#include <rphii/err.h>

void str_u8str(U8Str u8str, Str str) {
    ASSERT_ARG(u8str);
    str_cstr(str, u8str, U8_CAP);
}

void rstr_u8str(U8Str u8str, RStr str) {
    ASSERT_ARG(u8str);
    rstr_cstr(str, u8str, U8_CAP);
}

void u8str_rstr(RStr *str, U8Str u8str) {
    ASSERT_ARG(str);
    ASSERT_ARG(u8str);
    size_t len = strlen(u8str);
    ASSERT(len < U8_CAP, "length of string should not be > %u", U8_CAP);
    *str = RSTR_LL(u8str, len);
}

ErrImpl cstr_to_u8_point(U8Str in, U8Point *point)
{
    ASSERT_ARG(in);
    ASSERT_ARG(point);
    U8Point tinker = {0};
    // figure out how many bytes we need
    if((*in & 0x80) == 0) point->bytes = 1;
    else if((*in & 0xE0) == 0xC0) point->bytes = 2;
    else if((*in & 0xF0) == 0xE0) point->bytes = 3;
    else if((*in & 0xF8) == 0xF0) point->bytes = 4;
    else THROW("unknown utf-8 pattern");
    // magical mask shifting
    int shift = (point->bytes - 1) * 6;
    int mask = 0x7F;
    if(point->bytes > 1) mask >>= point->bytes;
    // extract info from bytes
    for(int i = 0; i < point->bytes; i++) {
        // add number to point
        if(!in[i]) THROW(ERR_CSTR_INVALID);
        tinker.val |= (uint32_t)((in[i] & mask) << shift);
        if(mask == 0x3F) {
            if((unsigned char)(in[i] & ~mask) != 0x80) {
                THROW("encountered invalid bytes in utf-8 sequence");
                point->bytes = 0;
                break;
            }
        }
        // adjust shift amount
        shift -= 6;
        // update mask
        mask = 0x3F;
    }
    // one final check, unicode doesn't go that far, wth unicode, TODO check ?!
    if(tinker.val > 0x10FFFF || !point->bytes) {
        point->val = (unsigned char)*in;
        point->bytes = 1;
    } else {
        point->val = tinker.val;
    }
    return 0;
error:
    return -1;
}

ErrImpl cstr_from_u8_point(U8Str out, U8Point *point)
{
    ASSERT_ARG(out);
    ASSERT_ARG(point);
    int bytes = 0;
    int shift = 0;  // shift in bits
    uint32_t in = point->val;
    // figure out how many bytes we need
    if(in < 0x0080) bytes = 1;
    else if(in < 0x0800) bytes = 2;
    else if(in < 0x10000) bytes = 3;
    else if(in < 0x200000) bytes = 4;
    else if(in < 0x4000000) bytes = 5;
    else if(in < 0x80000000) bytes = 6;
    shift = (bytes - 1) * 6;
    uint32_t mask = 0x7F;
    if(bytes > 1) mask >>= bytes;
    // create bytes
    for(int i = 0; i < bytes; i++)
    {
        // add actual character coding
        out[i] = (char)((in >> shift) & mask);
        // add first byte code
        if(!i && bytes > 1) {
            out[i] |= (char)(((uint32_t)~0 << (8 - bytes)) & 0xFF);
        }
        // add any other code
        if(i) {
            out[i] |= (char)0x80;
        }
        // adjust shift and reset mask
        shift -= 6;
        mask = 0x3F;
    }
    point->bytes = bytes;
    return 0;
error:
    return -1;
}

