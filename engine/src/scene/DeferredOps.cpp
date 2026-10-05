// =============================================================================
//  DeferredOps.cpp - a skeleton. Every function is here with the right
//  signature and an empty body. DeferredOps.h is the specification; read it
//  first.
//
//  Nothing structural happens immediately. Destroying goes into a queue applied
//  at ONE point in the frame, because removing an entity while a system is
//  walking its list is not an error in C++ - it is a crash months later.
// =============================================================================

#include <engine/core/Log.h>
#include <engine/scene/DeferredOps.h>
#include <engine/scene/Scene.h>

#include <algorithm>
#include <set>
#include <vector>

namespace eng {

    namespace {

std::vector<EntityId> g_destroys;

// The same set of ids as g_destroys, kept separately so that "is this one
// already queued?" is a quick lookup rather than a search through the whole
// list. std::set can do that because EntityId knows how to compare itself -
// see the <=> line in EntityId.h.
std::set<EntityId> g_pendingDestroy;

} // namespace
// Asks for an entity to be destroyed at the end of the current step. Asking
// twice is harmless, because game code does it constantly.
void DeferredOps::QueueDestroy(EntityId id) {
    if (id.IsNull()) {
        return;
    }
    // RULE 3: queueing the same destroy twice is ignored rather than treated
    // as a mistake. insert() returns a pair whose .second says whether
    // anything was actually added, which makes this a one-line check.
    if (!g_pendingDestroy.insert(id).second) {
        return;
    }
    g_destroys.push_back(id);
}

// Is this entity already on its way out? Systems that must not act on something
// already dying check this - a destroyed thing should stop colliding at once.
bool DeferredOps::IsPendingDestroy(EntityId id) {
    return !id.IsNull() && g_pendingDestroy.contains(id);
}

// Applies everything queued, once. Anything queued while draining belongs to
// the next frame - draining until empty risks never finishing, because
// something that spawns a copy of itself is reasonable to write.
void DeferredOps::Apply(Scene& scene) {
    // RULE 4: take the queue away and drain the copy ONCE. Anything queued
    // while draining lands in the now-empty original and happens next frame.
    // swap() is used because it hands over the contents without copying them -
    // it just exchanges what the two vectors point at.
    std::vector<EntityId> destroys;
    destroys.swap(g_destroys);

    for (const EntityId id : destroys) {
        // Checked again: something else may have destroyed it between the
        // queueing and now. Being able to ask that question at all is exactly
        // what the generation number in EntityId is for.
        if (scene.IsValid(id)) {
            scene.DestroyEntityImmediate(id);
        }
    }

    g_pendingDestroy.clear();
}

// Throws the queue away without applying it, used when a scene is unloaded.
void DeferredOps::Clear() {
    g_destroys.clear();
    g_pendingDestroy.clear();
}

// How many destroys are waiting.
std::size_t DeferredOps::PendingDestroyCount() {
    return g_destroys.size();
}

} // namespace eng
