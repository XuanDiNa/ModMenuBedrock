"""Generate an isolated UI-only xmake project from the real target definitions."""
import re
from pathlib import Path
root=Path(__file__).resolve().parents[1]
source=(root/'xmake.lua').read_text(encoding='utf-8')
source=source.replace('set_default(true)','set_default(false)',1)
source=source.replace('set_targetdir("build/$(plat)/$(arch)/$(mode)")','set_targetdir("bin/$(mode)")')
source=re.sub(r'"((?:src/|include|vendor/json|tests/|integration/|tools/)[^"\n]*)"',lambda m:'"'+root.as_posix()+'/'+m[1]+'"',source)
destination=root/'artifacts/regression';destination.mkdir(parents=True,exist_ok=True)
(destination/'xmake.lua').write_text(source,encoding='utf-8')
print(destination)
