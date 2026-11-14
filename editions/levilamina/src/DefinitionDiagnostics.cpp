#include "ModMenu/DefinitionDiagnostics.h"
#include <algorithm>
namespace menu {
namespace {
char const* Name(LayerProbeState state){
 switch(state){
 case LayerProbeState::Empty:return "empty";
 case LayerProbeState::Compatible:return "compatible-layout";
 case LayerProbeState::Unsupported:return "unsupported-layout";
 case LayerProbeState::InvalidString:return "invalid-string";
 case LayerProbeState::Budget:return "budget-limit";
 case LayerProbeState::Error:return "probe-error";
 default:return "unreadable";
 }
}
}
std::string DescribeDefinitionProbe(DefinitionProbe const& probe){
 std::string out="definition-probe target="+std::string(probe.pause?"pause":"start")+
  " policy="+(!probe.validVector?"invalid-vector":probe.layers==1?"single-layer":"multiple-layers-refused")+
  " layers="+std::to_string(probe.layers)+" inspected="+std::to_string(probe.inspected);
 for(unsigned i=0;i<std::min(probe.inspected,unsigned(probe.items.size()));++i){auto const& item=probe.items[i];
  out+=" [i="+std::to_string(i)+" tag="+std::to_string(item.tag)+" bytes="+std::to_string(item.bytes)+
   " state="+Name(item.state)+" same-base="+std::to_string(item.sameAsBase)+"]";
 }
 return out;
}
}
