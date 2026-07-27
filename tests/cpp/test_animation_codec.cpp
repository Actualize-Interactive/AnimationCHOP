// Tests for the .toe persistence codec.
//
// The bytes this codec writes end up inside users' project files, so the two
// things that matter are that a round trip is faithful, and that decode()
// refuses anything it does not fully understand rather than producing a
// half-built animation.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <string>
#include <vector>

#include "animation_codec.h"

#include <anim/animation.hpp>
#include <anim/channel.hpp>

using Catch::Matchers::WithinAbs;

namespace {

constexpr double kEps = 1e-9;

void requirePointsEqual(const anim::Point& actual, const anim::Point& expected)
{
    REQUIRE_THAT(actual.time, WithinAbs(expected.time, kEps));
    REQUIRE_THAT(actual.value, WithinAbs(expected.value, kEps));
}

void requireKeyframesEqual(const anim::Keyframe& actual, const anim::Keyframe& expected)
{
    requirePointsEqual(actual.position, expected.position);
    requirePointsEqual(actual.in_handle, expected.in_handle);
    requirePointsEqual(actual.out_handle, expected.out_handle);
    REQUIRE(actual.function == expected.function);
    REQUIRE(actual.handle_mode == expected.handle_mode);
}

void requireAnimationsEqual(const anim::Animation& actual, const anim::Animation& expected)
{
    REQUIRE_THAT(actual.start_time(), WithinAbs(expected.start_time(), kEps));
    REQUIRE_THAT(actual.end_time(), WithinAbs(expected.end_time(), kEps));
    REQUIRE(actual.num_channels() == expected.num_channels());

    for (size_t c = 0; c < expected.num_channels(); ++c) {
        const anim::Channel& a = actual.channel(c);
        const anim::Channel& e = expected.channel(c);

        INFO("channel " << c << " (" << e.name() << ")");
        REQUIRE(a.name() == e.name());
        REQUIRE(a.extend_start() == e.extend_start());
        REQUIRE(a.extend_end() == e.extend_end());
        REQUIRE(a.num_keyframes() == e.num_keyframes());

        for (size_t k = 0; k < e.num_keyframes(); ++k) {
            INFO("keyframe " << k);
            requireKeyframesEqual(a.keyframe(k), e.keyframe(k));
        }
    }
}

// Round trip and compare, returning the decoded animation for further checks.
anim::Animation roundTrip(const anim::Animation& source)
{
    const std::vector<uint8_t> bytes = animation_codec::encode(source);
    REQUIRE_FALSE(bytes.empty());

    anim::Animation decoded;
    std::string error;
    REQUIRE(animation_codec::decode(bytes.data(), bytes.size(), decoded, &error));
    REQUIRE(error.empty());
    return decoded;
}

} // namespace

TEST_CASE("An empty animation round trips", "[codec]")
{
    anim::Animation source;
    source.set_start_time(0.0);
    source.set_end_time(30.0);

    const anim::Animation decoded = roundTrip(source);

    REQUIRE(decoded.num_channels() == 0);
    requireAnimationsEqual(decoded, source);
}

TEST_CASE("Channels, keyframes and the range round trip", "[codec]")
{
    anim::Animation source;
    source.set_start_time(1.0);
    source.set_end_time(11.0);

    anim::Channel& tx = source.create_channel("tx");
    tx.create_keyframe(0.0, 0.0);
    tx.create_keyframe(5.0, 100.0);
    tx.create_keyframe(10.0, 50.0);

    anim::Channel& ty = source.create_channel("ty");
    ty.create_keyframe(2.5, -1.5);

    const anim::Animation decoded = roundTrip(source);

    REQUIRE(decoded.num_channels() == 2);
    REQUIRE(decoded.channel(0).name() == "tx");
    REQUIRE(decoded.channel(1).name() == "ty");
    requireAnimationsEqual(decoded, source);
}

TEST_CASE("Channel order is preserved", "[codec]")
{
    anim::Animation source;
    for (const char* name : { "z", "a", "m", "b" })
        source.create_channel(name).create_keyframe(0.0, 0.0);

    const anim::Animation decoded = roundTrip(source);

    REQUIRE(decoded.channel_names() == std::vector<std::string>{ "z", "a", "m", "b" });
}

TEST_CASE("Every interpolation function round trips", "[codec]")
{
    anim::Animation source;
    anim::Channel& channel = source.create_channel("f");
    channel.create_keyframe(0.0, 0.0, anim::Function::Constant, anim::HandleMode::Flat);
    channel.create_keyframe(1.0, 1.0, anim::Function::Linear, anim::HandleMode::Flat);
    channel.create_keyframe(2.0, 2.0, anim::Function::Bezier, anim::HandleMode::Flat);

    const anim::Animation decoded = roundTrip(source);

    REQUIRE(decoded.channel(0).keyframe(0).function == anim::Function::Constant);
    REQUIRE(decoded.channel(0).keyframe(1).function == anim::Function::Linear);

    // Note the third is NOT Bezier as created. A channel's last keyframe
    // inherits its predecessor's function and handle mode, because its own
    // would govern a segment that does not exist. Assert against the source
    // rather than what was asked for -- the codec's job is fidelity to the
    // animation as it actually is.
    REQUIRE(decoded.channel(0).keyframe(2).function == channel.keyframe(2).function);
    requireAnimationsEqual(decoded, source);
}

TEST_CASE("Restoring loses the last keyframe's pre-inheritance function", "[codec][known-gap]")
{
    // anim caches what the last keyframe's function/handle mode were before
    // inheritance overwrote them, and restores them if another keyframe is
    // appended after it. That cache is private, so the codec cannot see it and
    // cannot persist it.
    //
    // Consequence: append a keyframe to a channel that has been through a save
    // and reload, and the formerly-last keyframe keeps its inherited function
    // instead of reverting to the one it was created with. Pinned here so the
    // gap is known rather than discovered.
    anim::Animation source;
    anim::Channel& channel = source.create_channel("f");
    channel.create_keyframe(0.0, 0.0, anim::Function::Constant, anim::HandleMode::Flat);
    channel.create_keyframe(1.0, 1.0, anim::Function::Bezier, anim::HandleMode::Smooth);

    // The second keyframe was created Bezier but shows Constant, inherited.
    REQUIRE(channel.keyframe(1).function == anim::Function::Constant);

    anim::Animation decoded = roundTrip(source);

    // Append to both, which is what makes the cache observable.
    channel.create_keyframe(2.0, 2.0, anim::Function::Linear, anim::HandleMode::Flat);
    decoded.channel(0).create_keyframe(2.0, 2.0, anim::Function::Linear, anim::HandleMode::Flat);

    // The original restores the Bezier it had cached; the reloaded one cannot.
    REQUIRE(channel.keyframe(1).function == anim::Function::Bezier);
    REQUIRE(decoded.channel(0).keyframe(1).function == anim::Function::Constant);
}

TEST_CASE("Every handle mode round trips", "[codec]")
{
    // Explicit handles are the interesting case: inserting a keyframe re-solves
    // its neighbours' handles, so a naive rebuild loses whatever a Free or
    // Aligned keyframe was actually holding.
    anim::Animation source;
    anim::Channel& channel = source.create_channel("h");

    const auto modes = {
        anim::HandleMode::Flat,
        anim::HandleMode::Smooth,
        anim::HandleMode::Aligned,
        anim::HandleMode::Free,
        anim::HandleMode::AlignStrict,
        anim::HandleMode::AlignFlex,
        anim::HandleMode::AlignAdjustable,
    };

    double time = 0.0;
    for (anim::HandleMode mode : modes) {
        channel.create_keyframe(time, time * 2.0, anim::Function::Bezier, mode);
        time += 1.0;
    }

    const anim::Animation decoded = roundTrip(source);

    REQUIRE(decoded.channel(0).num_keyframes() == channel.num_keyframes());
    requireAnimationsEqual(decoded, source);
}

TEST_CASE("Free handles survive the round trip", "[codec]")
{
    anim::Animation source;
    anim::Channel& channel = source.create_channel("free");
    channel.create_keyframe(0.0, 0.0, anim::Function::Bezier, anim::HandleMode::Free);
    channel.create_keyframe(4.0, 10.0, anim::Function::Bezier, anim::HandleMode::Free);
    channel.create_keyframe(8.0, 0.0, anim::Function::Bezier, anim::HandleMode::Free);

    channel.set_keyframe_in_handle(1, anim::Point(3.25, 7.5));
    channel.set_keyframe_out_handle(1, anim::Point(5.75, 12.5));

    const anim::Animation decoded = roundTrip(source);

    requireAnimationsEqual(decoded, source);
    requirePointsEqual(decoded.channel(0).keyframe(1).in_handle,
                       channel.keyframe(1).in_handle);
    requirePointsEqual(decoded.channel(0).keyframe(1).out_handle,
                       channel.keyframe(1).out_handle);
}

TEST_CASE("Extend modes round trip", "[codec]")
{
    anim::Animation source;
    anim::Channel& channel = source.create_channel("e");
    channel.create_keyframe(0.0, 0.0);
    channel.create_keyframe(1.0, 1.0);
    channel.set_extend_start(anim::Extend::Repeat);
    channel.set_extend_end(anim::Extend::Mirror);

    const anim::Animation decoded = roundTrip(source);

    REQUIRE(decoded.channel(0).extend_start() == anim::Extend::Repeat);
    REQUIRE(decoded.channel(0).extend_end() == anim::Extend::Mirror);
}

TEST_CASE("Channel names with awkward content round trip", "[codec]")
{
    anim::Animation source;
    source.create_channel("");                    // empty
    source.create_channel("a name with spaces");
    source.create_channel("utf8: \xc3\xa9\xc3\xa8");
    source.create_channel(std::string("embedded\0nul", 12));

    const anim::Animation decoded = roundTrip(source);

    REQUIRE(decoded.num_channels() == 4);
    REQUIRE(decoded.channel(0).name().empty());
    REQUIRE(decoded.channel(1).name() == "a name with spaces");
    REQUIRE(decoded.channel(3).name() == std::string("embedded\0nul", 12));
}

TEST_CASE("The decoded curve evaluates identically", "[codec]")
{
    // The point of persistence: the curve a user gets back has to be the curve
    // they saved, not merely the same keyframe values.
    anim::Animation source;
    anim::Channel& channel = source.create_channel("curve");
    channel.create_keyframe(0.0, 0.0, anim::Function::Bezier, anim::HandleMode::Smooth);
    channel.create_keyframe(3.0, 10.0, anim::Function::Bezier, anim::HandleMode::Aligned);
    channel.create_keyframe(6.0, -5.0, anim::Function::Linear, anim::HandleMode::Free);
    channel.create_keyframe(9.0, 2.0, anim::Function::Constant, anim::HandleMode::Flat);

    const anim::Animation decoded = roundTrip(source);

    for (double t = -1.0; t <= 10.0; t += 0.125) {
        INFO("t = " << t);
        REQUIRE_THAT(decoded.channel(0).evaluate(t),
                     WithinAbs(channel.evaluate(t), 1e-9));
    }
}

// --- rejecting bad input ----------------------------------------------------

TEST_CASE("decode rejects null and empty data", "[codec]")
{
    anim::Animation animation;
    std::string error;

    REQUIRE_FALSE(animation_codec::decode(nullptr, 0, animation, &error));
    REQUIRE_FALSE(error.empty());

    const uint8_t nothing = 0;
    REQUIRE_FALSE(animation_codec::decode(&nothing, 0, animation, &error));
}

TEST_CASE("decode rejects a foreign blob", "[codec]")
{
    const char foreign[] = "not an animation at all, just some bytes";
    anim::Animation animation;
    std::string error;

    REQUIRE_FALSE(animation_codec::decode(foreign, sizeof(foreign), animation, &error));
    REQUIRE(error == "Not an AnimationCHOP blob");
}

TEST_CASE("decode rejects an unknown format version", "[codec]")
{
    anim::Animation source;
    source.create_channel("tx").create_keyframe(0.0, 1.0);
    std::vector<uint8_t> bytes = animation_codec::encode(source);

    // Version sits immediately after the 4-byte magic.
    bytes[4] = 0xFF;

    anim::Animation animation;
    std::string error;
    REQUIRE_FALSE(animation_codec::decode(bytes.data(), bytes.size(), animation, &error));
    REQUIRE(error == "Unsupported format version");
}

TEST_CASE("decode rejects truncation at every length", "[codec]")
{
    // A .toe can be truncated, and a blob written by a newer build can be
    // longer than we expect. No prefix of a valid blob should decode as
    // anything but a failure.
    anim::Animation source;
    source.set_end_time(12.0);
    anim::Channel& channel = source.create_channel("tx");
    channel.create_keyframe(0.0, 0.0);
    channel.create_keyframe(6.0, 3.0);
    source.create_channel("ty").create_keyframe(1.0, 1.0);

    const std::vector<uint8_t> bytes = animation_codec::encode(source);

    for (size_t length = 1; length < bytes.size(); ++length) {
        INFO("truncated to " << length << " of " << bytes.size() << " bytes");
        anim::Animation animation;
        REQUIRE_FALSE(animation_codec::decode(bytes.data(), length, animation));
    }
}

TEST_CASE("decode rejects an out-of-range enum", "[codec]")
{
    anim::Animation source;
    source.create_channel("tx").create_keyframe(0.0, 1.0);
    std::vector<uint8_t> bytes = animation_codec::encode(source);

    // The keyframe's two mode bytes are the last of the blob.
    bytes[bytes.size() - 1] = 0x7F;

    anim::Animation animation;
    std::string error;
    REQUIRE_FALSE(animation_codec::decode(bytes.data(), bytes.size(), animation, &error));
    REQUIRE(error == "Bad keyframe mode");
}

TEST_CASE("A failed decode leaves the target animation untouched", "[codec]")
{
    anim::Animation existing;
    existing.create_channel("keep").create_keyframe(0.0, 42.0);
    existing.set_end_time(7.0);

    const char garbage[] = "ACHP but then nonsense follows here";
    REQUIRE_FALSE(animation_codec::decode(garbage, sizeof(garbage), existing));

    REQUIRE(existing.num_channels() == 1);
    REQUIRE(existing.channel(0).name() == "keep");
    REQUIRE_THAT(existing.channel(0).keyframe(0).value(), WithinAbs(42.0, kEps));
    REQUIRE_THAT(existing.end_time(), WithinAbs(7.0, kEps));
}

TEST_CASE("A large animation round trips", "[codec]")
{
    anim::Animation source;
    for (int c = 0; c < 8; ++c) {
        anim::Channel& channel = source.create_channel("chan" + std::to_string(c));
        for (int k = 0; k < 500; ++k)
            channel.create_keyframe(k * 0.25, static_cast<double>(k % 17));
    }

    const std::vector<uint8_t> bytes = animation_codec::encode(source);
    anim::Animation decoded;
    REQUIRE(animation_codec::decode(bytes.data(), bytes.size(), decoded));

    REQUIRE(decoded.num_channels() == 8);
    REQUIRE(decoded.channel(0).num_keyframes() == 500);
    requireAnimationsEqual(decoded, source);
}
