#pragma once
namespace menu {
inline int LaminaVisibleStatus(int status,bool stopped){return stopped&&status==3?2:status;}
// LL client load precedes pack initialization; enable occurs at InitComplete.
// Live engine hooks intentionally live until process exit and are not reloadable.
class LaminaLifecycle {
    enum class Phase {Initial,Prepared,Running,Stopped,Failed};
    Phase phase{Phase::Initial};
public:
    template<class Ops> bool Load(Ops& op){
        if(phase==Phase::Prepared||phase==Phase::Running)return true;
        if(phase!=Phase::Initial)return false;
        phase=Phase::Failed;
        if(!op.Admit())return false;
        if(!op.Prepare()){op.Stop();return false;}
        phase=Phase::Prepared;return true;
    }
    template<class Ops> bool Enable(Ops& op){
        if(phase==Phase::Running)return true;
        if(phase!=Phase::Prepared)return false;
        phase=Phase::Failed;
        if(!op.Start()){op.Stop();return false;}
        phase=Phase::Running;return true;
    }
    template<class Ops> bool Disable(Ops& op){
        if(phase==Phase::Prepared||phase==Phase::Running){phase=Phase::Stopped;op.Stop();}
        return true;
    }
};
}
