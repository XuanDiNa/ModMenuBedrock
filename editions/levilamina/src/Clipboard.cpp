#include "ModMenu/Clipboard.h"
#include "ModMenu/TextEditor.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
namespace menu {
std::string ReadEditorClipboard(){std::string result;if(!OpenClipboard(nullptr))return {};auto h=GetClipboardData(CF_UNICODETEXT);if(h&&GlobalSize(h)<=65536){auto p=static_cast<wchar_t const*>(GlobalLock(h));if(p){auto bound=GlobalSize(h)/sizeof(wchar_t);std::size_t n=0;while(n<bound&&p[n])++n;if(n<bound&&n<=4096){auto count=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,p,int(n),nullptr,0,nullptr,nullptr);if(count>0&&count<=4096){result.resize(count);WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,p,int(n),result.data(),count,nullptr,nullptr);}}GlobalUnlock(h);}}CloseClipboard();return PlainText(result,0,4096)?result:std::string{};}
bool WriteEditorClipboard(std::string const&s){if(!PlainText(s,0,4096))return false;int n=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,s.data(),int(s.size()),nullptr,0);auto h=GlobalAlloc(GMEM_MOVEABLE,(std::size_t(n)+1)*sizeof(wchar_t));if(!h)return false;auto p=static_cast<wchar_t*>(GlobalLock(h));if(!p){GlobalFree(h);return false;}if(n)MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,s.data(),int(s.size()),p,n);p[n]=0;GlobalUnlock(h);if(!OpenClipboard(nullptr)){GlobalFree(h);return false;}bool ok=EmptyClipboard()&&SetClipboardData(CF_UNICODETEXT,h);CloseClipboard();if(!ok)GlobalFree(h);return ok;}
}
