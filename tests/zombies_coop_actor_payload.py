"""Check the actual 1C actor-callback dependency, pointer imports and arities."""
from pathlib import Path
import argparse,hashlib,struct,sys

sys.dont_write_bytecode=True
sys.path.insert(0,str(Path(__file__).resolve().parent))
from zombies_coop_disconnect_payload import inspect,retail_hash

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--compiled',type=Path,required=True);a=p.parse_args()
    includes,imports,exports=inspect(a.compiled)
    assert any(name in ('scripts/shared/bots/_bot_combat','scripts/shared/bots/_bot_combat.gsc') for name in includes)
    for name,namespace in (('engage_threat','bot_combat'),('player_fake_death','zm')):
        assert (retail_hash(name),retail_hash(namespace),0,1) in imports,(name,namespace)
    assert (retail_hash('isalive'),retail_hash('eroded_coop'),1,2) in imports
    for name in ('bot_threat_guard_owned','bot_threat_guard_compatible','install_bot_threat_guard',
        'guarded_bot_threat_engage','fake_death_guard_owned','fake_death_guard_compatible',
        'install_fake_death_guard','guarded_player_fake_death','actor_lifecycle_on_connect',
        'actor_lifecycle_observer','install_actor_lifecycle_guards'):
        assert retail_hash(name) in exports,name
    data=a.compiled.read_bytes();start=struct.unpack_from('<I',data,0x20)[0];count=struct.unpack_from('<H',data,0x3a)[0]
    arities={struct.unpack_from('<I',data,start+index*20+8)[0]:data[start+index*20+16] for index in range(count)}
    assert arities[retail_hash('guarded_bot_threat_engage')]==0
    assert arities[retail_hash('guarded_player_fake_death')]==2
    assert not any(row[0] in (retail_hash('isstruct'),retail_hash('isvector'),retail_hash('gettype')) for row in imports)
    print('PASS: exact stock actor callback pointer imports, dependency and wrapper0/2 arities; compilation/object checks only')
    print('sha256='+hashlib.sha256(data).hexdigest())

if __name__=='__main__':main()
