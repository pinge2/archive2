#pragma once

#define SDL_MAIN_HANDLED
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <SDL2/SDL.h>

#define print(x) std::cout<<x<<std::endl

#ifdef DEBUG
    #define debug(x) std::cout<<"[DEBUG:"<<__LINE__<<"] "<<x<<std::endl
#else
    #define debug(x)
#endif