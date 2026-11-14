#pragma once
#include "Workspace.h"
#include "TextEditor.h"
namespace menu {
struct Event;struct Frame;struct Paint;enum class Locale;
struct WorkspaceUi {
 WorkspaceSnapshot snapshot;std::size_t page{};int scroll{},focus{};
 std::string editing,notice,confirmation,pressed;std::uint64_t confirmRevision{};
 SettingTranslation resultMessage;std::uint64_t lastResult{};
 WorkspaceJson values=WorkspaceJson::object();TextEditor editor;
 bool close{},paste{},copy{};std::uint64_t editGeneration{};
 float inputWidth{},inputHeight{};
 void Open(WorkspaceSnapshot);void Accept(WorkspaceSnapshot);void Clear();void SuspendInput();
 bool Input(Event const&,Frame const&,bool repeat);
 std::vector<Paint> View(Frame const&,Locale)const;
private:
 bool Commit();void Activate(std::string const&,std::string const&,bool confirm=false);
};
}
