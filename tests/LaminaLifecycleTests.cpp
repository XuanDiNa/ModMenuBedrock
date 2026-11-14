#include "ModMenu/LaminaLifecycle.h"
#include <iostream>
struct Operations {
    bool allowed=true, prepared=true, started=true;
    unsigned admissions{},prepares{},starts{},stops{};
    bool Admit(){++admissions;return allowed;}
    bool Prepare(){++prepares;return prepared;}
    bool Start(){++starts;return started;}
    void Stop(){++stops;}
};
int main(){
 unsigned checks{},failures{};auto check=[&](bool ok,char const*n){++checks;if(!ok){++failures;std::cerr<<"FAIL "<<n<<'\n';}};
 check(menu::LaminaVisibleStatus(3,true)==2,"late Ready publication cannot reopen Workspace admission after disable");
 check(menu::LaminaVisibleStatus(3,false)==3&&menu::LaminaVisibleStatus(1,true)==1&&menu::LaminaVisibleStatus(4,true)==4,"active Ready and refusal/fault diagnostics are retained");
 {
  menu::LaminaLifecycle life;Operations op;
  check(!life.Enable(op)&&!op.starts,"enable cannot precede native load admission");
  check(life.Load(op)&&op.prepares==1&&!op.starts,"load prepares early definitions without starting UI worker");
  check(life.Load(op)&&op.admissions==1&&op.prepares==1,"duplicate load has no side effects");
  check(life.Enable(op)&&life.Enable(op)&&op.starts==1,"enable starts exactly one worker");
  check(life.Disable(op)&&op.stops==1,"disable stops active callbacks and joins worker");
  check(life.Disable(op)&&op.stops==1,"duplicate disable is harmless");
  check(!life.Enable(op)&&op.starts==1,"stopped native hooks require a process restart");
 }
 for(unsigned failure=0;failure<3;++failure){
  menu::LaminaLifecycle life;Operations op;op.allowed=failure!=0;op.prepared=failure!=1;op.started=failure!=2;
  bool loaded=life.Load(op);check(loaded==(failure==2),"missing/wrong environment or failed native guard refuses load");
  if(loaded)check(!life.Enable(op),"worker start failure refuses enable");
  check(!life.Enable(op),"failed lifecycle cannot retry partially installed hooks");
  check(failure?op.stops==1:!op.stops,"only admitted partial startup needs deactivation");
 }
 {menu::LaminaLifecycle life;Operations op;check(life.Load(op)&&life.Disable(op)&&op.stops==1&&!op.starts,"disable before enable deactivates early definitions");}
 std::cout<<checks<<" lifecycle checks / "<<failures<<" failures\n";return failures?1:0;
}
