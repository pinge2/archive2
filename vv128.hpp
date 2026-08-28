/*
vv128 Hash Algorithm
Copyright 2026 EtoPinge
SPDX-License-Identifier: Apache-2.0

DO NOT USE IN REAL CASES VIA IT'S POSSIBLE INSECURE
*/

#pragma once

#include "prng.hpp"
#include <iostream>
#include <vector>
#include <functional>

#ifndef NO_PRINT_MACRO
    #define print(x) std::cout << x << std::endl
#else
    #define print(x)
#endif

#define hex(x) std::hex << x << std::dec

#ifdef DEBUG
    #define debug(x) std::cout << x << std::endl;
#else
    #define debug(x)
#endif


#ifndef NO_ERRORS
    #define error(x) do{ \
        std::cout << "\033[31m[ERROR " << __FILE__ << ':' << __LINE__ << "]\033[0m " << x << std::endl; \
        exit(-1); \
    }while(0)

    #define ASSERT(x) do{ \
        if (!(x)) error("Assertion failed: " << #x); \
    }while(0)

#else
    #define error(x)
    #define ASSERT(x)
#endif


// By EtoPinge
uint64_t W(uint64_t a, uint64_t b){
    return (rol(a, 17) ^ b) + (a ^ rol(b, 19));
}


// By EtoPinge
uint64_t Ql(uint64_t a, uint64_t b){
    return rol(rol(a, 7) + (rol(b, 41) ^ a), 23);
}


struct vv128_state{
    uint64_t words[2];
    uint64_t a, b, c, d;
    uint64_t round_consts[16];
    uint64_t* ext_buffer;
    uint32_t length;
};


/*
Initializes vv128 state with salt, also generates round consts
*/
void vv128_init_state(vv128_state* state, uint64_t salt = 0){
    ASSERT(state != NULL);

    state->a = rol(0x6a09e667bb67ae85 + salt, 11);  // π
    state->b = rol(0x3c6ef372fe94f82b + salt, 23);  // e
    state->c = rol(0x9e3779b97f4a7c15 + salt, 37);  // φ
    state->d = rol(0x428a2f98d728ae22 + salt, 41);  // sqrt(2)

    Prng64 prng (salt);

    for (int i = 0; i < 16; ++i) state->round_consts[i] = prng.vvrand();
}


/*
Shedules blocks to words with state

block - a 256-bit block
*/
void vv128_word_shedule(vv128_state* state, uint64_t* block){
    ASSERT(state != NULL);
    ASSERT(block != NULL);
    
    state->words[0] = Ql(block[0], block[1]);
    state->words[1] = Ql(block[2], block[3]);

    state->words[0] ^= rol(state->words[0], 27);
    state->words[1] ^= rol(state->words[1], 29);
}


/*
VV128 round function

i - round number (0..12)
*/
void vv128_core(vv128_state* state, int i){
    ASSERT(state != NULL);

    uint64_t r1 = W(state->round_consts[i & 15], state->round_consts[-i & 15]);
    uint64_t r2 = W(state->round_consts[-i & 15], state->round_consts[i & 15]);

    uint64_t word1 = state->words[1] + mux(state->words[0], r1, r2);
    uint64_t word2 = state->words[0] + mux(state->words[1], r2, r1);

    state->a = Ql(word1, W(state->a, state->b)) + word2;
    state->b = Ql(word2, W(state->b, state->c)) + word1;
    state->c = Ql(word1, W(state->c, state->d)) + word2;
    state->d = Ql(word2, W(state->d, state->a)) + word1;
}


/*
Prints vv128 state - words, four state variables and constants

print_constants - prints 16 round constants if true
*/
void vv128_print_state(const vv128_state* state, bool print_consts = false){
    ASSERT(state != NULL);

    print("Word 1: " << hex(state->words[0]));
    print("Word 2: " << hex(state->words[1]));
    print("\ta = " << hex(state->a));
    print("\tb = " << hex(state->b));
    print("\tc = " << hex(state->c));
    print("\td = " << hex(state->d));

    if (print_consts){
        print("\nRound constants:");
        for (int i = 0; i < 16; ++i) print("\tConst " << i << " = " << hex(state->round_consts[i]));
    }
}


/*
Extends input string into zero-extended message

str - input string
len - length of string, strlen(str) if len == 0
*/
void vv128_extend(vv128_state* state, const char* str, size_t len = 0){
    ASSERT(state != NULL);
    ASSERT(str != NULL);

    if (len == 0) len = strlen(str);

    ASSERT(len != 0);

    uint64_t bit_len = len * 8;
    uint64_t padded_len = len + 1 + 8;
    
    uint64_t rem = padded_len & 31;
    if (rem != 0) padded_len += (32 - rem);
    
    uint64_t val = padded_len >> 5;
    
    state->ext_buffer = (uint64_t*)malloc(32 * val);
    memset(state->ext_buffer, 0, 32 * val);
    memcpy(state->ext_buffer, str, len);
    
    ((uint8_t*)state->ext_buffer)[len] = 0x80;
    
    uint64_t* len_ptr = (uint64_t*)((uint8_t*)state->ext_buffer + padded_len - 8);
    *len_ptr = bit_len;

    state->length = val;
}


/*
Makes finale diffuse

res - 256-bit result hash
*/
void vv128_finalize(const vv128_state* state, uint64_t* res){
    ASSERT(state != NULL);
    ASSERT(res != NULL);

    uint64_t ctrl = Ql(~state->length, state->round_consts[state->length & 15]);

    res[0] = Ql(state->a, W(ctrl, state->b));
    res[1] = Ql(state->b, W(ctrl, state->c));
    res[2] = Ql(state->c, W(ctrl, state->d));
    res[3] = Ql(state->d, W(ctrl, state->a));
}


/*
Clears v128 state, including buffer and consts

free_buffer - frees inner buffer message if true
*/
void vv128_clear_state(vv128_state* state, bool free_buffer = false){
    ASSERT(state != NULL);

    if (free_buffer){
        ASSERT(state->ext_buffer != NULL);
        free(state->ext_buffer);
    }

    vv128_init_state(state, 0);
}


/*
Core vv128 hash function

state - an intialized vv128 state
res - 256-bit result hash
*/
void vv128(vv128_state* state, uint64_t* res){
    ASSERT(state != NULL);
    ASSERT(res != NULL);

    for (int j = 0; j < state->length * 4; j += 4){
        vv128_word_shedule(state, state->ext_buffer + j);

        for (int i = 0; i < 12; ++i) vv128_core(state, i);
    }

    vv128_finalize(state, res);
}