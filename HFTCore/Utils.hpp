#pragma once
#include <cstdint>
#include <intrin.h>
#include <Windows.h>

inline void pinThread(int cpu) {
    // Set thread affinity mask to single CPU core
    DWORD_PTR mask = DWORD_PTR(1) << cpu;
    SetThreadAffinityMask(GetCurrentThread(), mask);
}

inline uint64_t rdtsc() {
    return __rdtsc();
}
