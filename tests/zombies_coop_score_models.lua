local root = arg and arg[1] or "."

local checks=0
local function check(v,m) checks=checks+1; assert(v,m) end
local function pack(...) return {n=select('#',...),...} end
local source=assert(loadfile(root .. "/data/coop_scripts/score_models.lua"))
local events={'damage','death_normal','death_torso','death_neck','death_head','death_melee'}
local function setup(slots, mode)
  local w={records={},calls={},dvars={},originalCalls=0}
  CoD={isFrontend=false,isCampaign=false,isZombie=true}
  if mode=='frontend' then CoD.isFrontend=true elseif mode=='campaign' then CoD.isCampaign=true elseif mode=='mp' then CoD.isZombie=false end
  Enum={UIModelClientFieldType={CF_COUNTER=3}}
  Engine={DvarInt=function()return slots end,SetDvar=function(k,v)w.dvars[k]=v end,
    RegisterClientUIModelField=function(name,kind,bits,version,...)
      w.calls[#w.calls+1]=name;w.records[name]={kind,bits,version,tail=pack(...)};return 'registered',nil,9
    end}
  w.register=Engine.RegisterClientUIModelField
  function w:stock(mutate)
    for slot=0,3 do for _,event in ipairs(events)do
      local name='PlayerList.client'..slot..'.score_cf_'..event;local bits=event=='damage' and 3 or 2
      if mutate and slot==2 and event=='damage' then bits=4 end
      local result=pack(Engine.RegisterClientUIModelField(name,3,bits,1,'extra',nil))
      check(result.n==3 and result[1]=='registered' and result[2]==nil and result[3]==9,'native return tuple preserved')
    end end
  end
  function w:wrap(fn)
    return source()(function(a,b,c)
      self.originalCalls=self.originalCalls+1;check(a=='arg' and b==false and c==nil,'original arguments forwarded');
      fn(self);return 11,false,nil,'tail'
    end)
  end
  return w
end
for cap=5,8 do
  local w=setup(cap);local wrapped=w:wrap(function(x)x:stock()end);local r=pack(wrapped('arg',false,nil))
  check(r.n==4 and r[1]==11 and r[2]==false and r[3]==nil and r[4]=='tail','original return tuple')
  check(#w.calls==24 and w.dvars.zm_coopSchemaUIRevision==4,'only stock24 counters, revision4')
  check(Engine.RegisterClientUIModelField==w.register and w.originalCalls==1,'temporary API ownership restored')
  check(w.records['PlayerList.client0.score_cf_damage'].tail.n==2,'native nil tail retained')
end
for _,name in ipairs({'PlayerList.client4.score_cf_damage','PlayerList.client7.score_cf_death_melee','PlayerList.client4.score_cf_custom',
 'playerlist.client4.score_cf_damage','PlayerList.client04.score_cf_damage','PlayerList.client8.score_cf_damage','PlayerList.client-1.score_cf_damage',
 'PlayerList.client4.score_cf_DAMAGE','PlayerList.client4.score_cf_damage.extra','PlayerList.client0.score_cf_custom','PLAYERLIST.CLIENT4.SCORE_CF_DAMAGE','PlayerList.client.score_cf_custom'})do
  local w=setup(8);local wrapped=w:wrap(function(x)x:stock();Engine.RegisterClientUIModelField(name,3,9000,1)end)
  check(not pcall(wrapped,'arg',false,nil),'unknown extra ownership declines')
  check(#w.calls==24 and w.records[name]==nil,'rejected BEFORE native forwarding; no packed bits consumed')
  check(Engine.RegisterClientUIModelField==w.register and next(w.dvars)==nil,'failure restores and never publishes ready')
end
for _,mode in ipairs({'frontend','campaign','mp'})do
 local w=setup(8,mode);local wrapped=w:wrap(function(x)Engine.RegisterClientUIModelField('PlayerList.client4.score_cf_custom',3,1,1)end)
 check(wrapped('arg',false,nil)==11 and #w.calls==1 and next(w.dvars)==nil,'nonZM/frontend full passthrough')
 check(Engine.RegisterClientUIModelField==w.register,'nonZM register identity unchanged')
end
for _,cap in ipairs({0,1,4,9,18,5.5})do
 local w=setup(cap);local wrapped=w:wrap(function(x)Engine.RegisterClientUIModelField('PlayerList.client4.score_cf_custom',3,1,1)end)
 check(wrapped('arg',false,nil)==11 and #w.calls==1 and next(w.dvars)==nil,'stock or unsupported capacity passthrough')
end
do local w=setup(8);local wrapped=w:wrap(function(x)x:stock(true)end)
 check(not pcall(wrapped,'arg',false,nil) and #w.calls==12,'bad stock signature rejected before offending native call')
 check(Engine.RegisterClientUIModelField==w.register,'signature failure restores API') end
do local w=setup(8);local wrapped=w:wrap(function(x)x:stock();Engine.RegisterClientUIModelField('PlayerList.client0.score_cf_damage',3,3,1)end)
 check(not pcall(wrapped,'arg',false,nil) and #w.calls==24,'duplicate rejected before native call')end
do local w=setup(8);local foreign=function()return 'foreign'end;local wrapped=w:wrap(function(x)x:stock();Engine.RegisterClientUIModelField=foreign end)
 check(not pcall(wrapped,'arg',false,nil) and Engine.RegisterClientUIModelField==foreign,'foreign registration owner preserved; no revision publication')end
do local w=setup(8);local wrapped=w:wrap(function()error('original failed')end)
 check(not pcall(wrapped,'arg',false,nil) and Engine.RegisterClientUIModelField==w.register,'original error restores API')end
do local w=setup(8);local wrapped=w:wrap(function(x)x:stock();CoD.isZombie=false;Engine.RegisterClientUIModelField('PlayerList.client4.score_cf_custom',3,9000,1)end)
 check(not pcall(wrapped,'arg',false,nil) and #w.calls==24,'entry-mode ownership prevents flag flip from forwarding unknown extra counter')
 check(Engine.RegisterClientUIModelField==w.register and next(w.dvars)==nil,'mode-flip refusal restores API without publishing')end
do local w=setup(8);local wrapped=w:wrap(function(x)x:stock();Engine.RegisterClientUIModelField('Unrelated.customCounter',3,1,1)end)
 check(wrapped('arg',false,nil)==11 and #w.calls==25,'unrelated fields forwarded without blanket rejection')end
return {passed=true,checks=checks,scope='Actual proposed Lua chunk in Lua5.1 with explicit native-registration stand-in; no engine UI.'}
