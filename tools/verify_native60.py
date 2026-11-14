"""Pinned 26.60 native additions and byte-exact legacy projections."""
import hashlib
import json
import re
import subprocess
from pathlib import Path

root = Path(__file__).resolve().parents[1]
audit = json.loads((root/'docs/native60-source-sha256.json').read_text())
for name, expected in audit.items():
    assert hashlib.sha256((root/name).read_bytes()).hexdigest().upper() == expected, name

def old(name):
    return subprocess.check_output(['git', 'show', '861e98a:'+name], cwd=root).decode().replace('\r\n', '\n')

def replace(text, before, after=''):
    assert text.count(before) == 1, before
    return text.replace(before, after)

compat=(root/'src/NativeCompatibility.inl').read_text()
legacy=re.sub(r'// Exact 1\.26\.60\.05 IDA/ABI review:[\s\S]+?(?=constexpr BuildDescriptor builds\[\])','',compat,count=1)
legacy=re.sub(r'    \{GameBuild::V60,[\s\S]+?&launcher60Current\},\n','',legacy,count=1)
assert legacy == old('src/NativeCompatibility.inl'), 'Legacy identities/addresses/guards changed'

header=(root/'include/ModMenu/NativeProfile.h').read_text()
header=replace(header, 'Unsupported,V45,V51,V52,V60', 'Unsupported,V45,V51,V52')
header=replace(header, ' unsigned screenValid{152},screenGui{168},screenClip{120},uiMode{125};\n')
assert header==old('include/ModMenu/NativeProfile.h')

profile=(root/'src/NativeProfile.cpp').read_text()
profile=replace(profile,' ExactImage const* additionalImage{};\n')
profile=replace(profile,'constexpr NativeLayout layout60{1512,456,968,416,424,1728,1536,80,64,1152,1157,828,832,144,160,112,117};\n')
profile=re.sub(r'BuildDescriptor const\* Descriptor\(ImageIdentity const& id\)\{[\s\S]+?(?=bool Hash\()',
              'BuildDescriptor const* Descriptor(ImageIdentity const& id){for(auto const& d:builds)for(auto const& image:d.images)if(SameIdentity(id,image.identity))return &d;return nullptr;}\n',profile,count=1)
profile=replace(profile,'        if(d->additionalImage && SameIdentity(id,d->additionalImage->identity))exact=d->additionalImage;\n')
assert profile==old('src/NativeProfile.cpp'), 'Legacy verification policy changed'

host=(root/'src/NativeHost.cpp').read_text()
host=re.sub(r'std::string_view AuditedDefinition60\([\s\S]+?(?=std::string_view AuditedDefinition\()', '',host,count=1)
host=replace(host,'    if(profile->Build()==GameBuild::V60)return AuditedDefinition60(values);\n')
host=replace(host,'if(ReadBytes(bytes,source.data(),length) &&\n               (profile->Build()!=GameBuild::V60 || VerifiedMenu60(source)==scope->target) &&\n               InsertModsDefinition(source,scope->bytes,scope->target))','if(ReadBytes(bytes,source.data(),length) && InsertModsDefinition(source,scope->bytes,scope->target))')
host=replace(host,'void* user,unsigned offset=125) noexcept{','void* user) noexcept{')
host=replace(host,'if(!screen || !submit || (offset!=125 && offset!=117))','if(!screen || !submit)')
host=replace(host,'static_cast<unsigned char*>(screen)+offset','static_cast<unsigned char*>(screen)+125')
host=replace(host,'SubmitPaints,&work,profile->Layout().uiMode','SubmitPaints,&work')
host=replace(host,'    auto const& layout=profile->Layout();\n    auto screen=Read<void*>(context,16),validity=Read<void*>(screen,layout.screenValid),gui=Read<void*>(screen,layout.screenGui);',
                   '    auto screen=Read<void*>(context,16),validity=Read<void*>(screen,152),gui=Read<void*>(screen,168);')
host=replace(host,'Read<unsigned char>(screen,layout.screenClip)','Read<unsigned char>(screen,120)')
host=replace(host,'    // The optional stencil diagnostic has no independently established 60 layout.\n    if(profile->Build()!=GameBuild::V60){auto gpu=Read<void*>(screen,16),stencil=Read<void*>(gpu,264);receipt.stencil=Read<unsigned short>(stencil);}',
                   '    auto gpu=Read<void*>(screen,16),stencil=Read<void*>(gpu,264);receipt.stencil=Read<unsigned short>(stencil);')
host=replace(host,'#include "DefinitionDiagnostics.inl"\n')
host=replace(host,'    ObserveDefinition(values,target);\n')
baseline=old('src/NativeHost.cpp')
baseline=replace(baseline,'(GetAsyncKeyState(VK_MENU)&0x8000)!=0,(GetKeyState(VK_CAPITAL)&1)!=0};',
                          '(GetAsyncKeyState(VK_MENU)&0x8000)!=0,(GetKeyState(VK_CAPITAL)&1)!=0,\n                (GetAsyncKeyState(VK_LMENU)&0x8000)!=0};')
baseline=replace(baseline,'id==ModsEventId','IsModsOpenEvent(id)')
assert host==baseline, 'Unexpected host/scene/input/hook mutation'
print('60 native audit passed: pinned additions; all legacy projections and hook/foreground/scene/ownership policies match.')
