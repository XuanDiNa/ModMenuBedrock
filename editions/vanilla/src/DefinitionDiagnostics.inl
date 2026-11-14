// Passive diagnostic only. Never supplies permission or replacement bytes.
DefinitionProbe ProbeDefinition(void const* values,std::string_view target) noexcept {
    DefinitionProbe result;result.pause=target=="pause";
    if(target!="start" && target!="pause")return result;
    std::array<std::uintptr_t,3> bounds{};
    if(!ReadBytes(values,bounds.data(),sizeof bounds))return result;
    auto [begin,end,limit]=bounds;
    if(!begin || end<begin || limit<end || (end-begin)%48 || limit-begin>48*256)return result;
    result.validVector=true;result.layers=unsigned((end-begin)/48);
    result.inspected=std::min(result.layers,unsigned(result.items.size()));
    std::size_t budget=2*1024*1024;std::string base;bool haveBase=false;
    for(unsigned i=0;i<result.inspected;++i){
        auto& item=result.items[i];
        try{
            std::array<std::byte,48> record{};
            if(!ReadBytes(reinterpret_cast<void const*>(begin+i*48),record.data(),record.size()))continue;
            item.tag=Read<unsigned>(record.data());
            auto length=Read<std::size_t>(record.data(),24),capacity=Read<std::size_t>(record.data(),32);
            item.bytes=length;
            if(capacity<length || capacity>16*1024*1024){item.state=LayerProbeState::InvalidString;continue;}
            if(!length){item.state=LayerProbeState::Empty;continue;}
            if(length>1024*1024 || length>budget){item.state=LayerProbeState::Budget;continue;}
            budget-=length;
            auto chars=capacity<16?reinterpret_cast<char const*>(record.data()+8):Read<char const*>(record.data(),8);
            item.state=LayerProbeState::Error;std::string source(length,'\0');
            if(!ReadBytes(chars,source.data(),length)){item.state=LayerProbeState::Unreadable;continue;}
            item.sameAsBase=i && haveBase && source==base;
            std::string discarded;
            item.state=InsertModsDefinition(source,discarded,target)?LayerProbeState::Compatible:LayerProbeState::Unsupported;
            if(!i){base=std::move(source);haveBase=true;}
        }catch(...){item.state=LayerProbeState::Error;}
    }
    return result;
}
void ObserveDefinition(void const* values,std::string_view target) noexcept {
    if(!shared || (target!="start" && target!="pause"))return;
    auto slot=shared->definitionProbeClaims.load(std::memory_order_relaxed);
    auto count=unsigned(shared->definitionProbes.size());
    while(slot<count && !shared->definitionProbeClaims.compare_exchange_weak(slot,slot+1,std::memory_order_relaxed)){}
    if(slot>=count)return;
    shared->definitionProbes[slot]=ProbeDefinition(values,target);
    // Slots have one writer and remain immutable after publication. No native
    // pointer, JSON text, I/O, menu lock, or transform authority crosses here.
    shared->definitionProbeReady[slot].store(true,std::memory_order_release);
}
