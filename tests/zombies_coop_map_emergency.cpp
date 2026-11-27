#include <thread>
#define main previous_public_api_suite_main
#include "zombies_coop_map_store.cpp"
#undef main

int main(int argc,char** argv){
  if(argc!=3)return 2;
  const auto regression_exit=previous_public_api_suite_main(argc,argv);
  const auto regression_checks=checks,regression_failures=failures;
  checks=0;failures=0;collision_failures=0;
  const auto raw=load(argv[1]);std::string error;error.reserve(1024);
  for(unsigned index=0;index<map_companion_names.size();++index){
    const auto name=map_companion_names[index];const auto payload=renamed(raw,name);
    {Store store;check(store.pin_map_companion(index,payload,error),"stale emergency pin");
      const auto old=store.capture_generation(name);check(store.begin_generation(false),"successor VM begins");const auto before=store.retained();
      int samples=0;const auto calls=allocation_calls.load();fail_all=true;
      auto* result=store.emergency_map_companion(name,old,[&]{++samples;return true;});fail_all=false;
      check(!result&&samples==0&&!store.exhausted()&&store.retained()==before&&allocation_calls==calls,"old lease neither samples native guard nor poisons/exposes successor and allocates nothing");
      check(contents(store.map_companion(name,error,store.capture_generation(name)))==payload,"successor remains fully usable after rejected old worker");
    }
    for(const unsigned realm:{1u,2u}){Store store;check(store.pin_map_companion(index,payload,error),"foreign realm pin");const auto generation=store.capture_generation(name).generation;
      const auto before=store.retained();int samples=0;const auto calls=allocation_calls.load();fail_all=true;
      auto* result=store.emergency_map_companion(name,{realm,generation},[&]{++samples;return true;});fail_all=false;
      check(!result&&!store.exhausted()&&store.retained()==before&&samples==0&&allocation_calls==calls,"client/invalid realm cannot poison server reservation");
      check(store.begin_generation(false),"foreign realm rejection leaves next server VM possible");
    }
    {Store store;check(store.pin_map_companion(index,payload,error),"native guard false pin");const auto lease=store.capture_generation(name);const auto before=store.retained();int samples=0;
      const auto calls=allocation_calls.load();fail_all=true;auto* result=store.emergency_map_companion(name,lease,[&]{++samples;return false;});fail_all=false;
      check(!result&&samples==1&&!store.exhausted()&&store.retained()==before&&allocation_calls==calls,"false current native guard leaves all store state and reservation untouched");
      check(store.begin_generation(false),"false guard did not latch emergency generation");const auto fresh=store.capture_generation(name);
      const auto calls2=allocation_calls.load();fail_all=true;result=store.emergency_map_companion(name,fresh,[]{return true;});fail_all=false;
      check(result&&contents(result)==payload&&store.exhausted()&&allocation_calls==calls2,"later current generation owns unused emergency without allocation");
      check(!store.begin_generation(false)&&!store.begin_generation(true),"exposed fatal emergency never reused across VM generations");
    }
    {Store store;check(store.pin_map_companion(index,payload,error),"guard throw pin");const auto lease=store.capture_generation(name);const auto before=store.retained();bool threw=false;
      try{store.emergency_map_companion(name,lease,[]()->bool{throw 7;});}catch(int){threw=true;}
      check(threw&&!store.exhausted()&&store.retained()==before&&store.begin_generation(false),"throwing native guard happens before poison or emergency generation latch");
    }
    {Store store;check(store.pin_map_companion(index,payload,error),"existing mutable emergency pin");const auto lease=store.capture_generation(name);auto* first=store.map_companion(name,error,lease);
      check(first!=nullptr,"normal current map cached");if(first)const_cast<char*>(first->buffer)[8]^=1;
      const auto before=store.retained();const auto calls=allocation_calls.load();fail_all=true;
      auto* denied=store.emergency_map_companion(name,lease,[]{return false;});fail_all=false;
      check(!denied&&!store.exhausted()&&store.retained()==before&&allocation_calls==calls,"guard rejection does not poison existing mutable current backing");
      const auto calls2=allocation_calls.load();fail_all=true;auto* fatal=store.emergency_map_companion(name,lease,[]{return true;});fail_all=false;
      check(fatal==first&&allocation_calls==calls2&&store.exhausted(),"accepted guard retains current mutable identity by its pinned immutable template");
    }
    {Store store;int samples=0;const auto before=store.retained();const auto calls=allocation_calls.load();fail_all=true;
      auto* result=store.emergency_map_companion("not-a-map",{0,1},[&]{++samples;return true;});fail_all=false;
      check(!result&&samples==0&&!store.exhausted()&&store.retained()==before&&allocation_calls==calls,"unknown identity is inert before native guard/poison");
    }
    for(const bool accept:{false,true}){Store store;check(store.pin_map_companion(index,payload,error),"locking order pin");const auto lease=store.capture_generation(name);
      std::atomic_bool guard_entered{false},advance_attempted{false};bool advanced=true;
      std::thread competing([&]{while(!guard_entered.load(std::memory_order_acquire))std::this_thread::yield();advance_attempted.store(true,std::memory_order_release);advanced=store.begin_generation(false);});
      auto* result=store.emergency_map_companion(name,lease,[&]{guard_entered.store(true,std::memory_order_release);while(!advance_attempted.load(std::memory_order_acquire))std::this_thread::yield();return accept;});
      competing.join();
      check(accept?(result&&store.exhausted()&&!advanced):(!result&&!store.exhausted()&&advanced),"native guard and poison/exposure are serialized against VM generation change");
    }
  }
  std::cout<<"{\"regression_checks\":"<<regression_checks<<",\"regression_failures\":"<<regression_failures<<",\"guarded_checks\":"<<checks<<",\"guarded_failures\":"<<failures<<",\"scope\":\"guarded map code store methods, own renamed metadata fixtures, native predicate stand-ins and real competing store threads; no engine/runtime proof\"}\n";
  return regression_exit||failures?1:0;
}
