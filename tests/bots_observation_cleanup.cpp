#include "../src/client/component/bots/cleanup_policy.hpp"
#include <cassert>
#include <iostream>
#include <type_traits>
using namespace bots;
static_assert(!std::is_convertible_v<observation::snapshot,policy::session_snapshot>);
static unsigned checks{};static void check(bool value){++checks;assert(value);}
int main(){
  for(int mode=-1;mode<=4;mode++)for(int cap=0;cap<=19;cap++)for(int frozen=0;frozen<=9;frozen++)
  for(bool host:{false,true})for(bool initialized:{false,true})for(bool failed:{false,true})for(bool source:{false,true}){
    observation::snapshot s{12,17,mode,cap,frozen,host,initialized,failed,source,"zm_zod","zm_zod"};
    bool expected=host&&initialized&&mode>=0&&mode<=2&&cap>=1&&cap<=18;
    if(expected&&mode==0){expected=!failed&&cap<=8&&frozen<=8;if(expected)expected=(cap>4||frozen>4)?cap>=5&&frozen==cap&&source:frozen==0||frozen==cap;}
    check(observation::valid(s)==expected);
  }
  observation::snapshot live{12,17,0,8,8,true,true,false,true,"zm_zod","zm_zod"};check(observation::valid(live));
  policy::session_snapshot allocation{12,0,8,0,true,true,false,8,"zm_zod"};
  check(!policy::valid(allocation)); // Closing admission stays closed.
  check(observation::valid(live)); // The distinct observation grants no allocation.
  auto changed=live;changed.capacity=4;check(!observation::valid(changed));
  for(const auto map:{"","core_frontend","bad/map","bad map","bad\"map"}){changed=live;changed.map=map;changed.frozen_map=map;check(!observation::valid(changed));}
  changed=live;changed.map=std::string(65,'a');changed.frozen_map=changed.map;check(!observation::valid(changed));
  for(const int epoch:{-1,0}){changed=live;changed.native_epoch=epoch;check(!observation::valid(changed));}
  changed=live;changed.epoch=0;check(!observation::valid(changed));
  for(int which=0;which<7;which++){
    changed=live;if(which==0)changed.epoch++;else if(which==1)changed.native_epoch++;else if(which==2){changed.capacity=7;changed.frozen_capacity=7;}else if(which==3){changed.map="zm_tomb";changed.frozen_map="zm_tomb";}else if(which==4){changed.mode=1;changed.frozen_capacity=0;}else if(which==5)changed.host=false;else changed.initialized=false;
    check(!observation::same_session(live,changed));
  }
  check(observation::same_session(live,live));

  static_assert(!std::is_convertible_v<cleanup::snapshot,policy::session_snapshot>);
  cleanup::snapshot end{live,true};check(cleanup::valid(end));
  for(int slot=-1;slot<=18;slot++)for(int generation=0;generation<=13;generation++)
  for(bool connected:{false,true})for(bool test:{false,true})for(bool ending:{false,true}){
    auto current=end;current.ending_verified=ending;
    const bool expected=slot>=0&&slot<8&&generation==12&&connected&&test&&ending;
    check(cleanup::may_cleanup_test_client(end,current,generation,slot,connected,test)==expected);
  }
  for(int mode=0;mode<=3;mode++)for(int cap=1;cap<=18;cap++){
    auto current=end;current.identity.mode=mode;current.identity.capacity=cap;current.identity.frozen_capacity=cap;
    check(cleanup::valid(current)==(mode==0&&cap>=5&&cap<=8));
  }
  for(int change=0;change<10;change++){
    auto current=end;
    if(change==0)current.identity.source_current=false;else if(change==1)current.identity.schema_failed=true;
    else if(change==2)current.identity.initialized=false;else if(change==3)current.identity.host=false;
    else if(change==4)current.identity.epoch++;else if(change==5)current.identity.native_epoch++;
    else if(change==6)current.identity.map="zm_tomb";else if(change==7)current.identity.frozen_map="zm_tomb";
    else if(change==8)current.identity.frozen_capacity=7;else current.ending_verified=false;
    check(!cleanup::may_cleanup_test_client(end,current,12,7,true,true));
  }
  // The original strict path still rejects lost schema without an ending identity.
  auto strict=allocation;strict.schema_ready=false;check(!policy::may_drop_test_client(strict,strict,12,7,true,true));
  std::cout<<"checks="<<checks<<" observation_only=true allocation_unchanged=true engine=false\n";
}
