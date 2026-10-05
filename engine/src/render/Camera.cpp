// =============================================================================
//  Camera.cpp - a skeleton. Every function is here with the right signature and
//  an empty body. Camera.h is the specification; read it before filling one in.
// =============================================================================

#include <engine/render/Camera.h>
#include <algorithm>

namespace eng {

// Sets how close the camera is. A zoom of zero or less would divide by zero
// later, so it has to be kept above it.
void Camera::SetZoom(float zoom) {
    // std::clamp keeps the value inside a range. The lower bound stops the
    // view matrix from becoming impossible to invert; the upper bound stops a
    // stray scroll wheel from zooming so far that positions lose precision.
    m_zoom = std::clamp(zoom, 0.01f, 1000.0f);
}

// Puts the camera back at the origin at normal zoom.
void Camera::Reset() {
    m_position = Vec2{0.0f, 0.0f};
    m_zoom = 1.0f;
}

// The camera as a matrix, BACKWARDS: it undoes placing something at the
// camera's position and scaling it by the zoom.
//
// THIS IS ALSO THE ONE PLACE THE Y AXIS FLIPS. The world is y-up and the screen
// is y-down; flip it anywhere else as well and the two cancel out, which looks
// almost right and is very hard to find.
Mat3 Camera::ViewMatrix() const {
    const Vec2 half{m_viewport.x * 0.5f, m_viewport.y * 0.5f};

    // Read left to right, this says exactly what it does:
    //   1. slide the world so the camera's position sits at the origin
    //   2. scale by the zoom, and FLIP Y (that is the -m_zoom)
    //   3. slide the origin to the middle of the picture
    //
    // The negative y is the single y flip in the engine. See Camera.h.
    return Mat3::Translation(-m_position) * Mat3::Scaling(Vec2{m_zoom, -m_zoom}) *
           Mat3::Translation(half);
}

// The matrix that undoes the view - what turns a screen position back into a
// world one.
Mat3 Camera::InverseViewMatrix() const {
    return ViewMatrix().Inverse();
}

// Where a point in the world appears on screen.
Vec2 Camera::WorldToScreen(Vec2 world) const {
    return ViewMatrix().TransformPoint(world);
}

// Where a point on screen is in the world. This is what makes clicking on
// something work.
Vec2 Camera::ScreenToWorld(Vec2 screen) const {
    // This is what makes clicking on things work. Because the camera is a
    // matrix, "undo the camera" is literally the inverse matrix - there is no
    // second piece of code that has to be kept in step with ViewMatrix.
    return InverseViewMatrix().TransformPoint(screen);
}

// Converts a world DIRECTION or size into screen units. Unlike a point, it
// ignores where the camera is and only takes the zoom and the y flip.
Vec2 Camera::WorldToScreenVector(Vec2 world) const {
    // TransformVector rather than TransformPoint: a size or a direction should
    // not be shifted by where the camera happens to be looking.
    return ViewMatrix().TransformVector(world);
}

// The rectangle of world currently on screen - useful for skipping anything
// outside it.
AABB Camera::VisibleBounds() const {
    // Push all four corners of the screen back out into the world and take the
    // box around them.
    //
    // Doing it this way rather than "viewport divided by zoom" costs nothing
    // and keeps working unchanged on the day the camera learns to rotate.
    const Mat3 inverse = InverseViewMatrix();

    const Vec2 corners[4] = {
        inverse.TransformPoint(Vec2{0.0f, 0.0f}),
        inverse.TransformPoint(Vec2{m_viewport.x, 0.0f}),
        inverse.TransformPoint(Vec2{0.0f, m_viewport.y}),
        inverse.TransformPoint(Vec2{m_viewport.x, m_viewport.y}),
    };

    AABB bounds{corners[0], corners[0]};
    for (int i = 1; i < 4; ++i) {
        bounds.Encapsulate(corners[i]);
    }
    return bounds;
}

} // namespace eng
