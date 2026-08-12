//Adjust these values to change how special rounds function

#define SPECIAL_ROUNDS_ENABLED                                              true            //Whether or not special rounds happen

#define SPECIAL_ROUNDS_FROST_ROUNDS_ENABLED                                 false           //Activates frost round effects of slowing player outside

#define SPECIAL_ROUNDS_METHONE_ROUNDS_ENABLED                               false           //Activates radhound round effects of spawning radiation pools around the map
#define SPECIAL_ROUNDS_METHONE_ROUNDS_RANDOMISE_POOL_LOCATIONS              true            //Spawn pools randomly around the map in a radius around the dog spawn location structs (if set to true, use the pool position prefab instead)
#define SPECIAL_ROUNDS_METHONE_ROUNDS_RANDOM_POOL_LOCATION_COUNT            20              //How many random pools to spawn across the map (adjust according to your map size, test the amount in game)

#define SPECIAL_ROUND_SPAWN_DOGS                                            true            //Spawn dogs in special rounds
#define SPECIAL_ROUND_MIDROUND_SPAWN_DOGS                                   false           //Spawn dogs in normal rounds
#define SPECIAL_ROUND_DELAY_DOG_SPAWNS                                      true           //Delay the next ai spawn until dog spawn finishes (only valid for normal hellhounds)

#define SPECIAL_ROUND_SPAWN_SUPER_SPRINTERS                                 false           //Spawn super sprinters in special rounds
#define SPECIAL_ROUND_MIDROUND_SPAWN_SUPER_SPRINTERS                        false           //Spawn super sprinters in normal rounds

#define SPECIAL_ROUND_START_MINIMUM                                         5               //Determines the lower bound of which rounds can be the first special round (BO3 default is 5)
#define SPECIAL_ROUND_START_MAXIMUM                                         5               //Determines the upper bound of which rounds can be the first special round (BO3 default is 7)

#define SPECIAL_ROUND_INTERVAL_MIN                                          5               //Determines the lower bound of how big the gap is between special rounds (BO3 default is 5)
#define SPECIAL_ROUND_INTERVAL_MAX                                          5               //Determines the upper bound of how big the gap is between special rounds (BO3 default is 5)

#define SPECIAL_ROUND_ANNOUNCER_VOX_DELAY                                   1               //Time until the announcer voiceline plays when a special round starts (BO3 default is 1)
#define SPECIAL_ROUND_SPAWN_START_DELAY                                     6               //Time after the announcer vox delay until spawns start (BO3 default is 6)

#define SPECIAL_ROUND_SPAWN_WAIT_TIME                                       1               //Time between spawns during special rounds