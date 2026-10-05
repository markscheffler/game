// =============================================================================
//  Transform2D.cpp - a skeleton. Every function is here with the right
//  signature and an empty body. Transform2D.h is the specification; read it
//  before filling one in.
// =============================================================================

#include <engine/math/Transform2D.h>
#include <engine/core/Log.h>


namespace eng {

// A transform being destroyed hands its children back to the world, so none of
// them is left pointing at something that no longer exists.
Transform2D::~Transform2D() {
    // Rule 1: hand the children back to the world rather than destroying them
    // or leaving them pointing at memory that is about to disappear.
    DetachChildren();

    // And take this node off its own parent's child list, so the parent is not
    // left holding a pointer to something that no longer exists.
    if (m_parent != nullptr) {
        m_parent->RemoveChild(this);
        m_parent = nullptr;
    }
}

// Records a child in this transform's list. Called by SetParent, not directly.
void Transform2D::AddChild(Transform2D* child) {
    m_children.push_back(child);
}

// Takes a child back out of this transform's list.
void Transform2D::RemoveChild(Transform2D* child) {
    // std::erase removes every matching element from a container, and does
    // nothing when there is no match - so there is no "did we find it?" step to
    // get wrong. A child appears in this list once, so "every match" is one.
    std::erase(m_children, child);
}

// Attaches this transform to a parent, so it moves when the parent moves.
// keepWorldTransform decides whether it stays where it visibly is, or keeps its
// local numbers and jumps.
void Transform2D::SetParent(Transform2D* parent, bool keepWorldTransform) {
    if (parent == m_parent) {
        return; // nothing to do
    }

    // Rule 2: refuse to build a loop. Walking up the proposed parent's chain
    // costs almost nothing, and it turns a permanent freeze inside the
    // renderer into a message in the Console at the moment the mistake is made.
    if (parent != nullptr) {
        if (parent == this || parent->IsDescendantOf(this)) {
            ENGINE_LOG_ERROR(Channels::kScene,
                             "refused to reparent a transform under itself or one of "
                             "its own children - that would make a loop");
            return;
        }
    }

    // Remember where the object currently looks, before the parent changes.
    const Mat3 worldBefore = keepWorldTransform ? WorldMatrix() : Mat3::Identity();

    if (m_parent != nullptr) {
        m_parent->RemoveChild(this);
    }
    m_parent = parent;
    if (m_parent != nullptr) {
        m_parent->AddChild(this);
    }

    if (keepWorldTransform) {
        // Work out what local position/rotation/scale would put the object
        // back exactly where it was: take the world transform it had, and undo
        // the new parent's transform from it.
        const Mat3 parentWorld = (m_parent != nullptr) ? m_parent->WorldMatrix() : Mat3::Identity();
        const Mat3 local = worldBefore * parentWorld.Inverse();
        m_position = local.GetTranslation();
        m_rotation = local.GetRotation();
        m_scale = local.GetScale();
    }
}

// Releases every child to the world, leaving each one where it visibly is.
void Transform2D::DetachChildren() {
    // The list is COPIED before it is walked. SetParent below calls
    // RemoveChild on this node, which erases from m_children - and modifying a
    // container while looping over it is how you end up reading freed memory.
    // Iterating a copy sidesteps that completely.
    const std::vector<Transform2D*> children = m_children;
    for (Transform2D* child : children) {
        child->SetParent(nullptr, /*keepWorldTransform=*/true);
    }
    m_children.clear();
}

// How many parents there are above this one. Zero means it is at the top.
int Transform2D::Depth() const {
    int depth = 0;
    for (const Transform2D* node = m_parent; node != nullptr; node = node->m_parent) {
        ++depth;
    }
    return depth;
}

// Is this transform somewhere below the candidate? Checked before parenting, so
// a transform cannot be made its own grandparent.
bool Transform2D::IsDescendantOf(const Transform2D* candidate) const {
    if (candidate == nullptr) {
        return false;
    }
    for (const Transform2D* node = m_parent; node != nullptr; node = node->m_parent) {
        if (node == candidate) {
            return true;
        }
    }
    return false;
}

// This transform's own position, rotation and scale, as one matrix - ignoring
// any parent.
Mat3 Transform2D::LocalMatrix() const {
    return Mat3::FromTRS(m_position, m_rotation, m_scale);
}

// This transform combined with every parent above it. This is what puts a moon
// in orbit around a planet that is itself orbiting a sun.
Mat3 Transform2D::WorldMatrix() const {
    // Start with this node's own transform, then apply each parent in turn
    // going outward. Under this engine's convention (see Mat3.h) "do local,
    // then the parent" is written local * parent, which is the same order it
    // reads in.
    //
    // This walks the whole chain every time it is called rather than caching
    // the answer. That is deliberate: caching means remembering to invalidate
    // the cache every time anything moves, and a stale transform is a much
    // harder bug than a slightly slower one. Scenes here are small enough that
    // it does not matter.
    Mat3 result = LocalMatrix();
    for (const Transform2D* node = m_parent; node != nullptr; node = node->m_parent) {
        result = result * node->LocalMatrix();
    }
    return result;
}

// Where this transform actually is in the world, parents included.
Vec2 Transform2D::WorldPosition() const {
    return WorldMatrix().GetTranslation();
}

// Which way it is actually facing in the world, parents included.
float Transform2D::WorldRotation() const {
    // Rotations simply add up the chain, so there is no need to build a matrix
    // and pull the angle back out of it.
    float total = m_rotation;
    for (const Transform2D* node = m_parent; node != nullptr; node = node->m_parent) {
        total += node->m_rotation;
    }
    return total;
}

// How big it actually is in the world, parents included.
Vec2 Transform2D::WorldScale() const {
    return WorldMatrix().GetScale();
}

// Puts this transform at a world position, working out the local position that
// produces it under whatever parent it has.
void Transform2D::SetWorldPosition(Vec2 world) {
    if (m_parent == nullptr) {
        // With no parent, local and world are the same thing.
        m_position = world;
        return;
    }
    // With a parent, undo the parent's transform to find the local position
    // that lands on the requested world position.
    m_position = m_parent->WorldMatrix().Inverse().TransformPoint(world);
}

// Converts a point from this transform's own space into world space.
Vec2 Transform2D::LocalToWorldPoint(Vec2 local) const {
    return WorldMatrix().TransformPoint(local);
}

// Converts a world point into this transform's own space.
Vec2 Transform2D::WorldToLocalPoint(Vec2 world) const {
    return WorldMatrix().Inverse().TransformPoint(world);
}

// Converts a direction out of this transform's space. Unlike a point, a
// direction ignores the move part.
Vec2 Transform2D::LocalToWorldVector(Vec2 local) const {
    return WorldMatrix().TransformVector(local);
}

// Converts a world direction into this transform's space.
Vec2 Transform2D::WorldToLocalVector(Vec2 world) const {
    return WorldMatrix().Inverse().TransformVector(world);
}

} // namespace eng
