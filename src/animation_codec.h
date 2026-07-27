#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <anim/animation.hpp>

// Binary serialization for an anim::Animation.
//
// This is what the operator hands to TouchDesigner's saveData()/loadData(), so
// the encoded bytes end up inside the .toe file. That means the format is
// persisted user data: anything written by a released build has to keep
// decoding in every later build, which is why the blob is versioned and why
// decode() validates rather than trusting its input.
//
// Deliberately not JSON or the Python state dict. The blob is written on every
// project save and read on every load, a keyframe-heavy animation runs to tens
// of thousands of values, and the codec has to work with no Python interpreter
// available -- loadData() runs during node construction.
//
// Kept free of both TouchDesigner and Python so it can be tested on its own;
// see tests/cpp.
namespace animation_codec {

// Bumped only when the layout changes incompatibly. decode() refuses anything
// it does not recognize rather than guessing at the bytes.
inline constexpr uint32_t kFormatVersion = 1;

// The key the operator stores the blob under, via OP_NodeSaveState::saveEntry.
inline constexpr const char* kSaveKey = "animation";

// Serializes the animation: range, then every channel with its extend modes and
// keyframes, in order.
std::vector<uint8_t> encode(const anim::Animation& animation);

// Rebuilds `animation` from bytes produced by encode().
//
// Returns false and leaves `animation` untouched if the data is not a blob this
// build understands -- wrong magic, unknown version, truncated, or internally
// inconsistent. A .toe can carry a blob from a newer build, or a corrupted one;
// neither should take the node down or half-load an animation. On failure,
// `error` (when given) describes what was rejected.
bool decode(const void* data,
            size_t byteSize,
            anim::Animation& animation,
            std::string* error = nullptr);

} // namespace animation_codec
