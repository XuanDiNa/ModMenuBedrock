#include "ModMenu/Locale.h"
#include <algorithm>
#include <array>
namespace menu {
Locale LocaleFromCode(std::string_view code) noexcept{
 if(code.size()!=5)return Locale::English;
 std::array<char,5> c{};for(unsigned i=0;i<5;++i)c[i]=code[i]>='A' && code[i]<='Z'?char(code[i]+32):code[i];
 if(c[0]!='z' || c[1]!='h' || (c[2]!='_' && c[2]!='-'))return Locale::English;
 if(c[3]=='t' && c[4]=='w')return Locale::TraditionalChinese;
 return c[3]=='c' && c[4]=='n'?Locale::SimplifiedChinese:Locale::English;
}
namespace {
struct Translation{std::string_view en,tw,cn;};
constexpr Translation words[]={
 {"Workspace","工作頁","工作页"},{"Unavailable","無法使用","不可用"},{"No results","沒有結果","没有结果"},{"Confirm","確認","确认"},
 {"Explicit actions only; closing does not stop jobs.","操作須明確觸發；關閉頁面不會停止工作。","操作须明确触发；关闭页面不会停止工作。"},
 {"Waiting for provider state.","等候插件回報狀態。","等待插件返回状态。"},
 {"Action completed.","操作已完成。","操作已完成。"},{"Action failed.","操作失敗。","操作失败。"},
 {"Action is unavailable or busy.","操作目前無法使用或忙碌中。","操作当前不可用或正忙。"},
 {"Command submitted; waiting for provider result.","已提交操作，等候插件回報結果。","已提交操作，等待插件返回结果。"},
 {"State changed; activate the action again.","狀態已變更，請重新點選操作。","状态已变化，请重新选择操作。"},
 {"Workspace unavailable or changed; try again.","工作頁無法使用或已變更，請重試。","工作页不可用或已变化，请重试。"},
 {"Invalid text field.","文字欄位無效。","文本字段无效。"},{"Missing form value.","缺少表單欄位。","缺少表单字段。"},
 {"Waiting for the previous action result.","等候上一次操作結果。","等待上一次操作结果。"},
 {"Command queue is full.","操作佇列已滿。","操作队列已满。"},{"Form exceeds command size limit.","表單超過操作資料長度限制。","表单超过操作数据长度限制。"},
 {"Text is invalid or exceeds its UTF-8 byte limit.","文字無效或超過 UTF-8 長度限制。","文本无效或超过 UTF-8 长度限制。"},
 {"Clipboard is empty, unreadable or exceeds limits.","剪貼簿為空、無法讀取或超過限制。","剪贴板为空、无法读取或超过限制。"},
 {"Text does not match the declared format or UTF-8 byte limits.","文字不符合格式或 UTF-8 長度限制。","文本不符合格式或 UTF-8 长度限制。"},
 {"Enter a value on the declared slider step within its range.","請輸入範圍內且符合滑桿步進的數值。","请输入范围内且符合滑块步进的数值。"},
 {"Drag to adjust; click the number to type.","拖曳調整；點擊數字可直接輸入。","拖动调整；点击数字可直接输入。"},
 {"Advanced keys","進階按鍵","高级按键"},
 {"Fixed click interval","固定點擊間隔","固定点击间隔"},{"Stable cadence","穩定點擊節奏","稳定点击节奏"},
 {"Pause when cursor is visible","游標顯示時暫停","光标显示时暂停"},{"Invert hotbar wheel","反轉快捷列滾輪方向","反转快捷栏滚轮方向"},
 {"Extra cursor protection","額外游標保護","额外光标保护"},{"Hotbar key mapping","快捷列按鍵映射","快捷栏按键映射"},
 {"UI pause keys","介面暫停按鍵","界面暂停按键"},{"Jump delay","跳躍延遲","跳跃延迟"},
 {"Unhook after scan","掃描後解除掛鉤","扫描后解除挂钩"},{"Debug key","除錯鍵","调试键"},{"Scan key","掃描鍵","扫描键"},
 {"Hooks key","掛鉤切換鍵","挂钩切换键"},{"Reload settings key","重讀設定鍵","重读设置键"},{"Bind target key","綁定目標鍵","绑定目标键"},
 {"Jump key","跳躍鍵","跳跃键"},{"Decrease delay key","減少延遲鍵","减少延迟键"},{"Increase delay key","增加延遲鍵","增加延迟键"},
 {"Decrease step key","減少步幅鍵","减少步幅键"},{"Increase step key","增加步幅鍵","增加步幅键"},
 {"No movement check","不檢查游標移動","不检查光标移动"},{"No movement check (legacy)","不檢查游標移動（舊值）","不检查光标移动（旧值）"},
 {"Pause while cursor moves","游標移動時暫停","光标移动时暂停"},
 {"Type text; Enter confirms / Escape cancels.","輸入文字；Enter 確認，Escape 取消。","输入文本；Enter 确认，Escape 取消。"},
 {"Enter 0-5000 ms or a range such as 120-150. Avoid leading zeros.","請輸入 0–5000 ms 或範圍，例如 120-150。避免前置零。","请输入 0–5000 ms 或范围，例如 120-150。避免前导零。"},
 {"Use KEY=1 through KEY=9, separated by commas; no duplicate keys. Or use default.","使用 KEY=1 至 KEY=9，以逗號分隔，不可重複按鍵；或輸入 default。","使用 KEY=1 至 KEY=9，以逗号分隔，不可重复按键；或输入 default。"},
 {"Use supported key names separated by commas, or none. No duplicate keys.","使用支援的按鍵名稱，以逗號分隔，或輸入 none；不可重複按鍵。","使用支持的按键名称，以逗号分隔，或输入 none；不可重复按键。"},
 {"0 uses CPS. A positive interval overrides CPS and the 16 CPS stable-cadence cap.","0 表示使用 CPS；正數間隔取代 CPS，也不受穩定節奏 16 CPS 上限限制。","0 表示使用 CPS；正数间隔取代 CPS，也不受稳定节奏 16 CPS 上限限制。"},
 {"Use stable cadence, capped at 16 CPS only when fixed interval is 0. Native UI checks remain active.","使用穩定節奏，僅固定間隔為 0 時上限 16 CPS；原生介面檢查仍保留。","使用稳定节奏，仅固定间隔为 0 时上限 16 CPS；原生界面检查仍保留。"},
 {"Pause clicks when the Windows cursor is visible. Verified native UI checks always remain active.","Windows 游標顯示時暫停點擊；原生介面檢查始終保留。","Windows 光标显示时暂停点击；原生界面检查始终保留。"},
 {"Reverse the wheel direction used to track the selected hotbar slot.","反轉追蹤目前快捷列格子時使用的滾輪方向。","反转跟踪当前快捷栏格子时使用的滚轮方向。"},
 {"Extra cursor heuristic. Visible-cursor pause is controlled separately; native UI checks cannot be disabled.","額外游標判斷。游標顯示暫停另行設定；0 與舊值 1 均不檢查移動，原生介面檢查不可停用。","额外光标判断。光标显示暂停单独设置；0 与旧值 1 均不检查移动，原生界面检查不可停用。"},
 {"Map keys to hotbar slots, e.g. R=1,E=2,1=7. Use default for standard number keys. No duplicate keys.","映射按鍵至快捷列，例如 R=1,E=2,1=7。default 使用原版數字鍵，不可重複按鍵。","映射按键至快捷栏，例如 R=1,E=2,1=7。default 使用原版数字键，不可重复按键。"},
 {"Keys that toggle an extra UI pause, e.g. MOUSE5,TAB; none disables this extra pause, not native checks.","切換額外介面暫停的按鍵，例如 MOUSE5,TAB；none 只停用此項，不停用原生檢查。","切换额外界面暂停的按键，例如 MOUSE5,TAB；none 仅停用此项，不停用原生检查。"},
 {"Actual jump delay: 134 for fixed delay, or 120-150 for a random range. Bounds: 0-5000 ms. Avoid leading zeros.","實際跳躍延遲：134 為固定值，120-150 為隨機範圍。限制 0–5000 ms，避免前置零。","实际跳跃延迟：134 为固定值，120-150 为随机范围。限制 0–5000 ms，避免前导零。"},
 {"Retain the plugin's automatic unhook behavior after scanning. Advanced setting; normally leave ON.","保留插件掃描後自動解除掛鉤的行為。進階設定，一般保持開啟。","保留插件扫描后自动解除挂钩的行为。高级设置，一般保持开启。"},
 {"Existing diagnostic or target-binding shortcut; changing it does not execute the action. Escape unbinds the key.","插件現有的診斷或目標綁定快捷鍵；修改不會執行動作。Escape 解除綁定。","插件现有的诊断或目标绑定快捷键；修改不会执行动作。Escape 解除绑定。"},
 {"Single keyboard key, including digits and punctuation. Saved in the plugin's native VK format. Escape unbinds the key.","單一鍵盤按鍵，支援數字與標點；以插件原生 VK 格式儲存。Escape 解除綁定。","单个键盘按键，支持数字与标点；以插件原生 VK 格式保存。Escape 解除绑定。"},
 {"Unbound","無綁定","未绑定"},
 {"Press key (Esc unbinds)","請按鍵（Esc 解除綁定）","请按键（Esc 解除绑定）"},
 {"Press a primary key (Esc unbinds)","請按主鍵（Esc 解除綁定）","请按主键（Esc 解除绑定）"},
 {"Mods","模組","模组"},{"Settings","設定","设置"},{"Done","完成","完成"},{"Configure","設定插件","配置插件"},
 {"Back","返回","返回"},{"Apply","套用","应用"},{"Cancel","取消","取消"},{"Reset","重設","重置"},{"Reset all","全部重設","全部重置"},
 {"Search mods...","搜尋模組…","搜索模组…"},{"Native client plugins","原生客戶端插件","原生客户端插件"},
 {"No matching plugins","沒有符合的插件","没有匹配的插件"},{"No description provided.","未提供說明。","未提供说明。"},
 {"Version unknown","版本未知","版本未知"},{"Author not provided","未提供作者","未提供作者"},{"Website","網站","网站"},
 {"No settings available","沒有可用設定","没有可用设置"},{"Configurable","可設定","可配置"},
 {"Ready","就緒","就绪"},{"Inactive","未啟用","未启用"},{"DLL loaded (Ready unknown)","DLL 已載入（就緒狀態未知）","DLL 已加载（就绪状态未知）"},
 {"Installed (not loaded)","已安裝（未載入）","已安装（未加载）"},{"Disabled manifest","載入設定已停用","加载配置已禁用"},
 {"Entry missing/unsafe","入口遺失或不安全","入口缺失或不安全"},
 {"All","全部","全部"},{"General","一般","常规"},{"Controls","操作","操作"},{"Diagnostics","診斷","诊断"},{"Dragging","拖曳","拖动"},
 {"Mouse wheel","滑鼠滾輪","鼠标滚轮"},{"Timing","時間","时间"},{"Reminder","提醒","提醒"},
 {"Enabled","啟用","启用"},{"Restock below","低於此數量補貨","低于此数量补货"},{"Hold preview key","按住預覽鍵","按住预览键"},
 {"Toggle key","切換鍵","切换键"},{"Notifications","通知","通知"},{"Debug logging","診斷日誌","诊断日志"},
 {"Right-click drag","右鍵拖曳","右键拖动"},{"Collect matching stacks","收集相同物品","收集相同物品"},{"Shift-transfer drag","Shift 拖曳搬運","Shift 拖动搬运"},
 {"Search order","搜尋順序","搜索顺序"},{"Scroll direction","滾動方向","滚动方向"},{"Scroll scaling","滾動數量","滚动数量"},
 {"Clicks per second","每秒點擊次數","每秒点击次数"},{"Required hotbar slot","指定快捷列格子","指定快捷栏格子"},
 {"Require left button held","需按住滑鼠左鍵","需按住鼠标左键"},{"Delay step","延遲調整步幅","延迟调整步幅"},
 {"Reminder overlay","提醒浮層","提醒浮层"},{"Reminder duration","提醒顯示時間","提醒显示时间"},{"Reminder opacity","提醒不透明度","提醒不透明度"},
 {"First to last","由前往後","从前到后"},{"Last to first","由後往前","从后到前"},{"Normal","正常","正常"},
 {"Inverted","反向","反向"},{"Position-aware","依格子位置","按格子位置"},{"Position-aware inverted","依格子位置反向","按格子位置反向"},
 {"Proportional","依滾動量","按滚动量"},{"Always one","每次一個","每次一个"},{"ON","開啟","开启"},{"OFF","關閉","关闭"},
 {"Press key (Esc cancels)","請按鍵（Esc 取消）","请按键（Esc 取消）"},{"Press a key...","請按一個鍵…","请按一个键…"},
 {"Working...","處理中…","处理中…"},{"Unsaved changes","有未儲存變更","有未保存更改"},{"No pending changes","沒有待儲存變更","没有待保存更改"},
 {"Type a number; Enter confirms / Escape cancels.","輸入數字；Enter 確認，Escape 取消。","输入数字；Enter 确认，Escape 取消。"},
 {"Edit draft only. Apply saves; Cancel discards.","目前只修改草稿。套用後儲存，取消則捨棄。","当前仅修改草稿。应用后保存，取消则放弃。"},
 {"Move Mods button","移動模組按鈕","移动模组按钮"},{"Reset position","重設位置","重置位置"},
 {"Saved to this plugin's config.json","儲存至本插件的 config.json","保存至本插件的 config.json"},
 {"F10 is the default, not a reserved key.","F10 是預設鍵，並非保留鍵。","F10 是默认键，并非保留键。"},
 {"Reset config.json if a key is unreachable.","無法使用快捷鍵時可重設 config.json。","无法使用快捷键时可重置 config.json。"},
 {"Only this menu's settings are changed.","只變更本選單的設定。","仅更改本菜单的设置。"},
 {"Drag Mods; Escape returns to Settings","拖曳模組按鈕；Escape 返回設定","拖动模组按钮；Escape 返回设置"},
 {"Enable this plugin's configured feature; this does not load or unload its DLL.","啟用插件功能；不會載入或卸載 DLL。","启用插件功能；不会加载或卸载 DLL。"},
 {"Restock strictly below this count: 17 means 16 or fewer. Main hand, main inventory only.","數量嚴格低於設定值才補貨：17 表示 16 或以下。只補主手，來源為主背包。","数量严格低于设定值才补货：17 表示 16 或以下。仅补主手，来源为主背包。"},
 {"Hold this key while aiming at a supported container to preview its contents.","對準支援的容器時按住此鍵，預覽其內容。","对准支持的容器时按住此键，预览其内容。"},
 {"Press to toggle edge protection. Escape or Backspace unbinds the key.","按鍵切換邊緣保護。Escape 或 Backspace 解除綁定。","按键切换边缘保护。Escape 或 Backspace 解除绑定。"},
 {"Show the plugin's toggle notifications.","顯示插件切換通知。","显示插件切换通知。"},
 {"Write extra diagnostic logs. Leave OFF for normal play.","記錄額外診斷日誌。一般遊玩建議關閉。","记录额外诊断日志。正常游玩建议关闭。"},
 {"Right-drag places one item in each visited slot, including revisits.","右鍵拖曳時，每經過一格放入一件物品，重複經過也有效。","右键拖动时，每经过一格放入一件物品，重复经过也有效。"},
 {"Left-drag with an item collects matching stacks; Shift transfers matching stacks.","持物品左鍵拖曳可收集相同物品；按住 Shift 則搬運。","持物品左键拖动可收集相同物品；按住 Shift 则搬运。"},
 {"Shift + held left-drag quickly transfers visited stacks to the other inventory.","Shift 加左鍵拖曳，將經過的物品堆快速搬至另一側。","Shift 加左键拖动，将经过的物品堆快速搬至另一侧。"},
 {"Use the mouse wheel over a slot to transfer individual items.","在格子上滾動滑鼠滾輪，逐件搬運物品。","在格子上滚动鼠标滚轮，逐件搬运物品。"},
 {"Order used to search slots in the other inventory.","在另一側背包搜尋物品的格子順序。","在另一侧背包搜索物品的格子顺序。"},
 {"Choose normal, reversed or slot-position-aware transfer direction.","選擇正常、反向或依格子位置判斷的搬運方向。","选择正常、反向或按格子位置判断的搬运方向。"},
 {"Transfer proportionally to scrolling, or exactly one item per wheel action.","依滾動量搬運，或每次滾輪操作只搬一件物品。","按滚动量搬运，或每次滚轮操作仅搬一件物品。"},
 {"Requested click rate. The plugin's existing safety and UI gates still apply.","目標點擊頻率；插件原有安全檢查與介面限制仍有效。","目标点击频率；插件原有安全检查与界面限制仍有效。"},
 {"Required hotbar slot, numbered 1 to 9.","允許使用的快捷列格子，編號為 1 至 9。","允许使用的快捷栏格子，编号为 1 至 9。"},
 {"Require the physical left mouse button to be held; does not enable the session.","必須按住實體滑鼠左鍵；此設定不會啟用工作階段。","必须按住实体鼠标左键；此设置不会启用会话。"},
 {"Adjustment step for the plugin's delay controls, not the jump delay itself.","延遲控制每次調整的步幅，並非跳躍延遲值。","延迟控制每次调整的步幅，并非跳跃延迟值。"},
 {"Enable the reminder overlay.","啟用提醒浮層。","启用提醒浮层。"},
 {"How long a reminder remains visible. Type an exact value or adjust in 50 ms steps.","提醒顯示時間；可輸入精確值，或以 50 ms 為步幅調整。","提醒显示时间；可输入精确值，或以 50 ms 为步幅调整。"},
 {"Reminder opacity from 0 (transparent) to 255 (opaque).","提醒的不透明度：0 為透明，255 為不透明。","提醒的不透明度：0 为透明，255 为不透明。"},
 {"Apply saves JSON; live application is not confirmed. Disabled DLLs need restart.","套用會儲存 JSON；尚未確認即時生效。停用的 DLL 需重啟。","应用会保存 JSON；尚未确认实时生效。禁用的 DLL 需重启。"},
 {"Saved with backup. Live reload expected, not confirmed; disabled DLLs need restart.","已儲存並備份；預期熱載入，但尚未確認。停用的 DLL 需重啟。","已保存并备份；预期热加载，但尚未确认。禁用的 DLL 需重启。"},
 {"Config changed externally. Cancel and reopen Configure.","設定檔已被外部修改，請取消並重新開啟設定。","配置文件已被外部修改，请取消并重新打开设置。"},
 {"Config replaced externally. Cancel and reopen Configure.","設定檔已被外部替換，請取消並重新開啟設定。","配置文件已被外部替换，请取消并重新打开设置。"},
 {"Invalid UTF-8 config","設定檔不是有效 UTF-8","配置文件不是有效 UTF-8"},{"Config nesting limit","設定檔巢狀層數超限","配置文件嵌套层数超限"},
 {"Duplicate config field","設定欄位重複","配置字段重复"},{"Config must be an object","設定檔必須是物件","配置文件必须是对象"},
 {"Config missing, unsafe, locked or too large","設定檔遺失、不安全、被鎖定或過大","配置文件缺失、不安全、被锁定或过大"},
 {"Cannot read config","無法讀取設定檔","无法读取配置文件"},{"Plugin entry does not match adapter","插件入口與設定介面不符","插件入口与设置接口不符"},
 {"Installed plugin manifest unavailable","無法取得插件載入設定","无法获取插件加载配置"},{"Invalid setting group","設定群組無效","设置分组无效"},
 {"No verified settings adapter","沒有已驗證的設定介面","没有已验证的设置接口"},{"Unsafe plugin directory","插件目錄不安全","插件目录不安全"},
 {"Invalid settings draft","設定草稿無效","设置草稿无效"},{"Unexposed setting edit refused","拒絕修改未公開的設定","拒绝修改未公开的设置"},
 {"Result exceeds plugin config limit","結果超過設定檔大小限制","结果超过配置文件大小限制"},{"Config is read-only","設定檔為唯讀","配置文件为只读"},
 {"Cannot create config transaction file","無法建立設定暫存檔","无法创建设置临时文件"},{"Cannot write config transaction file","無法寫入設定暫存檔","无法写入设置临时文件"},
 {"Atomic save failed; exact backup retained. Cancel and inspect the current config before retrying.","安全儲存失敗，已保留原始備份。請取消並檢查設定檔後重試。","安全保存失败，已保留原始备份。请取消并检查配置文件后重试。"},
 {"Expected boolean: ","需要布林值：","需要布尔值："},{"Expected key name: ","需要按鍵名稱：","需要按键名称："},
 {"Expected key or text: ","需要按鍵或文字：","需要按键或文本："},
 {"Invalid number: ","數字無效：","数字无效："},{"Unsupported setting value: ","不支援的設定值：","不支持的设置值："},{"Invalid value: ","值無效：","值无效："},
 {"Default: ","預設：","默认："},{" Range: "," 範圍："," 范围："},{"; step ","；步幅 ","；步幅 "},
 {"Apply saves JSON; the plugin declares hot reload. Application is not acknowledged.","套用會儲存 JSON；插件宣告支援即時重載，但未回報套用結果。","应用会保存 JSON；插件声明支持实时重载，但未回报应用结果。"},
 {"Apply saves JSON; restart Minecraft for these settings to take effect.","套用會儲存 JSON；需重新啟動 Minecraft 才能生效。","应用会保存 JSON；需重新启动 Minecraft 才能生效。"},
 {"Saved with backup. The plugin declares hot reload; application is not acknowledged.","已儲存並備份；插件宣告支援即時重載，但未回報套用結果。","已保存并备份；插件声明支持实时重载，但未回报应用结果。"},
 {"Saved with backup. Restart Minecraft for these settings to take effect.","已儲存並備份；需重新啟動 Minecraft 才能生效。","已保存并备份；需重新启动 Minecraft 才能生效。"},
 {"Settings description or manifest changed. Cancel and reopen Configure.","設定描述或載入設定已變更；請取消並重新開啟設定。","设置描述或加载配置已变更；请取消并重新打开设置。"},
 {"Description plugin version mismatch","設定描述不支援此插件版本","设置描述不支持此插件版本"},
 {"Unsupported settings description version","不支援此設定描述規格版本","不支持此设置描述规范版本"},
 {"Description plugin ID mismatch","設定描述的插件 ID 不符","设置描述的插件 ID 不匹配"},
 {"Description plugin entry mismatch","設定描述的 DLL 入口不符","设置描述的 DLL 入口不匹配"},
 {"Open key: ","開啟鍵：","打开键："},{"Button scale: ","按鈕縮放：","按钮缩放："}
};
}
std::string Tr(Locale locale,std::string_view s){
 if(locale!=Locale::English)for(auto const& word:words)if(word.en==s)return std::string(locale==Locale::TraditionalChinese?word.tw:word.cn);
 return std::string(s);
}
std::string DisplayCaption(SettingTranslation const& t,std::string_view fallback,Locale locale,bool external){
 if(!external)return Tr(locale,fallback);
 if(locale==Locale::TraditionalChinese && !t.tw.empty())return t.tw;
 if(locale==Locale::SimplifiedChinese && !t.cn.empty())return t.cn;
 return t.en.empty()?std::string(fallback):t.en;
}
std::string DisplayCategory(SettingsDraft const& draft,std::string_view category,Locale locale){
 if(category!="All")for(auto const& field:draft.fields)if(field.category==category && !field.categoryText.en.empty())
  return DisplayCaption(field.categoryText,category,locale,field.external);
 return Tr(locale,category);
}
std::string DisplayOption(SettingField const& f,std::size_t index,Locale locale){
 if(index>=f.options.size())return {};
 if(f.type==SettingType::Key)return f.options[index];
 if(f.external && index<f.optionText.size())return DisplayCaption(f.optionText[index],f.options[index],locale,true);
 return f.external?f.options[index]:Tr(locale,f.options[index]);
}
std::string DisplaySetting(SettingField const& f,Locale locale){
 if(f.type==SettingType::Choice)if(auto n=std::get_if<int>(&f.value);n && *n>=0)return DisplayOption(f,std::size_t(*n),locale);
 if(f.type==SettingType::Key || f.type==SettingType::Binding){auto caption=SettingKeyCaption(f);return caption=="Unbound"?Tr(locale,caption):caption;}
 auto s=SettingText(f);return f.type==SettingType::Text?s:Tr(locale,s);
}
std::string DisplayMessage(std::string_view s,Locale locale){
 if(s.empty())return {};
 auto translated=Tr(locale,s);if(locale==Locale::English || translated!=s)return translated;
 for(auto prefix:{"Expected boolean: ","Expected key name: ","Expected key or text: ","Invalid number: ","Unsupported setting value: ","Invalid value: "})
  if(s.starts_with(prefix))return Tr(locale,prefix)+Tr(locale,s.substr(std::string_view(prefix).size()));
 constexpr std::string_view prefix="Enter a whole number from ";
 if(s.starts_with(prefix) && s.ends_with('.')){
  auto range=s.substr(prefix.size(),s.size()-prefix.size()-1);auto split=range.find(" to ");
  if(split!=std::string_view::npos)return (locale==Locale::TraditionalChinese?"請輸入 ":"请输入 ")+std::string(range.substr(0,split))+"–"+std::string(range.substr(split+4))+(locale==Locale::TraditionalChinese?" 的整數。":" 的整数。");
 }
 // Third-party JSON parser exceptions are not stable translation keys.
 // Keep the diagnostic intact after a localized summary.
 return (locale==Locale::TraditionalChinese?"設定錯誤：":"设置错误：")+std::string(s);
}
float TextWidth(std::string_view s) noexcept{
 float width{};for(unsigned char c:s)if((c&0xc0)!=0x80)width+=c>=224?10.f:6.f;return width;
}
}
