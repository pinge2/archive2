#pragma once

#include <cstdint>
#include <bit>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <cstring>

#define PHI 0x9e3779b97f4a7c15
#define QMAG 0x9ddfea08eb382d69
#define MIXMAG PHI

#define SQ2 0x6a09e667f3bcc908
#define SQ3 0xbb67ae8584caa73b
#define SQ5 0x3c6ef372fe94f82b
#define SQ7 0xa54ff53a5f1d36f1


uint64_t time_ns(){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint64_t)ts.tv_sec * 1000000000 + ts.tv_nsec;
}


uint64_t mux(uint64_t s, uint64_t a, uint64_t b){
    return (a & ~s) | (b & s);
}


uint64_t rol(uint64_t a, uint64_t b){
    return std::__rotl(a, (int)b);
}


uint64_t popcnt(uint64_t n){
    return std::__popcount(n);
}


/*
A class with fast and good PRNG generators with the state of 64 bits
*/
class Prng64{
    uint64_t state = 0;

public:
    Prng64(uint64_t seed = 0){
        setstate(seed);
    }

    /*
    Updates state with a seed and warm-ups PRNG

    `warm` - Calls warmup() on vvrand if true
    */
    void setstate(uint64_t seed = 0, bool warm = true){
        state = seed * PHI + QMAG;
        
        if (warm) warmup();
    }

    /*
    Jumps on the delta value (for PRNG with counter-based state)

    `delta` - A jump value up to UINT64_MAX
    */
    void jump(uint64_t delta){
        state += delta * MIXMAG;
    }

    /*
    Warm-ups generator with the specified PRNG

    `fn` - An PRNG function, vvrand for default
    `rounds` - How much times to call, 6 for default
    */
    void warmup(uint64_t (Prng64::*fn)() = &Prng64::vvrand, int rounds = 6){
        for (int i = 0; i < rounds; ++i) (this->*fn)();
    }

    /*
    Resets state for counter-based PRNG

    `warm_up` - A Prng64 function for warming up. vvrand for default
    */
    void reset(uint64_t (Prng64::*warm_up)() = &Prng64::vvrand){
        state = 0;
        (this->*warm_up)();
    }

    /*
    [Counter-based]
    High-quality industrial PRNG

    Entropy: 4.5/5
    Speed: 1.5/5
    */
    uint64_t splitmix64(){
        state += MIXMAG;
        uint64_t z = state;

        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
        z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
        z ^= (z >> 31);

        return z;
    }

    /*
    State-based version of SplitMix64

    Entropy: 4.5/5
    Speed: 1.5/5
    */
    uint64_t statemix64(){
        uint64_t result = state;

        state = (state ^ (state >> 30)) * 0xbf58476d1ce4e5b9;
        state = (state ^ (state >> 27)) * 0x94d049bb133111eb;
        state ^= (state >> 31);

        return result;
    }

    /*
    [Counter-based]
    By EtoPinge
    Modified version of SplitMix64 based on roladd construction with excellent quality

    Entropy: 4.5/5
    Speed: 2.5/5
    */
    uint64_t roladdmix(){
        state += MIXMAG;
        uint64_t z = state;
        
        z = (z ^ rol(z, 29)) + rol(z, 23);
        z = (z ^ rol(z, 41)) + rol(z, 37);
        z = (z ^ rol(z, 31)) + rol(z, 19);
        
        return z;
    }

    /*
    Linear PRNG, but fast and enough good quality

    Entropy: 4/5
    Speed: 3.5/5
    */
    uint64_t xorshift64(){
        uint64_t result = state;

        state ^= state << 13;
        state ^= state >> 29;
        state ^= state << 17;

        return result;
    }

    /*
    Linear Conguent Generator - do not use at production via its bad quality

    Entropy: 2/5
    Speed: 4/5
    */
    uint64_t lcg(){
        uint64_t result = state;

        state = state * PHI + QMAG;

        return result;
    }

    /*
    Linear Conguent Generator with rotating in the multiply

    Entropy: 4/5
    Speed: 4/5
    */
    uint64_t rollcg(){
        uint64_t result = state;

        state = rol(state * PHI, 31) + QMAG;

        return result;
    }

    /*
    Permutation Conguent Generator - xor-shift version

    Entropy: 3.5/5
    Speed: 3.5/5
    */
    uint64_t pcg_xsh(){
        uint64_t result = state;

        state = state * PHI + QMAG;
        state ^= state >> 27;

        return result;
    }

    /*
    Permutation Conguent Generator - rol-add version

    Entropy: 4/5
    Speed: 3.5/5
    */
    uint64_t pcg_roladd(){
        uint64_t result = state;

        state = state * PHI + QMAG;
        state += rol(state, 27);

        return result;
    }

    /*
    By EtoPinge
    Extremely fast and quite good quality

    Entropy: 4/5
    Speed: 5/5
    */
    uint64_t hrand(){
        uint64_t result = state;

        uint64_t a0 = rol(state, 19);
        uint64_t a1 = rol(state, 29);
        uint64_t a2 = rol(state, 47);
        uint64_t a3 = state;

        state = (a0 + a1) + (a2 + a3);

        return result;
    }

    /*
    By EtoPinge
    Excellent quality and 2x faster than SplitMix64

    Entropy: 5/5
    Speed: 3/5
    */
    uint64_t vvrand(){
        state += MIXMAG;
        uint64_t z = state;

        z *= rol(z, 23);
        z += rol(z, 41);
        z ^= rol(z, 27);

        return z;
    }

    /*
    By EtoPinge

    Entropy: 4/5
    Speed: 4/5
    */
    uint64_t arxrand(){
        uint64_t result = state;

        state ^= rol(state, 27);
        state += rol(state, 37);

        return result;
    }

    /*
    Linear adder, not a PRNG

    Entropy: 1/5
    Speed: 5/5
    */
    uint64_t adder(){
        state += QMAG;
        return state;
    }
};


/*
A class with high period PRNG generators with the state of 256 bits
*/
class Prng256{
    uint64_t state[4] = {0};

public:
    Prng256(uint64_t seed = 0){
        setstate(seed);
    }

    /*
    Updates state with a seed and warm-ups PRNG

    `warm` - Calls warmup() on wqrand if true
    */
    void setstate(uint64_t seed = 0, bool warm = true){
        state[0] = seed * SQ2 + QMAG;
        state[1] = seed * SQ3 + QMAG;
        state[2] = seed * SQ5 + QMAG;
        state[3] = seed * SQ7 + QMAG;
        
        if (warm) warmup();
    }

    /*
    Warm-ups generator with the specified PRNG

    `fn` - An PRNG function, wqrand for default
    `rounds` - How much times to call, 8 for default
    */
    void warmup(uint64_t (Prng256::*fn)() = &Prng256::wqrand, int rounds = 8){
        for (int i = 0; i < rounds; ++i) (this->*fn)();
    }

    /*
    Resets state for counter-based PRNG

    `warm_up` - A Prng256 function for warming up. wqrand for default
    */
    void reset(uint64_t (Prng256::*warm_up)() = &Prng256::wqrand){
        std::memset(state, 0, 32);

        (this->*warm_up)();
    }

    /*
    Get n-th state

    `n` - index of 64-bit word in the state, must be 0..3
    */
    uint64_t getstate(int n = 0){
        return state[n];
    }

    /*
    WQRand by EtoPinge
    */
    uint64_t wqrand(){
        uint64_t result = state[0];

        uint64_t old = state[0];
        state[0] = rol(state[1], 23) + SQ7;
        state[1] = rol(state[2], 29) + SQ5;
        state[2] = rol(state[3], 31) + SQ3;
        state[3] = rol(old * PHI, 37) + QMAG;

        return result;
    }
};