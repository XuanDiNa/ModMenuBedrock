"""Bounded offline PE/package inspection; never executes Minecraft."""
import argparse,hashlib,json,struct,zipfile,re
from pathlib import Path,PurePosixPath
def require(ok,message):
    if not ok:raise RuntimeError(message)
def pe(path,workspace_api=False,workspace_example=False):
    data=path.read_bytes();require(data[:2]==b'MZ','MZ missing')
    off=struct.unpack_from('<I',data,60)[0];require(64<=off<=4096 and data[off:off+4]==b'PE\0\0','PE missing')
    machine,count,_,_,_,optsize,flags=struct.unpack_from('<HHIIIHH',data,off+4)
    opt=off+24;require(machine==0x8664 and flags&0x2000 and struct.unpack_from('<H',data,opt)[0]==0x20b,'Not an x64 DLL')
    require(0<count<=96 and optsize==240,'Unexpected PE layout')
    sections=[]
    for n in range(count):
        p=opt+optsize+n*40;size,rva,rawsize,raw=struct.unpack_from('<IIII',data,p+8)
        require(raw+rawsize<=len(data),'Section exceeds file');sections.append((rva,rawsize,raw))
    def rva(value):
        for start,size,raw in sections:
            if start<=value<start+size:return raw+value-start
        raise RuntimeError('Unmapped RVA')
    def string(value):
        p=rva(value);end=data.find(b'\0',p,min(len(data),p+1024));require(end>=p,'Unbounded PE string');return data[p:end].decode('ascii')
    exports=set();export=struct.unpack_from('<I',data,opt+112)[0];require(export or workspace_example,'No exports')
    if export:
        p=rva(export);n=struct.unpack_from('<I',data,p+24)[0];names=struct.unpack_from('<I',data,p+32)[0];require(n<=100,'Too many exports')
        for i in range(n):exports.add(string(struct.unpack_from('<I',data,rva(names)+i*4)[0]))
    expected=set() if workspace_example else {'MMB_GetStatus','MMB_GetVersion'}
    if workspace_api:expected.update({'MMB_WorkspaceRegisterV1','MMB_WorkspacePublishV1','MMB_WorkspacePollV1','MMB_WorkspaceUnregisterV1'})
    require(exports==expected,'Unexpected exports: '+str(exports))
    imports=[];symbols=[];imp=struct.unpack_from('<I',data,opt+120)[0]
    if imp:
        for n in range(128):
            p=rva(imp)+n*20;original,stamp,chain,name,thunk=struct.unpack_from('<IIIII',data,p)
            if not any((original,stamp,chain,name,thunk)):break
            imports.append(string(name))
            for i in range(4096):
                entry=struct.unpack_from('<Q',data,rva(original or thunk)+i*8)[0]
                if not entry:break
                if entry>>63:continue
                symbols.append(string(entry+2))
            else:raise RuntimeError('Import bound exceeded')
        else:raise RuntimeError('Import descriptor bound exceeded')
    require(not any('levilamina' in x.lower() or 'moss' in x.lower() for x in imports),'Unexpected UI/API dependency')
    forbidden={'SendInput','mouse_event','keybd_event','WriteProcessMemory','CreateRemoteThread','WinHttpSendRequest'}
    require(not forbidden.intersection(symbols),'Forbidden import: '+str(forbidden.intersection(symbols)))
    require(not any('vcruntime' in x.lower() or 'msvcp' in x.lower() for x in imports),'Shipping plugin needs nonstatic CRT')
    return {'machine':'AMD64','exports':sorted(exports),'imports':imports,'sha256':hashlib.sha256(data).hexdigest().upper(),'size':len(data)}
def package(path,payload):
    root=payload.name+'/';files={p.relative_to(payload).as_posix():p for p in payload.rglob('*') if p.is_file()}
    with zipfile.ZipFile(path) as z:
        actual={n for n in z.namelist() if not n.endswith('/')}
        require(actual=={root+n for n in files},'ZIP payload differs from frozen tree')
        require(len(actual)==len([n for n in z.namelist() if not n.endswith('/')]),'Duplicate ZIP entries')
        for n,p in files.items():
            name=root+n;require(not PurePosixPath(name).is_absolute() and '..' not in PurePosixPath(name).parts,'Unsafe ZIP path')
            require(hashlib.sha256(z.read(name)).digest()==hashlib.sha256(p.read_bytes()).digest(),'ZIP hash mismatch: '+name)
    checks=(payload/'SHA256SUMS.txt').read_text().splitlines()
    require(len(checks)==len(files)-1,'Incomplete checksums')
    covered=set()
    for line in checks:
        digest,name=line.split('  ',1);require(name in files and name not in covered,'Invalid checksum member');covered.add(name)
        require(hashlib.sha256(files[name].read_bytes()).hexdigest().upper()==digest,'Payload checksum mismatch: '+name)
    require(covered==set(files)-{'SHA256SUMS.txt'},'Checksum coverage mismatch')
    return {'members':len(files),'zip_sha256':hashlib.sha256(path.read_bytes()).hexdigest().upper()}
p=argparse.ArgumentParser();p.add_argument('--dll',type=Path,required=True);p.add_argument('--zip',type=Path);p.add_argument('--payload',type=Path);g=p.add_mutually_exclusive_group();g.add_argument('--workspace-api',action='store_true');g.add_argument('--workspace-example',action='store_true');a=p.parse_args()
report={'dll':pe(a.dll,a.workspace_api,a.workspace_example)}
if a.zip:require(a.payload is not None,'payload required');report['package']=package(a.zip,a.payload)
print(json.dumps(report,indent=2))
