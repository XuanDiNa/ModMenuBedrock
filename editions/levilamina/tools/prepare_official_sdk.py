"""Assemble exact upstream headers + an import library from official LL bytes.

No runtime is installed or rebuilt. Requires the official v26.60.7 git checkout,
official client release ZIP, and the Microsoft x64 librarian.
"""
import argparse, hashlib, json, shutil, struct, subprocess, zipfile
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument('--source',type=Path,required=True)
p.add_argument('--runtime-zip',type=Path,required=True)
p.add_argument('--lib-exe',type=Path,required=True)
p.add_argument('--output',type=Path,required=True)
p.add_argument('--skip-headers',action='store_true',help='Use already copied headers')
a=p.parse_args()
commit='dacca851fe5f8b97e292496f702db3a73b118f39'
assert subprocess.check_output(['git','-C',str(a.source),'rev-parse','HEAD'],text=True).strip()==commit
subprocess.run(['git','-C',str(a.source),'diff','--exit-code','--','src/ll/api','src/mc','src-client/ll/api','src-client/mc'],check=True,capture_output=True)
with zipfile.ZipFile(a.runtime_zip) as z:
    data=z.read('LeviLamina/LeviLamina.dll')
    notices={n:z.read(n) for n in ['COPYING','COPYING.LESSER']}
sha=hashlib.sha256(data).hexdigest().upper()
assert sha=='D2BB36559FEBF88D8A28CE65BF063DB0372C9D4FD6BAEFCCAC7CCF900812FF19'
sdk=a.output; sdk.mkdir(parents=True,exist_ok=True)
if not a.skip_headers:
    for base in ['src','src-client']:
        for sub in ['ll/api','mc']:
            source=a.source/base/sub;dest=sdk/'include'/sub
            subprocess.run(['robocopy',str(source),str(dest),'*.h','/E','/MT:16','/R:0','/W:0','/NFL','/NDL','/NJH','/NJS','/NP'],check=False)
            for header in source.rglob('*.h'):
                assert (dest/header.relative_to(source)).is_file()
off=struct.unpack_from('<I',data,60)[0];opt=off+24;count=struct.unpack_from('<H',data,off+6)[0];sections=[]
assert struct.unpack_from('<H',data,off+4)[0]==0x8664
for i in range(count):
    q=opt+240+i*40;vsize,rva,rawsize,raw=struct.unpack_from('<IIII',data,q+8)
    flags=struct.unpack_from('<I',data,q+36)[0];sections.append((rva,rawsize,raw,flags))
def fileoff(value):
    for va,size,raw,_ in sections:
        if va<=value<va+size:return raw+value-va
    raise ValueError(hex(value))
def string(value):
    q=fileoff(value);return data[q:data.index(b'\0',q)].decode('ascii')
exp=fileoff(struct.unpack_from('<I',data,opt+112)[0]);n=struct.unpack_from('<I',data,exp+24)[0]
functions,names,ordinals=struct.unpack_from('<III',data,exp+28)
exports=[]
for i in range(n):
    name=string(struct.unpack_from('<I',data,fileoff(names)+i*4)[0])
    ordinal=struct.unpack_from('<H',data,fileoff(ordinals)+i*2)[0]
    target=struct.unpack_from('<I',data,fileoff(functions)+ordinal*4)[0]
    executable=any(va<=target<va+size and flags&0x20000000 for va,size,_,flags in sections)
    exports.append(name+('' if executable else ' DATA'))
lib=sdk/'lib';lib.mkdir(exist_ok=True);definition=lib/'LeviLamina.def'
definition.write_text('LIBRARY LeviLamina.dll\nEXPORTS\n'+'\n'.join(exports)+'\n',encoding='ascii')
subprocess.run([str(a.lib_exe),'/nologo','/machine:x64','/def:'+str(definition),'/out:'+str(lib/'LeviLamina.lib')],check=True,capture_output=True)
for name,content in notices.items():(sdk/name).write_bytes(content)
meta={'source_tag':'v26.60.7','source_commit':commit,'official_runtime_sha256':sha,'runtime_bytes':len(data),'header_count':len(list((sdk/'include').rglob('*.h'))),'export_count':len(exports),'import_library':'derived from official PE exports; no handwritten ABI'}
(sdk/'provenance.json').write_text(json.dumps(meta,indent=2)+'\n',encoding='utf-8')
print(json.dumps(meta,indent=2))
