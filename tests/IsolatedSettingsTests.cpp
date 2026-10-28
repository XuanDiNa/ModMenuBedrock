#include "ModMenu/Core.h"
#include "ModMenu/Configure.h"
#include "json.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>

namespace {
namespace fs = std::filesystem;
using Json = nlohmann::json;
using namespace menu;
int checks{}, failures{};
void Check(bool value, char const* name) {
    ++checks;
    if (!value) { ++failures; std::cerr << "FAIL " << name << '\n'; }
}
void Write(fs::path const& path, std::string const& bytes) {
    std::ofstream file(path, std::ios::binary);
    file << bytes;
    if (!file) throw std::runtime_error("Cannot write isolated fixture");
}
std::string Read(fs::path const& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) throw std::runtime_error("Cannot read isolated fixture");
    return {std::istreambuf_iterator<char>(file), {}};
}
using Files = std::map<std::string, std::string>;
Files Snapshot(fs::path const& directory) {
    Files result;
    for (auto const& file : fs::recursive_directory_iterator(directory))
        if (file.is_regular_file()) result.emplace(file.path().lexically_relative(directory).generic_string(), Read(file.path()));
    return result;
}
Json Description() {
    return Json::parse(R"({
      "schemaVersion":1,
      "plugin":{"id":"ExamplePlugin","entry":"ExamplePlugin.dll","versions":["1.0.0"]},
      "config":{"file":"settings.json","reload":"restart"},
      "fields":[
        {"path":"/enabled","type":"boolean","default":true,"label":{"en_US":"Enabled","zh_TW":"啟用","zh_CN":"启用"}},
        {"path":"/limit","type":"integer","default":16,"min":1,"max":64,"label":"Limit"},
        {"path":"/mode","type":"choice","default":0,"label":"Mode","options":[{"en_US":"First","zh_TW":"第一","zh_CN":"第一"},{"en_US":"Last","zh_TW":"最後","zh_CN":"最后"}]},
        {"path":"/actionKey","type":"key","default":"F6","label":"Key","keys":["NONE","F6","F7","F10"]}
      ]})");
}
// These are ordinary temporary files and the real settings/controller code.
// No fixture DLL is built/loaded; a catalog row is supplied as controller input.
struct Fixture {
    fs::path root, directory, config, sidecar;
    Json description = Description();
    std::string original = R"({"enabled":true,"limit":16,"mode":0,"actionKey":"F6","unexposed":{"text":"物品","values":[1,false]}})";
    Frame frame{854, 469, true, true, false, MenuScene::Pause};
    Menu menu;
    Fixture() {
        auto temp = fs::temp_directory_path();
        root = temp / (L"ModMenuIsolated-" + std::to_wstring(GetCurrentProcessId()) + L"-" + std::to_wstring(GetTickCount64()));
        if (!fs::create_directory(root)) throw std::runtime_error("Isolated root already exists");
        directory = root / "ExamplePlugin";
        fs::create_directory(directory);
        config = directory / "settings.json"; sidecar = directory / "modmenu.settings.json";
        Write(directory / "manifest.json", R"({"entry":"ExamplePlugin.dll","version":"1.0.0"})");
        Write(config, original); Write(sidecar, description.dump());
        fs::create_directory(root / "UnrelatedFixture");
        Write(root / "UnrelatedFixture/config.json", "{\"sentinel\":true}\n");
        menu.nativeEntry = true;
        menu.entries = {{"ExamplePlugin", "Example plugin", "1.0.0"}};
        Refresh(); menu.OpenNative(frame);
    }
    ~Fixture() {
        std::error_code error;
        if (root.parent_path() == fs::temp_directory_path() && root.filename().wstring().starts_with(L"ModMenuIsolated-"))
            fs::remove_all(root, error);
    }
    void Refresh() {
        auto available = InspectSettings(root, "ExamplePlugin");
        menu.entries[0].settingsAvailable = available.available;
        menu.entries[0].settingsReason = available.reason;
    }
    void Click(Rect rectangle) {
        float x = rectangle.x + rectangle.w / 2, y = rectangle.y + rectangle.h / 2;
        menu.Input({Kind::Down, x, y, 1}, frame);
        menu.Input({Kind::Up, x, y, 1}, frame);
    }
    void Key(unsigned key) {
        menu.Input({Kind::Key, 0, 0, int(key), 1}, frame);
        menu.Input({Kind::Key, 0, 0, int(key), 0}, frame);
    }
    Rect Value(int index) {
        for (auto const& row : ArrangeSettings(menu, frame).rows) if (row.index == index) return row.value;
        throw std::runtime_error("Expected visible fixture field");
    }
    void Service() {
        // Same request boundary as the worker; no game or fabricated callback.
        auto request = menu.settingsRequest;
        if (request == Menu::Request::None) throw std::runtime_error("No settings request");
        auto draft = menu.draft; auto generation = menu.settingsGeneration;
        menu.settingsRequest = Menu::Request::None;
        auto response = request == Menu::Request::Load ? LoadSettings(root, draft.id) : ApplySettings(root, draft);
        menu.CompleteSettings(generation, std::move(response));
    }
    void OpenConfigure() { Click(Arrange(menu.config, frame).configure); Service(); }
};

void ControlsAndStorage() {
    Fixture f;
    auto before = Snapshot(f.root);
    f.Click(Arrange(f.menu.config, f.frame).configure);
    Check(f.menu.configuring && f.menu.settingsBusy && f.menu.settingsRequest == Menu::Request::Load,
          "external catalog Configure queues a real load before editing");
    Check(Snapshot(f.root) == before, "Configure click does not write any fixture file");
    f.Service();
    Check(f.menu.draft.valid && f.menu.draft.external && f.menu.draft.fields.size() == 4 && !f.menu.settingsBusy,
          "real description resolves into the four shared controls");
    if (!f.menu.draft.valid || f.menu.draft.fields.size() != 4) return;
    auto const& mode = f.menu.draft.fields[2];
    Check(DisplayOption(mode, 1, Locale::English) == "Last", "external choice exposes English caption");
    Check(DisplayOption(mode, 1, Locale::TraditionalChinese) == "最後", "external choice exposes Traditional Chinese caption");
    Check(DisplayOption(mode, 1, Locale::SimplifiedChinese) == "最后", "external choice exposes Simplified Chinese caption");

    f.Click(f.Value(3)); f.Key(119); // F8 is not in the description allowlist.
    Check(f.menu.captureField == 3 && SettingText(f.menu.draft.fields[3]) == "F6",
          "unsupported physical key cannot finish external capture or change its token");
    f.Key(118);
    Check(f.menu.captureField == -1 && SettingText(f.menu.draft.fields[3]) == "F7",
          "external key capture stores an allowed canonical token in the draft");
    f.Click(f.Value(2)); f.Key(40); f.Key(13);
    Check(f.menu.choiceField == -1 && f.menu.draft.fields[2].value == SettingValue{1},
          "external popup keyboard selection stores an index rather than a localized caption");
    f.Click(f.Value(1)); f.Key('3'); f.Key('2'); f.Key(13); f.Click(f.Value(0));
    Check(f.menu.draft.fields[1].value == SettingValue{32} && f.menu.draft.fields[0].value == SettingValue{false},
          "real shared numeric and Boolean controls edit the external draft");
    Check(Snapshot(f.root) == before, "all four control edits remain draft-only");
    f.Click(Arrange(f.menu.config, f.frame).cancel);
    Check(!f.menu.configuring && f.menu.open && f.menu.settingsRequest == Menu::Request::None,
          "Cancel returns to Mods without queueing Apply");
    Check(Snapshot(f.root) == before, "Cancel preserves every file byte and creates no backup");

    f.OpenConfigure();
    Check(f.menu.draft.fields[2].value == SettingValue{0} && SettingText(f.menu.draft.fields[3]) == "F6",
          "reopening after Cancel restores persisted choice and key");
    f.Click(f.Value(3)); f.Key(118); f.Click(f.Value(2)); f.Key(40); f.Key(13);
    f.Click(f.Value(0)); f.Click(f.Value(1)); f.Key('3'); f.Key('2');
    f.Click(Arrange(f.menu.config, f.frame).apply);
    Check(f.menu.settingsBusy && f.menu.settingsRequest == Menu::Request::Apply && Snapshot(f.root) == before,
          "Apply queues the validated numeric buffer before any storage write");
    f.Service();
    Check(f.menu.draft.saved && !f.menu.settingsChanged && !f.menu.settingsBusy,
          "real storage result completes the submitted request");
    auto saved = Json::parse(Read(f.config));
    Check(saved["enabled"] == false && saved["limit"] == 32 && saved["mode"] == 1 && saved["actionKey"] == "F7",
          "Apply persists all four literal typed values");
    Check(saved["unexposed"] == Json::parse(f.original)["unexposed"], "Apply preserves unexposed nested values");
    auto after = Snapshot(f.root); int backups{}, temporary{};
    for (auto const& [path, bytes] : after) {
        if (fs::path(path).extension() == ".bak") { ++backups; Check(bytes == f.original, "Apply backup retains exact original bytes"); }
        if (fs::path(path).extension() == ".tmp") ++temporary;
        if (path != "ExamplePlugin/settings.json" && fs::path(path).extension() != ".bak")
            Check(before.contains(path) && before.at(path) == bytes, "Apply cannot alter adjacent fixture metadata or unrelated files");
    }
    Check(backups == 1 && temporary == 0 && after.size() == before.size() + 1, "successful Apply leaves one backup and no temporary file");
    f.Click(Arrange(f.menu.config, f.frame).cancel); f.OpenConfigure();
    Check(f.menu.draft.fields[2].value == SettingValue{1} && SettingText(f.menu.draft.fields[3]) == "F7",
          "reopen reads the persisted external choice and key through the resolver");
}

void CaptureAndCancellation() {
    Fixture f; f.OpenConfigure(); auto before = Snapshot(f.root);
    f.Click(f.Value(3)); f.Key(27);
    Check(f.menu.configuring && f.menu.captureField == -1 && f.menu.settingsChanged && SettingText(f.menu.draft.fields[3])=="NONE",
          "first Escape clears a declared external key without closing Configure");
    f.Key(27);
    Check(f.menu.open && !f.menu.configuring && Snapshot(f.root) == before,
          "fresh second Escape discards Configure without filesystem changes");
    f.OpenConfigure(); std::array<bool, 256> keys{}; f.menu.PollKeyboard(keys, f.frame);
    f.Click(f.Value(3)); keys[121] = true; f.menu.PollKeyboard(keys, f.frame); f.menu.PollKeyboard(keys, f.frame);
    Check(f.menu.open && f.menu.configuring && f.menu.captureField == -1 && SettingText(f.menu.draft.fields[3]) == "F10",
          "binding the menu shortcut via physical sampling cannot close the same Configure page");
    keys[121] = false; f.menu.PollKeyboard(keys, f.frame);
    f.Click(f.Value(2)); auto apply = Arrange(f.menu.config, f.frame).apply; f.Click(apply);
    Check(f.menu.choiceField == -1 && f.menu.settingsRequest == Menu::Request::None,
          "click outside an external popup does not click through to Apply");
    auto lost = f.frame; lost.foreground = false; f.menu.Tick(lost);
    Check(!f.menu.open && !f.menu.configuring && f.menu.draft.fields.empty() && Snapshot(f.root) == before,
          "focus loss discards external key edits and control state without writing");
}

void RefusalAndRecovery() {
    Fixture f; auto before = Snapshot(f.root);
    f.description["plugin"]["versions"] = {"not-installed"}; Write(f.sidecar, f.description.dump()); f.Refresh();
    Check(f.menu.entries.size() == 1 && f.menu.Filtered().size() == 1 && !Configurable(f.menu.entries[0]) && !f.menu.entries[0].settingsReason.empty(),
          "version mismatch disables Configure without removing the supplied catalog row");
    f.Click(Arrange(f.menu.config, f.frame).configure);
    Check(!f.menu.configuring && f.menu.settingsRequest == Menu::Request::None && Read(f.config) == f.original,
          "refused Configure cannot queue a load or modify target config");
    f.description = Description(); Write(f.sidecar, f.description.dump()); f.Refresh(); f.OpenConfigure();
    Check(f.menu.draft.valid && f.menu.draft.external, "restoring the exact version recovers external Configure");
    f.Click(f.Value(1)); f.Key('2'); f.Key('3'); f.Key(13);
    Write(f.sidecar, f.description.dump() + " "); auto staleBefore = Snapshot(f.root);
    f.Click(Arrange(f.menu.config, f.frame).apply); f.Service();
    Check(!f.menu.draft.saved && f.menu.configuring && f.menu.settingsChanged && !f.menu.settingsBusy,
          "source-change refusal keeps the draft open but does not acknowledge a save");
    Check(Snapshot(f.root) == staleBefore, "description-only whitespace change refuses stale Apply without backup or temporary residue");
    f.Click(Arrange(f.menu.config, f.frame).cancel); f.OpenConfigure();
    Check(f.menu.draft.valid && f.menu.draft.fields[1].value == SettingValue{16}, "Cancel and reopen recover from stale-source refusal");
    auto generation = f.menu.settingsGeneration; auto delayed = f.menu.draft; delayed.fields[1].value = 63;
    f.Click(Arrange(f.menu.config, f.frame).cancel); f.menu.CompleteSettings(generation, delayed);
    Check(!f.menu.configuring && f.menu.draft.fields.empty() && Read(f.config) == f.original,
          "late load completion cannot resurrect a canceled external draft");
    fs::remove(f.sidecar); f.Refresh();
    Check(!Configurable(f.menu.entries[0]) && f.menu.entries.size() == 1 && !LoadSettings(f.root, "ExamplePlugin").valid,
          "unknown fixture without a sidecar has no invented fallback or catalog removal");
    Check(Read(f.root / "UnrelatedFixture/config.json") == before.at("UnrelatedFixture/config.json"),
          "all refusal and recovery operations preserve the neighboring fixture");

    // A known adapter ID in this temporary root only, never the installed mod.
    auto legacy = f.root / "AutoRestock"; fs::create_directory(legacy);
    Write(legacy / "manifest.json", R"({"entry":"AutoRestock.dll","version":"fixture-only"})");
    Write(legacy / "config.json", R"({"enabled":true,"preRestockThreshold":17})");
    auto original = Snapshot(legacy); auto d = LoadSettings(f.root, "AutoRestock");
    Check(d.valid && !d.external && d.fields.size() == 2, "absent sidecar selects the existing built-in adapter in a temporary root");
    Write(legacy / "modmenu.settings.json", "{}"); auto invalid = Snapshot(legacy);
    Check(!InspectSettings(f.root, "AutoRestock").available && !LoadSettings(f.root, "AutoRestock").valid,
          "invalid present description cannot fall back to a known built-in adapter");
    Check(!ApplySettings(f.root, d).saved && Snapshot(legacy) == invalid,
          "new sidecar also invalidates an already opened built-in draft without writing");
    fs::remove(legacy / "modmenu.settings.json"); d = LoadSettings(f.root, "AutoRestock");
    Check(d.valid && !d.external && Snapshot(legacy) == original,
          "removing only the invalid fixture sidecar restores the original adapter and exact files");
}
}
int main() {
    try { ControlsAndStorage(); CaptureAndCancellation(); RefusalAndRecovery(); }
    catch (std::exception const& error) { ++failures; std::cerr << "FAIL isolated setup/execution: " << error.what() << '\n'; }
    std::cout << checks << " isolated checks / " << failures << " failures\n";
    return failures ? 1 : 0;
}
