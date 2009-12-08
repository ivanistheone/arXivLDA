#include <stdio.h>

//
// uint32 must be an unsigned integer type capable of holding at least 32
// bits; exactly 32 should be fastest, but 64 is better on an Alpha with
// GCC at -O3 optimization so try your options and see what's best for you
//

typedef unsigned long uint32;

/* Period parameters */  
#define N 624			// length of state vector
#define M 397			// a period parameter
#define K 0x9908b0dfUL		// a magic constant 
#define hiBit(u)       ((u) & 0x80000000U)   // mask all but highest   bit of u
#define loBit(u)       ((u) & 0x00000001U)   // mask all but lowest    bit of u
#define loBits(u)      ((u) & 0x7FFFFFFFU)   // mask     the highest   bit of u
#define mixBits(u, v)  (hiBit(u)|loBits(v))  // move hi bit of u to hi bit of v

static uint32   state[N+1];     // state vector + 1 extra to not violate ANSI C
static uint32   *next;          // next random value is computed from here
static int      left = -1;      // can *next++ this many times before reloading

/**
 * Cokus versions/optimizations
 **/

void cokusseed(int seed)
 {
    //
    // We initialize state[0..(N-1)] via the generator
    //
    //   x_new = (69069 * x_old) mod 2^32
    //
    // from Line 15 of Table 1, p. 106, Sec. 3.3.4 of Knuth's
    // _The Art of Computer Programming_, Volume 2, 3rd ed.
    //
    // Notes (SJC): I do not know what the initial state requirements
    // of the Mersenne Twister are, but it seems this seeding generator
    // could be better.  It achieves the maximum period for its modulus
    // (2^30) iff x_initial is odd (p. 20-21, Sec. 3.2.1.2, Knuth); if
    // x_initial can be even, you have sequences like 0, 0, 0, ...;
    // 2^31, 2^31, 2^31, ...; 2^30, 2^30, 2^30, ...; 2^29, 2^29 + 2^31,
    // 2^29, 2^29 + 2^31, ..., etc. so I force seed to be odd below.
    //
    // Even if x_initial is odd, if x_initial is 1 mod 4 then
    //
    //   the          lowest bit of x is always 1,
    //   the  next-to-lowest bit of x is always 0,
    //   the 2nd-from-lowest bit of x alternates      ... 0 1 0 1 0 1 0 1 ... ,
    //   the 3rd-from-lowest bit of x 4-cycles        ... 0 1 1 0 0 1 1 0 ... ,
    //   the 4th-from-lowest bit of x has the 8-cycle ... 0 0 0 1 1 1 1 0 ... ,
    //    ...
    //
    // and if x_initial is 3 mod 4 then
    //
    //   the          lowest bit of x is always 1,
    //   the  next-to-lowest bit of x is always 1,
    //   the 2nd-from-lowest bit of x alternates      ... 0 1 0 1 0 1 0 1 ... ,
    //   the 3rd-from-lowest bit of x 4-cycles        ... 0 0 1 1 0 0 1 1 ... ,
    //   the 4th-from-lowest bit of x has the 8-cycle ... 0 0 1 1 1 1 0 0 ... ,
    //    ...
    //
    // The generator's potency (min. s>=0 with (69069-1)^s = 0 mod 2^32) is
    // 16, which seems to be alright by p. 25, Sec. 3.2.1.3 of Knuth.  It
    // also does well in the dimension 2..5 spectral tests, but it could be
    // better in dimension 6 (Line 15, Table 1, p. 106, Sec. 3.3.4, Knuth).
    //
    // Note that the random number user does not see the values generated
    // here directly since reloadMT() will always munge them first, so maybe
    // none of all of this matters.  In fact, the seed values made here could
    // even be extra-special desirable if the Mersenne Twister theory says
    // so-- that's why the only change I made is to restrict to odd seeds.
    //

    register uint32 x = (seed | 1U) & 0xFFFFFFFFU, *s = state;
    register int    j;

    //for(left=0, *s++=x, j=N; --j;
    //    *s++ = (x*=69069U) & 0xFFFFFFFFU);
    
    left = 0;
    *s++ = x;
    for(j = N; j > 0; --j) 
        *s++ = (x*=69069U) & 0xFFFFFFFFU;

 }

 int cokusreload() 
 {
    register uint32 *p0=state, *p2=state+2, *pM=state+M, s0, s1;
    register int    j;

    if(left < -1)
        cokusseed(4357U);

    left=N-1, next=state+1;

    //for(s0=state[0], s1=state[1], j=N-M+1; --j; s0=s1, s1=*p2++)
    //    *p0++ = *pM++ ^ (mixBits(s0, s1) >> 1) ^ (loBit(s1) ? K : 0U);

    //for(j = 0; j < N-M+1; j++)
    //    printf("%u\n", state[j]);


    s0=state[0], s1=state[1];
    for(j= N - M; j>0; --j) {
        *p0++ = *pM++ ^ (mixBits(s0, s1) >> 1) ^ (loBit(s1) ? K : 0U);
        s0=s1, s1=*p2++;
    }

    //for(pM=state, j=M; --j; s0=s1, s1=*p2++)
    //    *p0++ = *pM++ ^ (mixBits(s0, s1) >> 1) ^ (loBit(s1) ? K : 0U);

    pM=state;
    for(j=M; j>0; --j) {
        *p0++ = *pM++ ^ (mixBits(s0, s1) >> 1) ^ (loBit(s1) ? K : 0U);
        s0=s1, s1=*p2++;
    }
    //printf("\n\n");
    //for(j = N - M; j < N; j++)
    //    printf("%u\n", state[j]);


    s1=state[0], *p0 = *pM ^ (mixBits(s0, s1) >> 1) ^ (loBit(s1) ? K : 0U);
    s1 ^= (s1 >> 11);
    s1 ^= (s1 <<  7) & 0x9D2C5680U;
    s1 ^= (s1 << 15) & 0xEFC60000U;

    return(s1 ^ (s1 >> 18));
 }


int cokusrand() 
 {
    uint32 y;

    if(--left < 0)
        return(cokusreload());

    y  = *next++;
    y ^= (y >> 11);
    y ^= (y <<  7) & 0x9D2C5680U;
    y ^= (y << 15) & 0xEFC60000U;
    return (int)(y ^ (y >> 18));
 } 


int main(char **argv, int argc) 
{
    unsigned int i, a;
    //unsigned short x = -128;
    //printf("%u\t\t%d\n", x, x);
    cokusseed(4357);
    cokusreload();
    //for(i = 0; i < 50; i++)
    //{
    //    a = cokusrand();
    //    printf("%u\t\t%d\n", a, a);

    //}
    

}
