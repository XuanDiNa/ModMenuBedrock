#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <cstdint>
#include <array>
#include <optional>
#include "Settings.h"
#include "Locale.h"
#include "WorkspaceUi.h"
namespace menu {
struct Rect {float x{},y{},w{},h{};bool Contains(float,float) const;};
struct Placement {int anchor{0};float x{12},y{12},scale{1};Rect Bounds(float,float) const;};
struct Config {Placement button;unsigned openKey{121};unsigned openModifiers{};};
inline bool UsableKey(unsigned k){return modmenu::binding::Allowed(k);}
struct Entry {std::string id,name,version,author,description,status,website;bool loaded{};std::optional<bool> settingsAvailable;std::string settingsReason;};
inline bool Configurable(Entry const& entry){return entry.settingsAvailable?*entry.settingsAvailable:HasSettings(entry.id);}
enum class Kind {Move,Down,Up,Wheel,Key,Text};
struct Modifiers {bool shift{},control{},alt{},caps{},leftAlt{};};
inline unsigned BindingModifiers(Modifiers m,unsigned key){return ((m.control?modmenu::binding::Ctrl:0u)|(m.shift?modmenu::binding::Shift:0u)|(m.leftAlt?modmenu::binding::LeftAlt:0u))&~modmenu::binding::PrimaryModifier(key);}
struct Event {Kind kind;float x{},y{};int code{},value{};std::string text;Modifiers modifiers;};
// observerOnly is set only for a verified sibling scene while the same
// client's verified main or pause menu is topmost. It has no input/draw authority.
enum class MenuScene {Unknown,Main,Pause};
MenuScene ClassifyMenuScene(std::string_view);
struct Frame {float width{},height{};bool eligible{},foreground{},observerOnly{};MenuScene scene{};};
struct Layout {Rect button,search,count,list,details,description,done,settings,back,edit,reset,key,scale,configure,apply,cancel,fields;float row{28},catalogRow{32};int visible{},fieldVisible{};};
Layout Arrange(Config const&,Frame const&);
std::string AsciiKey(unsigned,bool shift,bool caps);
class Menu {
public:
    Config config;
    Locale locale{Locale::English};
    std::vector<Entry> entries;
    std::string query;
    bool open{},settings{},editing{},keyCapture{},searchFocus{},dirty{};
    bool nativeEntry{};
    bool OpenNative(Frame const&);
    int selected{},scroll{};
    float pointerX{},pointerY{};
    std::vector<WorkspaceSnapshot> workspaces;WorkspaceUi workspace;bool workspaceOpen{};
    TextEditor textEditor;bool textEditorActive{};std::uint64_t textGeneration{};
    bool clipboardPaste{},clipboardCopy{};std::uint64_t clipboardGeneration{};std::string clipboardText;
    void CompleteClipboard(std::uint64_t,std::string const&);
    WorkspaceSnapshot const* WorkspaceFor(std::string const&)const;
    SettingsDraft draft;
    bool configuring{},settingsBusy{},settingsChanged{};
    int fieldSelected{},fieldScroll{},captureField{-1};
    int settingCategory{},editField{-1},choiceField{-1},choiceSelected{};
    std::string numberText,numberError;bool numberReplace{};
    enum class Request {None,Load,Apply};
    Request settingsRequest{};
    unsigned long long settingsGeneration{};
    void CompleteSettings(unsigned long long,SettingsDraft);
    void Tick(Frame const&);
    // Local menu action only. No OS/game key injection or native navigation.
    bool PollShortcut(bool down,Frame const&,Modifiers={});
    bool PollKeyboard(std::array<bool,256> const&,Frame const&,bool caps=false,std::uint64_t now=0);
    bool TakePasteRequest(Frame const&);
    bool CaptureMapped(unsigned action,bool down,Frame const&);
    bool Input(Event const&,Frame const&);
    std::vector<int> Filtered() const;
    bool Captured() const {return pressed_ || dragging_;}
    std::string BindingCaptureCaption() const;
    bool BlocksMapped() const {if(open || editing || Captured())return true;for(bool b:capturedKeys_)if(b)return true;return false;}
private:
    MenuScene origin_{};
    bool focusSuspended_{},awaitingKeyboard_{};
    // Release barriers are not input ownership: they only suppress stale
    // presses on a retained page after foreground authority is re-established.
    std::array<bool,256> releaseKeys_{};unsigned releaseButtons_{};
    void SuspendInput();
    bool shortcutPrimed_{};unsigned shortcutKey_{},shortcutModifiers_{};
    modmenu::binding::Capture bindingCapture_;
    int bindingTarget_{-3};
    void BeginBindingCapture(int);
    bool CaptureBindingKey(unsigned,bool,Modifiers,std::array<bool,256> const& before);
    bool keyboardPrimed_{};
    bool pasteRequested_{};
    unsigned searchRepeatKey_{};std::uint64_t searchRepeatAt_{};
    bool SearchActive() const {return open && searchFocus && !settings && !configuring && !workspaceOpen && !keyCapture && !editing;}
    void SearchKey(unsigned,Modifiers);
    void AppendSearch(std::string const&);
    bool ConfigureKey(unsigned,Modifiers,bool repeat);
    void ConfigureText(std::string_view);
    bool CommitNumber();
    void ClearSettingControl();
    void CancelSlider();
    void MoveSlider(float x);
    void ConfigureClick(int action,int row,Event const&,Frame const&);
    bool ConfigurePress(Event const&,Frame const&);
    std::array<bool,256> previousKeys_{};
    std::array<unsigned,256> mappedKeys_{};
    unsigned mappedCount_{};bool mappedOverflow_{};
    bool pressed_{},dragging_{};
    int action_{},row_{-1};float dragX_{},dragY_{};
    int sliderField_{-1};SliderValue sliderStart_{};Rect sliderRail_{};
    float sliderWidth_{},sliderHeight_{};bool sliderChangedBefore_{};
    std::array<bool,256> held_{}, capturedKeys_{};
    unsigned capturedButtons_{};
};
}
