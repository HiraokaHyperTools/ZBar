/*------------------------------------------------------------------------
 *  Copyright 2008-2010 (c) Jeff Brown <spadix@users.sourceforge.net>
 *
 *  This file is part of the ZBar Bar Code Reader.
 *
 *  The ZBar Bar Code Reader is free software; you can redistribute it
 *  and/or modify it under the terms of the GNU Lesser Public License as
 *  published by the Free Software Foundation; either version 2.1 of
 *  the License, or (at your option) any later version.
 *
 *  The ZBar Bar Code Reader is distributed in the hope that it will be
 *  useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser Public License
 *  along with the ZBar Bar Code Reader; if not, write to the Free
 *  Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *  Boston, MA  02110-1301  USA
 *
 *  http://sourceforge.net/projects/zbar
 *------------------------------------------------------------------------*/

#include <config.h>
#include <string.h>     /* memmove */

#include <zbar.h>

#ifdef DEBUG_CODE39
# define DEBUG_LEVEL (DEBUG_CODE39)
#endif
#include "debug.h"
#include "decoder.h"

#define NUM_CHARS (0x2c)

static const unsigned char code39_hi[32] = {
    0x80 | 0x00,  /* 2 next */
    0x40 | 0x02,  /* 4 */
    0x80 | 0x06,  /* 2 next */
    0xc0 | 0x08,  /* 2 skip */
    0x40 | 0x0a,  /* 4 */
    0x80 | 0x0e,  /* 2 next */
    0xc0 | 0x10,  /* 2 skip */
    0x00 | 0x12,  /* direct */

    0x80 | 0x13,  /* 2 next */
    0xc0 | 0x15,  /* 2 skip */
    0x80 | 0x17,  /* 2 next */
    0xff,
    0xc0 | 0x19,  /* 2 skip */
    0x00 | 0x1b,  /* direct */
    0xff,
    0xff,

    0x40 | 0x1c,  /* 4 */
    0x80 | 0x20,  /* 2 next */
    0xc0 | 0x22,  /* 2 skip */
    0x00 | 0x24,  /* direct */
    0x80 | 0x25,  /* 2 next */
    0xff,
    0x00 | 0x27,  /* direct */
    0xff,

    0xc0 | 0x28,  /* 2 skip */
    0x00 | 0x2a,  /* direct */
    0xff,
    0xff,
    0x00 | 0x2b,  /* direct */
    0xff,
    0xff,
    0xff,
};

typedef struct char39_s {
    unsigned char chk, rev, fwd;
} char39_t;

static const char39_t code39_encodings[NUM_CHARS] = {
    { 0x07, 0x1a, 0x20 }, /* 00 */
    { 0x0d, 0x10, 0x03 }, /* 01 */
    { 0x13, 0x17, 0x22 }, /* 02 */
    { 0x16, 0x1d, 0x23 }, /* 03 */
    { 0x19, 0x0d, 0x05 }, /* 04 */
    { 0x1c, 0x13, 0x06 }, /* 05 */
    { 0x25, 0x07, 0x0c }, /* 06 */
    { 0x2a, 0x2a, 0x27 }, /* 07 */
    { 0x31, 0x04, 0x0e }, /* 08 */
    { 0x34, 0x00, 0x0f }, /* 09 */
    { 0x43, 0x15, 0x25 }, /* 0a */
    { 0x46, 0x1c, 0x26 }, /* 0b */
    { 0x49, 0x0b, 0x08 }, /* 0c */
    { 0x4c, 0x12, 0x09 }, /* 0d */
    { 0x52, 0x19, 0x2b }, /* 0e */
    { 0x58, 0x0f, 0x00 }, /* 0f */
    { 0x61, 0x02, 0x11 }, /* 10 */
    { 0x64, 0x09, 0x12 }, /* 11 */
    { 0x70, 0x06, 0x13 }, /* 12 */
    { 0x85, 0x24, 0x16 }, /* 13 */
    { 0x8a, 0x29, 0x28 }, /* 14 */
    { 0x91, 0x21, 0x18 }, /* 15 */
    { 0x94, 0x2b, 0x19 }, /* 16 */
    { 0xa2, 0x28, 0x29 }, /* 17 */
    { 0xa8, 0x27, 0x2a }, /* 18 */
    { 0xc1, 0x1f, 0x1b }, /* 19 */
    { 0xc4, 0x26, 0x1c }, /* 1a */
    { 0xd0, 0x23, 0x1d }, /* 1b */
    { 0x03, 0x14, 0x1e }, /* 1c */
    { 0x06, 0x1b, 0x1f }, /* 1d */
    { 0x09, 0x0a, 0x01 }, /* 1e */
    { 0x0c, 0x11, 0x02 }, /* 1f */
    { 0x12, 0x18, 0x21 }, /* 20 */
    { 0x18, 0x0e, 0x04 }, /* 21 */
    { 0x21, 0x01, 0x0a }, /* 22 */
    { 0x24, 0x08, 0x0b }, /* 23 */
    { 0x30, 0x05, 0x0d }, /* 24 */
    { 0x42, 0x16, 0x24 }, /* 25 */
    { 0x48, 0x0c, 0x07 }, /* 26 */
    { 0x60, 0x03, 0x10 }, /* 27 */
    { 0x81, 0x1e, 0x14 }, /* 28 */
    { 0x84, 0x25, 0x15 }, /* 29 */
    { 0x90, 0x22, 0x17 }, /* 2a */
    { 0xc0, 0x20, 0x1a }, /* 2b */
};

static const unsigned char code39_characters[NUM_CHARS] =
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%*";

static inline unsigned char code39_decode1 (unsigned char enc,
                                            unsigned e,
                                            unsigned s)
{
    unsigned char E = decode_e(e, s, 72);
    if(E > 18)
        return(0xff);
    enc <<= 1;
    if(E > 6) {
        enc |= 1;
        dbprintf(2, "1");
    }
    else
        dbprintf(2, "0");
    return(enc);
}

static int toNarrowWidePattern(zbar_decoder_t *dcode, int reverse)
{
    const int numCounters = 9;
    int maxNarrowCounter = 0;
    int wideCounters;
    do
    {
        int minCounter = INT_MAX;
        for (int index = 0; index < 9; index++)
        {
            int counter = get_width(dcode, index);
            if (counter < minCounter && counter > maxNarrowCounter)
            {
                minCounter = counter;
            }
        }
        maxNarrowCounter = minCounter;
        wideCounters = 0;
        int totalWideCountersWidth = 0;
        int pattern = 0;
        for (int index = 0; index < numCounters; index++)
        {
            int counter = get_width(dcode, index);
            if (counter > maxNarrowCounter)
            {
                if (reverse) {
                    pattern |= 1 << (numCounters - 1 - index);
                }
                else {
                    pattern |= 1 << (index);
                }
                wideCounters++;
                totalWideCountersWidth += counter;
            }
        }
        if (wideCounters == 3)
        {
            // Found 3 wide counters, but are they close enough in width?
            // We can perform a cheap, conservative check to see if any individual
            // counter is more than 1.5 times the average:
            for (int index = 0; index < numCounters && wideCounters > 0; index++)
            {
                int counter = get_width(dcode, index);
                if (counter > maxNarrowCounter)
                {
                    wideCounters--;
                    // totalWideCountersWidth = 3 * average, so this checks if counter >= 3/2 * average
                    if ((counter << 1) >= totalWideCountersWidth)
                    {
                        return -1;
                    }
                }
            }
            return pattern;
        }
    } while (wideCounters > 3);
    return -1;
}

static inline int patternToIndex(unsigned pattern) {
    static int CHARACTER_ENCODINGS[] = {
        0x034, 0x121, 0x061, 0x160, 0x031, 0x130, 0x070, 0x025, 0x124, 0x064, // 0-9
        0x109, 0x049, 0x148, 0x019, 0x118, 0x058, 0x00D, 0x10C, 0x04C, 0x01C, // A-J
        0x103, 0x043, 0x142, 0x013, 0x112, 0x052, 0x007, 0x106, 0x046, 0x016, // K-T
        0x181, 0x0C1, 0x1C0, 0x091, 0x190, 0x0D0, 0x085, 0x184, 0x0C4, 0x0A8, // U-$
        0x0A2, 0x08A, 0x02A // /-%
    };

    int i;
    for (i = 0; i < 43; i++)
    {
        if (CHARACTER_ENCODINGS[i] == pattern)
        {
            return i;
        }
    }
    return i;
}

static inline signed char code39_decode9_zxing_flavor(zbar_decoder_t *dcode)
{
    code39_decoder_t *dcode39 = &dcode->code39;

    if (dcode39->s9 < 9)
        return(-1);

    int pattern = toNarrowWidePattern(dcode, dcode39->direction);
    if (pattern == -1)
        return(-1);

    int idx = patternToIndex(pattern);
    if (idx == -1)
        return -1;

    return idx;
}

static inline signed char code39_decode9 (zbar_decoder_t *dcode)
{
    code39_decoder_t *dcode39 = &dcode->code39;

    if(dcode39->s9 < 9)
        return(-1);

    /* threshold bar width ratios */
    unsigned char i, enc = 0;
    for(i = 0; i < 5; i++) {
        enc = code39_decode1(enc, get_width(dcode, i), dcode39->s9);
        if(enc == 0xff)
            return(-1);
    }
    zassert(enc < 0x20, -1, " enc=%x s9=%x\n", enc, dcode39->s9);

    /* lookup first 5 encoded widths for coarse decode */
    unsigned char idx = code39_hi[enc];
    if(idx == 0xff)
        return(-1);

    /* encode remaining widths (NB first encoded width is lost) */
    for(; i < 9; i++) {
        enc = code39_decode1(enc, get_width(dcode, i), dcode39->s9);
        if(enc == 0xff)
            return(-1);
    }

    if((idx & 0xc0) == 0x80)
        idx = (idx & 0x3f) + ((enc >> 3) & 1);
    else if((idx & 0xc0) == 0xc0)
        idx = (idx & 0x3f) + ((enc >> 2) & 1);
    else if(idx & 0xc0)
        idx = (idx & 0x3f) + ((enc >> 2) & 3);
    zassert(idx < 0x2c, -1, " idx=%x enc=%x s9=%x\n", idx, enc, dcode39->s9);

    const char39_t *c = &code39_encodings[idx];
    dbprintf(2, " i=%02x chk=%02x c=%02x/%02x", idx, c->chk, c->fwd, c->rev);
    if(enc != c->chk)
        return(-1);

    dcode39->width = dcode39->s9;
    return((dcode39->direction) ? c->rev : c->fwd);
}

static inline signed char code39_decode_start (zbar_decoder_t *dcode)
{
    code39_decoder_t *dcode39 = &dcode->code39;
    dbprintf(2, " s=%d ", dcode39->s9);

    signed char c = code39_decode9(dcode);
    if(c != 0x19 && c != 0x2b) {
        dbprintf(2, "\n");
        return(ZBAR_NONE);
    }
    dcode39->direction ^= (c == 0x19);

    /* check leading quiet zone - spec is 10x */
    unsigned quiet = get_width(dcode, 9);
    if(quiet && quiet < dcode39->s9 / 4) {
        dbprintf(2, " [invalid quiet]\n");
        return(ZBAR_NONE);
    }

    dcode39->element = 9;
    dcode39->character = 0;
    dbprintf(1, " dir=%x [valid start]\n", dcode39->direction);
    return(ZBAR_PARTIAL);
}

static inline int code39_postprocess (zbar_decoder_t *dcode)
{
    code39_decoder_t *dcode39 = &dcode->code39;
    dcode->direction = 1 - 2 * dcode39->direction;
    int i, written, length, invalidCombination;
    unsigned char c, next;
    if(dcode39->direction) {
        /* reverse buffer */
        dbprintf(2, " (rev)");
        for(i = 0; i < dcode39->character / 2; i++) {
            unsigned j = dcode39->character - 1 - i;
            char code = dcode->buf[i];
            dcode->buf[i] = dcode->buf[j];
            dcode->buf[j] = code;
        }
    }
    for(i = 0; i < dcode39->character; i++)
        dcode->buf[i] = ((dcode->buf[i] < 0x2b)
                         ? code39_characters[(unsigned)dcode->buf[i]]
                         : '?');
    // This converter taken from ZXing.Net: Code39Reader.cs
    if (TEST_CFG(dcode39->config, ZBAR_CFG_ASCII)) {
        length = dcode39->character;
        written = 0;
        invalidCombination = 0;
        for (i = 0; i < length; ) {
            c = dcode->buf[i++];
            if (c == '+' || c == '$' || c == '%' || c == '/') {
                if (i < length) {
                    next = dcode->buf[i++];
                    if (0) {}
                    else if (c == '+') {
                        if (next >= 'A' && next <= 'Z') {
                            // "+A" ¨ 'a'
                            //  ...
                            // "+Z" ¨ 'z'
                            dcode->buf[written++] = next + 32;
                        }
                        else {
                            invalidCombination = 1;
                        }
                    }
                    else if (c == '$') {
                        if (next >= 'A' && next <= 'Z') {
                            // "$A" ¨ SOH (0x01)
                            //  ...
                            // "$Z" ¨ SUB (0x1A)
                            dcode->buf[written++] = next - 64;
                        }
                        else {
                            invalidCombination = 1;
                        }
                    }
                    else if (c == '%') {
                        if (0) {}
                        else if (next >= 'A' && next <= 'E') {
                            // "%A" ¨ ESC (0x1B)
                            //  ...
                            // "%E" ¨ US  (0x1F)
                            dcode->buf[written++] = next - 38;
                        }
                        else if (next >= 'F' && next <= 'J') {
                            // "%F" ¨ ';' 0x3B
                            //  ...
                            // "%J" ¨ '?' 0x3F
                            dcode->buf[written++] = next - 11;
                        }
                        else if (next >= 'K' && next <= 'O') {
                            // "%K" ¨ '[' 0x5B
                            //  ...
                            // "%O" ¨ '_' 0x5F
                            dcode->buf[written++] = next + 16;
                        }
                        else if (next >= 'P' && next <= 'T') {
                            // "%P" ¨ '{' 0x7B
                            //  ...
                            // "%T" ¨ DEL 0x7F
                            dcode->buf[written++] = next + 43;
                        }
                        else if (next == 'U') {
                            dcode->buf[written++] = 0;
                        }
                        else if (next == 'V') {
                            dcode->buf[written++] = '@';
                        }
                        else if (next == 'W') {
                            dcode->buf[written++] = '`';
                        }
                        else if (next == 'X' || next == 'Y' || next == 'Z') {
                            dcode->buf[written++] = 127;
                        }
                        else {
                            invalidCombination = 1;
                        }
                    }
                    else if (c == '/') {
                        if (0) {}
                        else if (next >= 'A' && next <= 'O') {
                            // "/A" ¨ '!' 0x21
                            //  ...
                            // "/O" ¨ '/' 0x2F
                            dcode->buf[written++] = next - 32;
                        }
                        else if (next == 'Z') {
                            dcode->buf[written++] = ':';
                        }
                        else {
                            invalidCombination = 1;
                        }
                    }
                }
                else {
                    invalidCombination = 1;
                }
            }
            else {
                dcode->buf[written++] = c;
            }
        }
        if (invalidCombination) {
            return(1);
        }
        i = written; // trimming
    }
    zassert(i < dcode->buf_alloc, -1, "i=%02x %s\n", i,
            _zbar_decoder_buf_dump(dcode->buf, dcode39->character));
    dcode->buflen = i;
    dcode->buf[i] = '\0';
    dcode->modifiers = 0;
    return(0);
}

static inline int
check_width (unsigned ref,
             unsigned w)
{
    unsigned dref = ref;
    ref *= 4;
    w *= 4;
    return(ref - dref <= w && w <= ref + dref);
}

zbar_symbol_type_t _zbar_decode_code39 (zbar_decoder_t *dcode)
{
    code39_decoder_t *dcode39 = &dcode->code39;

    /* update latest character width */
    dcode39->s9 -= get_width(dcode, 9);
    dcode39->s9 += get_width(dcode, 0);

    if(dcode39->character < 0) {
        if(get_color(dcode) != ZBAR_BAR)
            return(ZBAR_NONE);
        dbprintf(2, "      code39:");
        return(code39_decode_start(dcode));
    }

    if(++dcode39->element < 9)
        return(ZBAR_NONE);

    dbprintf(2, "      code39[%c%02d+%x]",
             (dcode39->direction) ? '<' : '>',
             dcode39->character, dcode39->element);

    if(dcode39->element == 10) {
        unsigned space = get_width(dcode, 0);
        if(dcode39->character &&
           dcode->buf[dcode39->character - 1] == 0x2b) {  /* STOP */
            /* trim STOP character */
            dcode39->character--;
            zbar_symbol_type_t sym = ZBAR_NONE;

            /* trailing quiet zone check */
            if(space && space < dcode39->width / 2)
                dbprintf(2, " [invalid qz]\n");
            else if(dcode39->character < CFG(*dcode39, ZBAR_CFG_MIN_LEN) ||
                    (CFG(*dcode39, ZBAR_CFG_MAX_LEN) > 0 &&
                     dcode39->character > CFG(*dcode39, ZBAR_CFG_MAX_LEN)))
                dbprintf(2, " [invalid len]\n");
            else if(!code39_postprocess(dcode)) {
                /* FIXME checksum */
                dbprintf(2, " [valid end]\n");
                sym = ZBAR_CODE39;
            }
            dcode39->character = -1;
            if(!sym)
                release_lock(dcode, ZBAR_CODE39);
            return(sym);
        }
        if(space > dcode39->width / 2) {
            /* inter-character space check failure */
            dbprintf(2, " ics>%d [invalid ics]", dcode39->width);
            if(dcode39->character)
                release_lock(dcode, ZBAR_CODE39);
            dcode39->character = -1;
        }
        dcode39->element = 0;
        dbprintf(2, "\n");
        return(ZBAR_NONE);
    }

    dbprintf(2, " s=%d ", dcode39->s9);
    if(!check_width(dcode39->width, dcode39->s9)) {
        dbprintf(2, " [width]\n");
        if(dcode39->character)
            release_lock(dcode, ZBAR_CODE39);
        dcode39->character = -1;
        return(ZBAR_NONE);
    }

    signed char c = TEST_CFG(dcode39->config, ZBAR_CFG_ZXING)
        ? code39_decode9_zxing_flavor(dcode)
        : code39_decode9(dcode);
    dbprintf(2, " c=%d", c);

    /* lock shared resources */
    if(!dcode39->character && acquire_lock(dcode, ZBAR_CODE39)) {
        dcode39->character = -1;
        return(ZBAR_PARTIAL);
    }

    if(c < 0 || size_buf(dcode, dcode39->character + 1)) {
        dbprintf(1, (c < 0) ? " [aborted]\n" : " [overflow]\n");
        release_lock(dcode, ZBAR_CODE39);
        dcode39->character = -1;
        return(ZBAR_NONE);
    }
    else {
        zassert(c < 0x2c, ZBAR_NONE, "c=%02x s9=%x\n", c, dcode39->s9);
        dbprintf(2, "\n");
    }

    dcode->buf[dcode39->character++] = c;

    return(ZBAR_NONE);
}
