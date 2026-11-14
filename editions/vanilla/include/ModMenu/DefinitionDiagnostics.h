#pragma once
#include <array>
#include <cstdint>
#include <string>
namespace menu {
enum class LayerProbeState { Unreadable, Empty, Compatible, Unsupported, InvalidString, Budget, Error };
struct DefinitionLayerProbe {
 std::uint64_t bytes{};
 unsigned tag{};
 LayerProbeState state{LayerProbeState::Unreadable};
 bool sameAsBase{};
};
struct DefinitionProbe {
 bool pause{},validVector{};
 unsigned layers{},inspected{};
 std::array<DefinitionLayerProbe,8> items{};
};
std::string DescribeDefinitionProbe(DefinitionProbe const&);
}
