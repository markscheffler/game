// =============================================================================
//  Random.cpp - a skeleton. Every function is here with the right signature and
//  an empty body. Random.h is the specification; read it before filling one in.
// =============================================================================

#include <engine/core/Log.h>
#include <engine/math/Random.h>
#include <engine/math/Vec2.h>

#include <cmath>
#include <utility>

namespace eng {

// A whole number from lo to hiInclusive, both ends possible. NextInt(1, 6) is a
// six-sided die.
int Random::NextInt(int lo, int hiInclusive) {
    if (lo > hiInclusive) {
        // Rather than return something arbitrary, say so - a reversed range is
        // almost always a typo in the calling code - and then carry on with
        // the range the caller probably meant.
        ENGINE_LOG_WARN(Channels::kCore,
                        "Random::NextInt called with lo={} greater than hi={}; "
                        "swapping them",
                        lo, hiInclusive);
        std::swap(lo, hiInclusive);
    }

    // std::uniform_int_distribution gives every value in the range an equal
    // chance. Writing `m_engine() % range` by hand instead is very slightly
    // biased towards the low numbers and is the classic beginner mistake here.
    std::uniform_int_distribution<int> distribution(lo, hiInclusive);
    return distribution(m_engine);
}

// A decimal from 0 up to, but never exactly, 1.
float Random::NextFloat01() {
    // The range is written as [0, 1) - 0 is possible, 1 is not. That is the
    // convention every random-float API uses, and it is what makes
    // `array[(int)(NextFloat01() * size)]` safe.
    std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
    return distribution(m_engine);
}

// A decimal somewhere between lo and hi.
float Random::NextRange(float lo, float hi) {
    if (lo > hi) {
        std::swap(lo, hi);
    }
    std::uniform_real_distribution<float> distribution(lo, hi);
    return distribution(m_engine);
}

// A coin flip.
bool Random::NextBool() {
    // std::bernoulli_distribution is the standard "true with probability p"
    // distribution; 0.5 makes it a fair coin.
    std::bernoulli_distribution distribution(0.5);
    return distribution(m_engine);
}

// A direction: a vector of length 1 pointing at a random angle. Useful for
// scattering things without them drifting towards one corner.
Random::UnitVector Random::NextDirection() {
    // Pick an angle anywhere around the circle, then convert it to x and y.
    // The result always has length 1, which is what "a direction" means.
    const float angle = NextRange(0.0f, kTwoPi);
    return UnitVector{std::cos(angle), std::sin(angle)};
}

// The one shared generator, for code that does not need its own sequence.
// Seeded from a fixed number rather than the clock, so a bug can be reproduced.
Random& GlobalRandom() {
    static Random instance;
    return instance;
}

} // namespace eng
