/*
 * (C) Copyright 2005, Gregor Heinrich (gregor :: arbylon : net) (This file is
 * part of the org.knowceans.* experimental software packages.)
 */
/*
 * lda-j is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 */
/*
 * lda-j is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 */
/*
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * Created on Jan 9, 2005
 */
package org.knowceans.util;

/**
 * Java version of the Mersenne Twister mt19937, based on Shawn Cokus's C
 * implementation. See <a
 * href="http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html">
 * http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html </a> for details.
 * 
 * @author heinrich
 */
public class Cokus {

    //
    // uint32 must be an unsigned integer type capable of holding at least 32
    // bits; exactly 32 should be fastest, but 64 is better on an Alpha with
    // GCC at -O3 optimization so try your options and see what's best for you
    //

    /*
     * Java port: The notorious uint32 values are dubbed as longs and masking
     * operations performed on the lower half while keeping the higher half 0;
     */

    /* Period parameters */
    // length of state vector
    public static int N = 624;

    // a period parameter
    public static int M = 397;

    // a magic constant
    public static long K = 0x9908b0dfL;

    // state vector
    static long[] state = new long[N + 1];

    // next random value is computed from here (java: index into state[])
    static int next;

    // can *next++ this many times before
    // reloading
    static int left = -1;

    // mask all but highest bit of u (uint32)
    public static long hiBit(long u) {
        return (u) & 0x80000000l;
    }

    // mask all but lowest bit of u (uint32)
    public static long loBit(long u) {
        return (u) & 0x00000001l;
    }

    // mask the highest bit of u (uint32)
    public static long loBits(long u) {
        return (u) & 0x7FFFFFFFl;
    }

    // move hi bit of u to hi bit of v (uint32)
    public static long mixBits(long u, long v) {
        return hiBit(u) | loBits(v);
    }

    /**
     * Cokus versions/optimizations
     */

    public static void cokusseed(int seed) {
        //
        // We initialize state[0..(N-1)] via the generator
        //
        // x_new = (69069 * x_old) mod 2^32
        //
        // from Line 15 of Table 1, p. 106, Sec. 3.3.4 of Knuth's
        // _The Art of Computer Programming_, Volume 2, 3rd ed.
        //
        // Notes (SJC): I do not know what the initial state requirements
        // of the Mersenne Twister are, but it seems this seeding generator
        // could be better. It achieves the maximum period for its modulus
        // (2^30) iff x_initial is odd (p. 20-21, Sec. 3.2.1.2, Knuth); if
        // x_initial can be even, you have sequences like 0, 0, 0, ...;
        // 2^31, 2^31, 2^31, ...; 2^30, 2^30, 2^30, ...; 2^29, 2^29 + 2^31,
        // 2^29, 2^29 + 2^31, ..., etc. so I force seed to be odd below.
        //
        // Even if x_initial is odd, if x_initial is 1 mod 4 then
        //
        // the lowest bit of x is always 1,
        // the next-to-lowest bit of x is always 0,
        // the 2nd-from-lowest bit of x alternates ... 0 1 0 1 0 1 0 1 ... ,
        // the 3rd-from-lowest bit of x 4-cycles ... 0 1 1 0 0 1 1 0 ... ,
        // the 4th-from-lowest bit of x has the 8-cycle ... 0 0 0 1 1 1 1 0 ...
        // ,
        // ...
        //
        // and if x_initial is 3 mod 4 then
        //
        // the lowest bit of x is always 1,
        // the next-to-lowest bit of x is always 1,
        // the 2nd-from-lowest bit of x alternates ... 0 1 0 1 0 1 0 1 ... ,
        // the 3rd-from-lowest bit of x 4-cycles ... 0 0 1 1 0 0 1 1 ... ,
        // the 4th-from-lowest bit of x has the 8-cycle ... 0 0 1 1 1 1 0 0 ...
        // ,
        // ...
        //
        // The generator's potency (min. s>=0 with (69069-1)^s = 0 mod 2^32) is
        // 16, which seems to be alright by p. 25, Sec. 3.2.1.3 of Knuth. It
        // also does well in the dimension 2..5 spectral tests, but it could be
        // better in dimension 6 (Line 15, Table 1, p. 106, Sec. 3.3.4, Knuth).
        //
        // Note that the random number user does not see the values generated
        // here directly since reloadMT() will always munge them first, so maybe
        // none of all of this matters. In fact, the seed values made here could
        // even be extra-special desirable if the Mersenne Twister theory says
        // so-- that's why the only change I made is to restrict to odd seeds.
        //

        long x = (seed | 1) & 0xFFFFFFFFl;
        // long s = state;
        long[] s = state;
        int si = 0;
        int j;

        // s++ is done with an array
        left = 0;
        s[si++] = x;
        for (j = N; j > 0; --j) {
            s[si] = (x *= 69069);
            s[si] &= 0xFFFFFFFFl;
            si++;
        }

        // for (int i = 0; i < s.length; i++) {
        // System.out.println(s[i]);
        // }
    }

    /**
     * reload the random number buffer
     * 
     * @return
     */
    public static int cokusreload() {
        int p0 = 0;
        int p2 = 2;
        int pM = M;
        long s0, s1;

        int j;

        if (left < -1)
            cokusseed(4357);

        left = N - 1;
        next = 1;

        // for(s0=state[0], s1=state[1], j=N-M+1; --j; s0=s1, s1=*p2++)
        // *p0++ = *pM++ ^ (mixBits(s0, s1) >> 1) ^ (loBit(s1) ? K : 0U);

        for (s0 = state[0], s1 = state[1], j = N - M + 1; --j != 0; s0 = s1, s1 = state[p2++]) {
            state[p0++] = state[pM++] ^ (mixBits(s0, s1) >> 1)
                ^ (loBit(s1) != 0 ? K : 0);
        }

        // for(pM=state, j=M; --j; s0=s1, s1=*p2++)
        // *p0++ = *pM++ ^ (mixBits(s0, s1) >> 1) ^ (loBit(s1) ? K : 0U);

        for (pM = 0, j = M; --j != 0; s0 = s1, s1 = state[p2++]) {
            state[p0++] = state[pM++] ^ (mixBits(s0, s1) >> 1)
                ^ (loBit(s1) != 0 ? K : 0);
        }

        s1 = state[0];
        state[p0] = state[pM] ^ (mixBits(s0, s1) >> 1)
            ^ (loBit(s1) != 0 ? K : 0);

        s1 ^= (s1 >> 11);
        s1 ^= (s1 << 7) & 0x9D2C5680l;
        s1 ^= (s1 << 15) & 0xEFC60000l;

        return (int) (s1 ^ (s1 >> 18));
    }

    public static int cokusrand() {
        long y;

        if (--left < 0)
            return (cokusreload());

        y = state[next++];
        y ^= (y >> 11);
        y ^= (y << 7) & 0x9D2C5680l;
        y ^= (y << 15) & 0xEFC60000l;
        return (int) (y ^ (y >> 18));
    }

    public static void main(String[] args) {

        cokusseed(4357);
        for (int i = 0; i < 50; i++) {
            int a = cokusrand();
            System.out.println((long) a & 0xFFFFFFFFl);
        }
    }
}
