//
// Created by Dima on 24.02.2024.
//

#include "UUID.h"
#include <random>

static std::random_device s_RandomDevice;
static std::mt19937_64 s_RandomEngine(s_RandomDevice());
static std::uniform_int_distribution<uint64_t> s_UniformDistribution;

MusicPlayer::UUID::UUID()
: m_UUID(s_UniformDistribution(s_RandomEngine))
{

}
