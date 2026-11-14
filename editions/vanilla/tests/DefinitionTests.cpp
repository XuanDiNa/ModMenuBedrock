#include "ModMenu/Definition.h"
#include "json.hpp"
#include <fstream>
#include <iostream>
int checks{},failures{};void Check(bool b,char const* s){++checks;if(!b){++failures;std::cerr<<"FAIL "<<s<<'\n';}}
int main(int argc,char** argv){using namespace menu;using Json=nlohmann::json;
 if(argc>2){std::cerr<<"Usage: ModMenuDefinitionTests.exe [private-menu-fixtures.json]\n";return 2;}
 Check(DefinitionNamespace("ui/start_screen.json")=="start" && DefinitionNamespace("ui/pause_screen.json")=="pause","exact resource paths bind only to their own namespaces");
 Check(DefinitionNamespace("ui/settings_screen.json").empty() && DefinitionNamespace("other/pause_screen.json").empty() && DefinitionNamespace("ui/PAUSE_screen.json").empty(),"unrelated paths and case variants have no transform authority");
 std::string source=R"({"namespace":"start","main_button@common.button":{},"stacked_row":{"type":"panel"},"main_buttons_panel":{"controls":[{"stacked_rows":{"type":"stack_panel","controls":[{"play@start.play_button_stack":{}},{"settings@start.stacked_row":{"keep":7}}]}}]},"unknown":{"retain":true}})";
 std::string result="unchanged";Check(InsertModsDefinition(source,result),"native definition transform accepts verified vanilla structure");
 if(result=="unchanged")return 1;auto j=Json::parse(result);auto const& rows=j["main_buttons_panel"]["controls"][0]["stacked_rows"]["controls"];
 Check(rows.size()==3 && rows[0].contains("play@start.play_button_stack") && rows[1].contains("mmb_mods@start.stacked_row") && rows[2]["settings@start.stacked_row"]["keep"]==7,"Mods follows Play without replacing Settings");
 Check(j["unknown"]["retain"]==true && j["mmb_button@start.main_button"]["$pressed_button_name"]=="button.mmb_open_mods","unknown fields retained and unique event emitted");
 std::string again;Check(InsertModsDefinition(result,again) && again==result,"repeat load cannot duplicate Mods row");
 auto refuse=[&](std::string text,char const* name){std::string out="retained";Check(!InsertModsDefinition(text,out) && out=="retained",name);};
 refuse(R"({"namespace":"other"})","other namespace passes through");refuse(R"({"namespace":"start"})","unsupported custom layout passes through");
 refuse(R"({"namespace":"start","namespace":"start"})","duplicate keys refused");refuse(std::string(1024*1024+1,' '),"definition size bounded");
 auto ambiguous=Json::parse(source);auto& controls=ambiguous["main_buttons_panel"]["controls"][0]["stacked_rows"]["controls"];controls.push_back(controls[0]);refuse(ambiguous.dump(),"ambiguous Play rows refused");
 auto collision=Json::parse(source);collision["mmb_button@start.main_button"]={{"$pressed_button_name","button.menu_settings"}};refuse(collision.dump(),"existing foreign control cannot be overwritten");
 auto duplicateStack=Json::parse(source);duplicateStack["main_buttons_panel"]["controls"].push_back(duplicateStack["main_buttons_panel"]["controls"][0]);refuse(duplicateStack.dump(),"ambiguous stacked panel refused");
 auto noSettings=Json::parse(source);noSettings["main_buttons_panel"]["controls"][0]["stacked_rows"]["controls"].erase(1);refuse(noSettings.dump(),"incomplete layout refuses partial integration");
 auto reordered=Json::parse(source);auto& reorderedRows=reordered["main_buttons_panel"]["controls"][0]["stacked_rows"]["controls"];std::swap(reorderedRows[0],reorderedRows[1]);refuse(reordered.dump(),"custom Settings-before-Play order cannot violate insertion contract");
 Check(InsertModsDefinition("// native UI permits comments\n"+source,again),"native JSON comments can be normalized safely");
 auto deep=source;for(int n=0;n<70;++n)deep="{\"nest\":"+deep+"}";refuse(deep,"excessive nested UI input refuses before recursion exhaustion");
 {
  std::string pause=R"({"namespace":"pause","pause_button_template@common_buttons.light_text_button":{"size":["100%",28]},"settings_button@pause.pause_button_template":{"$pressed_button_name":"button.menu_settings"},"menu_background@pause.transparent_background":{"controls":[{"button_panel":{"type":"stack_panel","controls":[{"return_to_game_button@pause.return_to_game_button":{}},{"settings_button@pause.settings_button":{"ignored":"$pause_menu_icon_settings_layout"}},{"settings@pause.vertical_padding":{"size":[0,3],"ignored":"$pause_menu_icon_settings_layout"}},{"store_button_panel@pause.store_button_panel":{"retain":true}},{"quit_button@pause.quit_button":{}}]}}]},"unknown":{"keep":7}})";
  std::string out;bool accepted=InsertModsDefinition(pause,out);
  Check(accepted,"verified pause layout receives native Mods below Settings");
  if(accepted){
   auto menu=Json::parse(out);auto const& pauseRows=menu["menu_background@pause.transparent_background"]["controls"][0]["button_panel"]["controls"];
   Check(pauseRows.size()==7 && pauseRows[1].contains("settings_button@pause.settings_button") && pauseRows[2].contains("settings@pause.vertical_padding") && pauseRows[3].contains("mmb_mods@pause.mmb_button") && pauseRows[4].contains("mmb_spacing@pause.vertical_padding") && pauseRows[5]["store_button_panel@pause.store_button_panel"]["retain"]==true && pauseRows[6].contains("quit_button@pause.quit_button"),"pause insertion retains Settings spacer subsequent rows and Quit");
   Check(menu["mmb_button@pause.pause_button_template"]["$pressed_button_name"]=="button.mmb_open_mods" && menu["unknown"]["keep"]==7,"pause button reuses template and shared event without dropping unknown data");
   std::string repeat;Check(InsertModsDefinition(out,repeat) && repeat==out,"pause transformation is idempotent");
  }
  auto missing=Json::parse(pause);missing["menu_background@pause.transparent_background"]["controls"][0]["button_panel"]["controls"].erase(1);refuse(missing.dump(),"pause without Settings is refused");
  auto duplicate=Json::parse(pause);auto& duplicateRows=duplicate["menu_background@pause.transparent_background"]["controls"][0]["button_panel"]["controls"];duplicateRows.push_back(duplicateRows[1]);refuse(duplicate.dump(),"ambiguous pause Settings is refused");
  auto foreign=Json::parse(pause);foreign["mmb_button@pause.pause_button_template"]={{"$pressed_button_name","button.menu_quit"}};refuse(foreign.dump(),"pause cannot overwrite an existing foreign control");
  auto wrongOrder=Json::parse(pause);auto& customRows=wrongOrder["menu_background@pause.transparent_background"]["controls"][0]["button_panel"]["controls"];std::swap(customRows[0],customRows[4]);refuse(wrongOrder.dump(),"pause with reversed Resume and Quit ordering refuses");
 }
 if(argc==1)std::cout<<"Private vanilla-menu fixtures skipped; synthetic layout tests still run.\n";
 if(argc==2){std::ifstream fixtures(argv[1]);
 Check(bool(fixtures),"fresh exact-version main and pause resource fixtures are available");
 if(fixtures){Json versions;fixtures>>versions;
  for(auto version:{"1.26.45.01","1.26.51.01","1.26.52.03"})for(auto name:{"start_screen.json","pause_screen.json"}){
   Check(versions.contains(version) && versions[version].contains(name),"each exact version supplies both native menus");
   auto actual=versions.at(version).at(name).get<std::string>();std::string transformed;
   bool ok=InsertModsDefinition(actual,transformed,name==std::string("start_screen.json")?"start":"pause");
   Check(ok,"exact-version main and pause definitions accept native button insertion");
   std::string repeated;Check(ok && InsertModsDefinition(transformed,repeated) && repeated==transformed,"all three versions retain idempotent native menu insertion");
  }
 }}
 std::cout<<checks<<" checks / "<<failures<<" failures\n";return failures?1:0;
}
