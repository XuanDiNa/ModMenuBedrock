// Exercise the real Core/WorkspaceUi/broker boundary used by NativeHost:
// Move carries coordinates; mapped Down/Up read Menu's stored pointer.
// No game, provider worker, account, clipboard or network operation is started.
#include "ModMenu/Core.h"
#include <iostream>

namespace {
using namespace menu;
int failures{}, checks{};
void Check(bool ok, char const* label) {
    ++checks;
    std::cout << (ok ? "PASS " : "FAIL ") << label << '\n';
    if (!ok) ++failures;
}
Frame const frame{854, 469, true, true, false, MenuScene::Main};
std::uint64_t Session(char const* id) {
    WorkspaceDefinition def;
    def.id = id;
    def.entry = std::string(id) + ".dll";
    def.version = "1";
    WorkspaceControl action;
    action.id = "probe";
    action.type = "action";
    action.label.en = "Probe";
    WorkspacePage page;
    page.id = "first";
    page.label.en = "First";
    WorkspaceControl status;
    status.id = "session_state";
    status.type = "status";
    status.label.en = "Session";
    page.controls.push_back(status);
    status.id = "settings";
    status.label.en = "Settings";
    page.controls.push_back(status);
    page.controls.push_back(action);
    def.pages.push_back(page);
    page.id = "second";
    page.label.en = "Second";
    page.controls.clear();
    def.pages.push_back(page);
    auto session = Workspaces().Register(std::move(def));
    Workspaces().Publish(session, R"({"protocolVersion":1,"revision":1,"controls":{"session_state":{"text":"idle"},"settings":{"text":"enabled=true"},"probe":{"enabled":true}}})");
    return session;
}
void Open(Menu& m, std::uint64_t session) {
    m.Tick(frame); // Establish the same verified origin as real native entry.
    m.open = m.workspaceOpen = true;
    m.nativeEntry = true;
    // Previous pointer: the Workspace entry in the Mods catalogue. It falls
    // on a read-only row, so stale mapped clicks cannot activate the action.
    m.pointerX = 600;
    m.pointerY = 80;
    m.workspaces = Workspaces().Snapshots();
    for (auto const& s : m.workspaces)
        if (s.session == session) m.workspace.Open(s);
}
void MappedClick(Menu& m) {
    m.Input({Kind::Down, m.pointerX, m.pointerY, 1}, frame);
    m.Input({Kind::Up, m.pointerX, m.pointerY, 1}, frame);
}
bool PollAction(std::uint64_t session) {
    char bytes[32768]{};
    std::uint32_t required{};
    return Workspaces().Poll(session, bytes, sizeof(bytes), required) &&
        required && WorkspaceJson::parse(bytes).at("action") == "probe";
}
}

int main() {
    {
        Menu m; auto s = Session("MappedAction"); Open(m, s);
        m.Input({Kind::Move, 20, 121}, frame);
        Check(m.pointerX == 20 && m.pointerY == 121,
              "workspace movement updates mapped-button pointer");
        MappedClick(m);
        Check(PollAction(s), "move then native-style mapped click submits third-row action");
        Workspaces().Unregister(s);
    }
    {
        Menu m; auto s = Session("MappedNavigation"); Open(m, s);
        m.Input({Kind::Move, 835, 40}, frame);
        MappedClick(m);
        Check(m.workspace.page == 1, "move then mapped click changes workspace page");
        Workspaces().Unregister(s);
    }
    {
        Menu m; auto s = Session("MappedBack"); Open(m, s);
        m.Input({Kind::Move, 20, 450}, frame);
        MappedClick(m);
        Check(!m.workspaceOpen && m.open, "move then mapped Back returns to Mods");
        Workspaces().Unregister(s);
    }
    {
        Menu m; auto s = Session("KeyboardControl"); Open(m, s);
        m.workspace.focus = 2; // Skip the two read-only rows.
        m.Input({Kind::Key, 0, 0, 13, 1}, frame);
        Check(PollAction(s), "keyboard action reaches real broker without pointer");
        Workspaces().Unregister(s);
    }
    {
        Menu m; auto s = Session("DirectCoordinateControl"); Open(m, s);
        m.Input({Kind::Down, 20, 121, 1}, frame);
        m.Input({Kind::Up, 20, 121, 1}, frame);
        Check(PollAction(s), "direct-coordinate click remains compatible");
        Workspaces().Unregister(s);
    }
    struct GuardCase { Frame frame; bool remainsOpen; char const* label; };
    for (auto const& test : {
        GuardCase{{854, 469, false, true, false, MenuScene::Unknown}, false,
                  "ineligible scene cannot update pointer or keep workspace authority"},
        GuardCase{{854, 469, true, false, false, MenuScene::Main}, true,
                  "lost focus retains known page but cannot update pointer or submit action"},
        GuardCase{{239, 469, true, true, false, MenuScene::Main}, false,
                  "too-narrow viewport cannot update pointer or keep workspace authority"},
        GuardCase{{854, 159, true, true, false, MenuScene::Main}, false,
                  "too-short viewport cannot update pointer or keep workspace authority"},
        GuardCase{{854, 469, true, true, true, MenuScene::Main}, true,
                  "observer-only input cannot update pointer or submit an action"}
    }) {
        Menu m; auto s = Session("InputGuard"); Open(m, s);
        bool consumed = m.Input({Kind::Move, 20, 121}, test.frame);
        m.Input({Kind::Down, m.pointerX, m.pointerY, 1}, test.frame);
        m.Input({Kind::Up, m.pointerX, m.pointerY, 1}, test.frame);
        std::uint32_t required{};
        Workspaces().Poll(s, nullptr, 0, required);
        Check(!consumed && m.workspaceOpen == test.remainsOpen &&
              m.pointerX == 600 && m.pointerY == 80 && required == 0, test.label);
        Workspaces().Unregister(s);
    }
    std::cout << "workspace pointer checks=" << checks << "; failures=" << failures << '\n';
    return failures ? 1 : 0;
}
