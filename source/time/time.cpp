#include <SDL3/SDL.h>
#include <time/time.hpp>

namespace paranoixa {
float Time::seconds() { return static_cast<float>(SDL_GetTicks() / 1000.f); }

uint32_t Time::milli() { return SDL_GetTicks(); }
} // namespace paranoixa
