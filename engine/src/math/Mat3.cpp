// =============================================================================
//  Mat3.cpp - a skeleton. Every function is here with the right signature and
//  an empty body. Mat3.h is the specification; read it before filling one in.
// =============================================================================

#include <engine/math/Mat3.h>
#include <engine/core/Log.h>
namespace eng {

// The "do nothing" matrix. Every other matrix here is built by changing a copy
// of this one, so it is worth writing first.
Mat3 Mat3::Identity() {
    Mat3 result;
    result.m[0][0] = 1.0f;
    result.m[0][1] = 0.0f;
    result.m[0][2] = 0.0f;
    result.m[1][0] = 0.0f;
    result.m[1][1] = 1.0f;
    result.m[1][2] = 0.0f;
    result.m[2][0] = 0.0f;
    result.m[2][1] = 0.0f;
    result.m[2][2] = 1.0f;
    return result;
}

// A matrix that moves a point by t. The move part lives in the BOTTOM ROW,
// because points are written as rows in this engine.
Mat3 Mat3::Translation(Vec2 t) {
    Mat3 result = Identity();
    // The BOTTOM ROW holds the move, because points are written as rows here.
    result.m[2][0] = t.x;
    result.m[2][1] = t.y;
    return result;
}

// A matrix that turns a point anticlockwise about the origin.
Mat3 Mat3::Rotation(float radians) {
    const float c = std::cos(radians);
    const float s = std::sin(radians);

    Mat3 result = Identity();
    //  [  c  s  0 ]   Multiplying the row (1, 0) by this gives (c, s), so a
    //  [ -s  c  0 ]   positive angle turns anticlockwise in a y-up world -
    //  [  0  0  1 ]   which is what the rest of the engine assumes.
    result.m[0][0] = c;
    result.m[0][1] = s;
    result.m[1][0] = -s;
    result.m[1][1] = c;
    return result;
}

// A matrix that resizes about the origin.
Mat3 Mat3::Scaling(Vec2 s) {
    Mat3 result = Identity();
    result.m[0][0] = s.x;
    result.m[1][1] = s.y;
    return result;
}

// Scale, then rotate, then move - combined into one matrix. Every object's
// world position goes through this every frame.
Mat3 Mat3::FromTRS(Vec2 translation, float radians, Vec2 scale) {
    // This is Scaling(scale) * Rotation(radians) * Translation(translation)
    // with the multiplication already worked out on paper. It is written
    // longhand because every object asks for its world matrix every frame, and
    // the straightforward version builds three whole matrices and multiplies
    // them just to fill in six numbers.
    const float c = std::cos(radians);
    const float s = std::sin(radians);

    Mat3 result;
    result.m[0][0] = scale.x * c;
    result.m[0][1] = scale.x * s;
    result.m[0][2] = 0.0f;
    result.m[1][0] = scale.y * -s;
    result.m[1][1] = scale.y * c;
    result.m[1][2] = 0.0f;
    result.m[2][0] = translation.x;
    result.m[2][1] = translation.y;
    result.m[2][2] = 1.0f;
    return result;
}

// Transforms a POSITION: the move part applies, so a point slides with the space.
Vec2 Mat3::TransformPoint(Vec2 point) const {
    // The row [x y 1] multiplied by this matrix, keeping the first two results.
    // The 1 on the end is what picks up the bottom row, which is exactly why
    // the move applies to a point.
    return Vec2{point.x * m[0][0] + point.y * m[1][0] + m[2][0],
                point.x * m[0][1] + point.y * m[1][1] + m[2][1]};
}

// Transforms a DIRECTION: the move part does NOT apply, because sliding the
// world sideways does not change which way something is facing.
Vec2 Mat3::TransformVector(Vec2 direction) const {
    // The row [x y 0]. The 0 is the whole difference from TransformPoint: it
    // multiplies the bottom row away, so the move is ignored and only the
    // turn and the resize apply.
    return Vec2{direction.x * m[0][0] + direction.y * m[1][0],
                direction.x * m[0][1] + direction.y * m[1][1]};
}

// The matrix that undoes this one. Turning a mouse click back into a world
// position is this, and nothing else.
Mat3 Mat3::Inverse() const {
    // Every matrix this engine builds has (0, 0, 1) as its third column - that
    // is what "made only of moves, turns and resizes" means. Knowing that
    // turns a full 3x3 inverse into a small 2x2 one plus an adjusted offset.
    const float a = m[0][0];
    const float b = m[0][1];
    const float c = m[1][0];
    const float d = m[1][1];

    // The determinant of the 2x2 part. When it is zero the matrix squashes the
    // world flat onto a line, and there is no way to un-squash it.
    const float determinant = a * d - b * c;
    if (std::fabs(determinant) < 1e-12f) {
        ENGINE_LOG_WARN(Channels::kCore, "Mat3::Inverse called on a matrix that cannot be undone "
                                         "(is something scaled to zero?); returning identity");
        return Identity();
    }

    const float invDet = 1.0f / determinant;

    Mat3 result = Identity();
    result.m[0][0] = d * invDet;
    result.m[0][1] = -b * invDet;
    result.m[1][0] = -c * invDet;
    result.m[1][1] = a * invDet;

    // Undoing the move as well: the offset has to be pushed back through the
    // inverted rotate/scale part and negated.
    const float tx = m[2][0];
    const float ty = m[2][1];
    result.m[2][0] = -(tx * result.m[0][0] + ty * result.m[1][0]);
    result.m[2][1] = -(tx * result.m[0][1] + ty * result.m[1][1]);

    return result;
}

// Pulls the move part back out of a finished matrix.
Vec2 Mat3::GetTranslation() const {
    return Vec2{m[2][0], m[2][1]};
}

// Pulls the resize out of a finished matrix.
Vec2 Mat3::GetScale() const {
    // How long each of the first two rows is. A matrix that also mirrors
    // (a negative scale) reports a positive number here; that limitation does
    // not matter for what this is used for - sprite sizes and Inspector boxes.
    const Vec2 rowX{m[0][0], m[0][1]};
    const Vec2 rowY{m[1][0], m[1][1]};
    return Vec2{rowX.Length(), rowY.Length()};
}

// Pulls the turn out of a finished matrix, in radians.
float Mat3::GetRotation() const {
    // Row 0 is (scaleX * cos, scaleX * sin), and atan2 only cares about the
    // ratio of its two arguments, so a positive scale cancels out.
    return std::atan2(m[0][1], m[0][0]);
}

// Combines two transforms. "Do a, then b" is written a * b, which is what the
// row-vector convention buys.
Mat3 operator*(const Mat3& a, const Mat3& b) {
    // Ordinary matrix multiplication: each output element is one row of `a`
    // multiplied through one column of `b`.
    Mat3 result;
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            result.m[row][col] =
                a.m[row][0] * b.m[0][col] + a.m[row][1] * b.m[1][col] + a.m[row][2] * b.m[2][col];
        }
    }
    return result;
}

// Compares two matrices allowing for the small errors decimal arithmetic makes,
// because two matrices that should be equal rarely are exactly.
bool ApproxEqual(const Mat3& a, const Mat3& b, float epsilon) {
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            if (!ApproxEqual(a.m[row][col], b.m[row][col], epsilon)) {
                return false;
            }
        }
    }
    return true;
}

} // namespace eng
