#pragma once
#include "Core.h"
#include "NativeProfile.h"
#include "DefinitionDiagnostics.h"
#include <mutex>
#include <atomic>
namespace menu {
struct RenderReceipt {
 std::array<char,64> scene{};
 std::uint32_t rendererTable{},beginMethod{},endMethod{};
 float nativeWidth{},nativeHeight{},contextAlpha{},shaderAlpha{};
 unsigned stencil{},depth{},clip{};
 bool foreground{},top{};
};
struct SharedState {
 std::mutex mutex;Menu menu;
 std::array<DefinitionProbe,8> definitionProbes{};
 std::array<std::atomic<bool>,8> definitionProbeReady{};
 std::atomic<unsigned> definitionProbeClaims{};
 std::array<RenderReceipt,12> renderReceipts{};unsigned renderReceiptCount{};
 std::atomic<int> status{0};
 std::atomic<unsigned long long> draws{},inputs{},eligible{},faults{},unknownScenes{};
 std::atomic<unsigned long long> contexts{};
 std::atomic<unsigned long long> definitionScopes{},definitionsPatched{},definitionsRefused{},nativeOpens{};
 std::atomic<unsigned long long> definitionCandidates{},scopedControllerEvents{},modsEvents{};
 std::atomic<unsigned> definitionLayers{},lastControllerTable{};
 std::atomic<unsigned long long> foregroundRenders{},backgroundRenders{},siblingCallbacks{},rawEvents{},keyEvents{},paintCommands{};
 std::atomic<unsigned long long> pointerEvents{},primaryPointerEvents{},downEvents{},upEvents{},textEvents{},wheelEvents{},physicalKeyFrames{};
 std::atomic<unsigned long long> sampledShortcutPresses{};
 std::atomic<unsigned> lastMappedId{};
 std::atomic<int> lastRawKey{},lastRawType{},lastRawState{};
 std::atomic<int> lastGate{};
 std::atomic<float> width{},height{};
};
enum class InstallResult {Ready,Wait,Conflict,Failure};
InstallResult InstallNative(NativeProfile const&,SharedState&,bool expectMouseTweaks,std::string& reason);
bool InstallDefinitions(NativeProfile const&,SharedState&,std::string& reason);
void DeactivateNative() noexcept;
}
