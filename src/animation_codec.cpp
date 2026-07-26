#include "animation_codec.h"

#include <cstring>
#include <limits>

#include <anim/channel.hpp>
#include <anim/extend.hpp>
#include <anim/function.hpp>
#include <anim/handle_mode.hpp>
#include <anim/keyframe.hpp>
#include <anim/point.hpp>

namespace animation_codec {
namespace {

// "ACHP" -- checked before anything else so a blob written by some other
// operator, or a stale entry under the same key, is rejected rather than
// interpreted.
constexpr uint8_t kMagic[4] = { 'A', 'C', 'H', 'P' };

// A channel count or name length larger than this means the data is corrupt.
// Without a ceiling a bogus length would have us reserve gigabytes before the
// bounds check on the following read ever ran.
constexpr uint32_t kSaneCountLimit = 100u * 1000u * 1000u;

// Both platforms TouchDesigner runs on are little-endian IEEE-754, so doubles
// go out raw. If that ever stops being true this is the assumption to revisit;
// a .toe is expected to move between Windows and macOS.
static_assert(sizeof(double) == 8, "Encoding assumes 8-byte doubles");

// --- writing ---------------------------------------------------------------

void put(std::vector<uint8_t>& out, const void* bytes, size_t count)
{
    const auto* p = static_cast<const uint8_t*>(bytes);
    out.insert(out.end(), p, p + count);
}

void putU8(std::vector<uint8_t>& out, uint8_t value)
{
    out.push_back(value);
}

void putU32(std::vector<uint8_t>& out, uint32_t value)
{
    put(out, &value, sizeof(value));
}

void putDouble(std::vector<uint8_t>& out, double value)
{
    put(out, &value, sizeof(value));
}

void putPoint(std::vector<uint8_t>& out, const anim::Point& point)
{
    putDouble(out, point.time);
    putDouble(out, point.value);
}

void putString(std::vector<uint8_t>& out, const std::string& value)
{
    putU32(out, static_cast<uint32_t>(value.size()));
    put(out, value.data(), value.size());
}

// --- reading ---------------------------------------------------------------

// A bounds-checked cursor. Every read goes through take(), so a truncated blob
// fails the read instead of running off the end of the buffer.
class Reader
{
public:
    Reader(const uint8_t* data, size_t size) : m_data(data), m_size(size) {}

    bool take(void* dest, size_t count)
    {
        if (count > m_size - m_offset)  // m_offset <= m_size always, so no overflow
            return false;
        std::memcpy(dest, m_data + m_offset, count);
        m_offset += count;
        return true;
    }

    bool u8(uint8_t& value) { return take(&value, sizeof(value)); }
    bool u32(uint32_t& value) { return take(&value, sizeof(value)); }
    bool real(double& value) { return take(&value, sizeof(value)); }

    bool point(anim::Point& value)
    {
        return real(value.time) && real(value.value);
    }

    bool string(std::string& value, uint32_t limit)
    {
        uint32_t length = 0;
        if (!u32(length) || length > limit || length > remaining())
            return false;
        value.resize(length);
        return length == 0 || take(value.data(), length);
    }

    size_t remaining() const { return m_size - m_offset; }

private:
    const uint8_t* m_data;
    size_t         m_size;
    size_t         m_offset = 0;
};

bool fail(std::string* error, const char* message)
{
    if (error)
        *error = message;
    return false;
}

// The enums are stored as their underlying byte, so a blob from a build with
// more modes than this one must not produce an out-of-range enum.
template <typename Enum>
bool readEnum(Reader& reader, Enum& out, uint8_t limit)
{
    uint8_t raw = 0;
    if (!reader.u8(raw) || raw >= limit)
        return false;
    out = static_cast<Enum>(raw);
    return true;
}

constexpr uint8_t kFunctionCount   = 3;  // Constant, Linear, Bezier
constexpr uint8_t kHandleModeCount = static_cast<uint8_t>(anim::HandleMode::Count);
constexpr uint8_t kExtendCount     = 3;  // Hold, Repeat, Mirror

} // namespace

std::vector<uint8_t> encode(const anim::Animation& animation)
{
    std::vector<uint8_t> out;

    put(out, kMagic, sizeof(kMagic));
    putU32(out, kFormatVersion);
    putDouble(out, animation.start_time());
    putDouble(out, animation.end_time());

    const size_t channelCount = animation.num_channels();
    putU32(out, static_cast<uint32_t>(channelCount));

    for (size_t c = 0; c < channelCount; ++c) {
        const anim::Channel& channel = animation.channel(c);

        putString(out, channel.name());
        putU8(out, static_cast<uint8_t>(channel.extend_start()));
        putU8(out, static_cast<uint8_t>(channel.extend_end()));

        const auto& keyframes = channel.keyframes();
        putU32(out, static_cast<uint32_t>(keyframes.size()));

        for (const anim::Keyframe& keyframe : keyframes) {
            putPoint(out, keyframe.position);
            putPoint(out, keyframe.in_handle);
            putPoint(out, keyframe.out_handle);
            putU8(out, static_cast<uint8_t>(keyframe.function));
            putU8(out, static_cast<uint8_t>(keyframe.handle_mode));
        }
    }

    return out;
}

bool decode(const void* data,
            size_t byteSize,
            anim::Animation& animation,
            std::string* error)
{
    if (!data || byteSize == 0)
        return fail(error, "No data");

    Reader reader(static_cast<const uint8_t*>(data), byteSize);

    uint8_t magic[4] = {};
    if (!reader.take(magic, sizeof(magic)) || std::memcmp(magic, kMagic, sizeof(magic)) != 0)
        return fail(error, "Not an AnimationCHOP blob");

    uint32_t version = 0;
    if (!reader.u32(version))
        return fail(error, "Truncated header");
    if (version != kFormatVersion)
        return fail(error, "Unsupported format version");

    double startTime = 0.0;
    double endTime = 0.0;
    if (!reader.real(startTime) || !reader.real(endTime))
        return fail(error, "Truncated animation range");

    uint32_t channelCount = 0;
    if (!reader.u32(channelCount) || channelCount > kSaneCountLimit)
        return fail(error, "Bad channel count");

    // Decode into a scratch animation and only commit on success, so a blob
    // that fails halfway does not leave the node holding half an animation.
    anim::Animation decoded;

    for (uint32_t c = 0; c < channelCount; ++c) {
        std::string name;
        if (!reader.string(name, kSaneCountLimit))
            return fail(error, "Bad channel name");

        anim::Extend extendStart = anim::Extend::Hold;
        anim::Extend extendEnd = anim::Extend::Hold;
        if (!readEnum(reader, extendStart, kExtendCount) ||
            !readEnum(reader, extendEnd, kExtendCount))
            return fail(error, "Bad extend mode");

        uint32_t keyframeCount = 0;
        if (!reader.u32(keyframeCount) || keyframeCount > kSaneCountLimit)
            return fail(error, "Bad keyframe count");

        // create_channel appends, so channels come back in their saved order.
        // A duplicate name in the blob would collide; anim allows it at the
        // index level, and lookups by name resolve to the first, which matches
        // how the animation behaved when it was saved.
        anim::Channel& channel = decoded.create_channel(name);

        std::vector<anim::Keyframe> keyframes;
        keyframes.reserve(keyframeCount < 4096 ? keyframeCount : 4096);

        for (uint32_t k = 0; k < keyframeCount; ++k) {
            anim::Keyframe keyframe;
            if (!reader.point(keyframe.position) ||
                !reader.point(keyframe.in_handle) ||
                !reader.point(keyframe.out_handle))
                return fail(error, "Truncated keyframe");

            if (!readEnum(reader, keyframe.function, kFunctionCount) ||
                !readEnum(reader, keyframe.handle_mode, kHandleModeCount))
                return fail(error, "Bad keyframe mode");

            keyframes.push_back(keyframe);
            channel.emplace_keyframe(std::move(keyframe));
        }

        // Inserting re-solves the neighbouring handles, and it does so
        // incrementally -- a keyframe's handles can be adjusted again by the
        // ones inserted after it. That is correct for the derived modes
        // (Flat, Smooth), which are meant to follow their neighbours, but it
        // loses the explicit handles of a Free or Aligned keyframe. Replay
        // them now that every neighbour exists.
        for (size_t k = 0; k < keyframes.size() && k < channel.num_keyframes(); ++k) {
            const anim::Keyframe& saved = keyframes[k];
            if (saved.handle_mode == anim::HandleMode::Flat ||
                saved.handle_mode == anim::HandleMode::Smooth)
                continue;
            channel.set_keyframe_in_handle(k, saved.in_handle);
            channel.set_keyframe_out_handle(k, saved.out_handle);
        }

        channel.set_extend_start(extendStart);
        channel.set_extend_end(extendEnd);
    }

    decoded.set_end_time(std::max(startTime, endTime));
    decoded.set_start_time(startTime);
    decoded.set_end_time(endTime);

    animation = std::move(decoded);
    return true;
}

} // namespace animation_codec
