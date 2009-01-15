#include <stdio.h>

#include "protos.h"
#include "rhyodin.h"
#include "carceri.h"
#include "lucertole.h"

#if HASH
extern struct hash_header room_db;
#else
extern struct room_data *room_db;
#endif
extern struct index_data *mob_index;
extern struct index_data *obj_index;
void boot_the_shops();
void assign_the_shopkeepers();

struct special_proc_entry 
{
  int vnum;
  int (*proc)( struct char_data *, int, char *, char_data *, int );
};

struct RoomSpecialProcEntry 
{
  int vnum;
  int (*proc)( struct char_data *, int, char *, struct room_data *, int );
};

int fighter_mage( struct char_data *ch, int cmd, char *arg, 
                  struct char_data *mob, int type );
int fighter_cleric( struct char_data *ch, int cmd, char *arg, 
                    struct char_data *mob, int type);
int cleric_mage( struct char_data *ch, int cmd, char *arg, 
                 struct char_data *mob, int type);

int SputoVelenoso( struct char_data *ch, int cmd, char *arg,
                   struct char_data *mob, int type );

int Pungiglione( struct char_data *ch, int cmd, char *arg, 
                 struct char_data *mob, int type );

int Ezmerelda( struct char_data *pChar, int iCmd, char *szArg,
               struct char_data *pMob, int itype );

/* ********************************************************************
*  Assignments                                                        *
******************************************************************** */

/* murder is disabled for now */
#define MAX_MUTYPE 16
int is_murdervict(struct char_data *ch)
{
  int i;
  int mutype[] =
  {
    3060,  /* killing these mobs will make the PC a murderer */
    3067,
    3069,
    3072,
    3141,
    3661,
    3662,
    3663,
    3682,
    16101,
    17809,
    18215,
    18222,
    18223,
    22601,
    27011,
    -1
    
  };
  
  if( ch->nr >= 0 )
  {
    for( i = 0; mutype[i] >= 0; i++ ) 
    {
      if (mob_index[ch->nr].iVNum == mutype[i])
        return TRUE;
    }
  }
   
  return FALSE;
}


/* assign special procedures to mobiles */
void assign_mobiles()
{

  static struct special_proc_entry specials[] = 
  {
    { 1, puff },
    { 2, Ringwraith },
    { 3, tormentor },
    /*{ 4, Inquisitor},*/
    /*{ 6, AcidBlob },*/

    { 30, MageGuildMaster }, 
    { 31, ClericGuildMaster }, 
    { 32, ThiefGuildMaster }, 
    { 33, WarriorGuildMaster },
    { 34, MageGuildMaster }, 
    { 35, ClericGuildMaster }, 
    { 36, ThiefGuildMaster }, 
    { 37, WarriorGuildMaster },
    { 39, creeping_death},  

    {199, AGGRESSIVE},
    /*{200, AGGRESSIVE},*/
                
    /*
     **  D&D Standard MOBS
     */

    { 223, ghoul },           /* ghoul */
    { 236, ghoul },            /* ghast */
    { 227, snake },        /* spider */
    { 230, BreathWeapon }, /* baby black */
    { 232, blink },       /* blink dog */
    { 233, BreathWeapon }, /* baby blue */
    { 239, shadow },      /* shadow    */
    { 240, SputoVelenoso },       /* toad      */
    { 243, BreathWeapon }, /* teenage white */
    { 251, CarrionCrawler },
    { 262, regenerator },
    { 267, Devil},
    { 269, Demon},
    { 271, regenerator },
    { 248, snake },       /* snake       */
    { 249, snake },       /* snake       */
    { 250, snake },       /* snake       */

    /* Creature per le carceri di Alma */

    { 300, KyussSon },
    { 303, VermeDellaMorte }, 
    { 308, Minicius },
    { 309, Piovra },
    { 314, Moribondo },

    {600, DruidChallenger},
    {601, DruidChallenger},
    {602, DruidChallenger},
    {603, DruidChallenger},
    {604, DruidChallenger},
    {605, DruidChallenger},
    {606, DruidChallenger},
    {607, DruidChallenger},
    {608, DruidChallenger},
    {609, DruidChallenger},
    {610, DruidChallenger},
    {611, DruidChallenger},
    {612, DruidChallenger},
    {613, DruidChallenger},
    {614, DruidChallenger},
    {615, DruidChallenger},
    {616, DruidChallenger},
    {617, DruidChallenger},
    {618, DruidChallenger},
    {619, DruidChallenger},
    {620, DruidChallenger},
    {621, DruidChallenger},
    {622, DruidChallenger},
    {623, DruidChallenger},
    {624, DruidChallenger},
    {625, DruidChallenger},
    {626, DruidChallenger},
    {627, DruidChallenger},
    {628, DruidChallenger},
    {629, DruidChallenger},
    {630, DruidChallenger},
    {631, DruidChallenger},
    {632, DruidChallenger},
    {633, DruidChallenger},
    {634, DruidChallenger},
    {635, DruidChallenger},
    {636, DruidChallenger},
    {637, DruidChallenger},
    {638, DruidChallenger},
    {639, DruidChallenger},
    {640, DruidChallenger},
    {641, DruidGuildMaster},
    {642, DruidGuildMaster},

    {651, MonkChallenger},
    {652, MonkChallenger},
    {653, MonkChallenger},
    {654, MonkChallenger},
    {655, MonkChallenger},
    {656, MonkChallenger},
    {657, MonkChallenger},
    {658, MonkChallenger},
    {659, MonkChallenger},
    {660, MonkChallenger},
    {661, MonkChallenger},
    {662, MonkChallenger},
    {663, MonkChallenger},
    {664, MonkChallenger},
    {665, MonkChallenger},
    {666, MonkChallenger},
    {667, MonkChallenger},
    {668, MonkChallenger},
    {669, MonkChallenger},
    {670, MonkChallenger},
    {671, MonkChallenger},
    {672, MonkChallenger},
    {673, MonkChallenger},
    {674, MonkChallenger},
    {675, MonkChallenger},
    {676, MonkChallenger},
    {677, MonkChallenger},
    {678, MonkChallenger},
    {679, MonkChallenger},
    {680, MonkChallenger},
    {681, MonkChallenger},
    {682, MonkChallenger},
    {683, MonkChallenger},
    {684, MonkChallenger},
    {685, MonkChallenger},
    {686, MonkChallenger},
    {687, MonkChallenger},
    {688, MonkChallenger},
    {689, MonkChallenger},
    {690, MonkChallenger},
    {691, monk_master},

    /*
     * frost giant area 
     */

    { 9416, fido},
    { 9418, BreathWeapon },
    { 9419, BreathWeapon },
    { 9424, StormGiant },
    { 9426, MonkChallenger },
    { 9430, regenerator },
    { 9431, snake },
    { 9435, SputoVelenoso },
    { 9436, fido },

    /*
     **   shire
     */
    
    { 1031, receptionist },
    
    { 6001, real_rabbit},
    { 6005, real_fox},
    
    /*
     * prydain
     */
    
    { 6601, PrydainGuard},
    { 6602, PrydainGuard},
    { 6605, PrydainGuard},
    { 6606, PrydainGuard},
    { 6619, PrydainGuard},
    { 6620, PrydainGuard},
    { 6614, PrydainGuard},
    { 6609, BreathWeapon},
    { 6642, BreathWeapon},
    { 6640, jugglernaut },
    { 6635, BreathWeapon},
    { 6625, Demon},
    { 6638, StatTeller},

    /*
     **  Hill giants 1
     */
    
    { 9213, CarrionCrawler},
    { 9217, BreathWeapon},

    /*
     **  chessboard
     */
    
#if 0
    { 1400, chess_game },  /* black pieces */
    { 1401, chess_game },
    { 1402, chess_game },
    { 1403, chess_game },
    { 1404, chess_game },
    { 1405, chess_game },
    { 1406, chess_game },
    { 1407, chess_game },
    { 1408, chess_game },
    { 1409, chess_game },
    { 1410, chess_game },
    { 1411, chess_game },
    { 1412, chess_game },
    { 1413, chess_game },
    { 1414, chess_game },
    { 1415, chess_game },
    
    { 1448, chess_game },  /* white pieces */
    { 1449, chess_game },
    { 1450, chess_game },
    { 1451, chess_game },
    { 1452, chess_game },
    { 1453, chess_game },
    { 1454, chess_game },
    { 1455, chess_game },
    { 1456, chess_game },
    { 1457, chess_game },
    { 1458, chess_game },
    { 1459, chess_game },
    { 1460, chess_game },
    { 1461, chess_game },
    { 1462, chess_game },
    { 1463, chess_game },
    
#endif

    { 1499, sisyphus }, 
    { 1471, paramedics }, 
    { 1470, jabberwocky },
    { 1472, flame }, 
    { 1437, banana }, 
    { 1428, jugglernaut },
    { 1495, delivery_elf },  
    { 1493, delivery_beast },

    /*
     **  Bandits Temple
     */
    
    { 2113, ghoul },
    { 2115, ghost },
    { 2116, ghost },
    { 2117, druid_protector },


    /* Astral plane */
    { 2715, astral_portal },
    { 2716, astral_portal },
    { 2717, astral_portal },
    { 2718, astral_portal },
    { 2719, astral_portal },
    { 2720, astral_portal },
    { 2721, astral_portal },
    { 2722, astral_portal },
    { 2723, astral_portal },
    { 2724, astral_portal },
    { 2725, astral_portal },
    { 2726, astral_portal },
    { 2727, astral_portal },
    { 2728, astral_portal },
    { 2729, astral_portal },
    { 2730, astral_portal },
    { 2731, astral_portal },
    { 2732, astral_portal },

    /*
     **  Valley of the Mage
     */
    
    { 21136, snake },
    { 21107, RustMonster},
    { 21108, wraith},
    { 21111, web_slinger},
    { 21112, trapper},
    { 21114, troguard},
    { 21121, trogcook},
    { 21122, shaman},
    { 21123, troguard},
    { 21124, golgar},
    { 21118, troguard},
    { 21119, troguard},

    { 21130, Valik},
    
    { 21135, regenerator},
    { 21138, ghostsoldier},
    { 21139, ghostsoldier},

    { 21140, keystone},
    { 21141, lattimore},
    { 21142, guardian},

    { 21144, troguard},
    { 21145, troguard},
    { 21146, coldcaster},
    { 21147, RustMonster},

    /*
     **  New Thalos
     */
    
    { 3600, MageGuildMaster },
    { 3601, ClericGuildMaster },
    { 3602, WarriorGuildMaster },
    { 3603, ThiefGuildMaster },
    { 3604, receptionist},
    { 3656, NewThalosGuildGuard},
    { 3657, NewThalosGuildGuard},
    { 3658, NewThalosGuildGuard},
    { 3659, NewThalosGuildGuard},
    { 3661, SultanGuard},   /* wandering */
    { 3662, SultanGuard},   /* not */
    { 3682, SultanGuard},   /* royal */
    { 3670, BreathWeapon},  /* Cryohydra */
    { 3674, BreathWeapon},  /* Behir */
    { 3675, BreathWeapon},  /* Chimera */
    { 3676, BreathWeapon},  /* Couatl */
    { 3689, NewThalosMayor }, /* Guess */
    { 3644, fido},
    
    /*
     **  Skexie
     */
    
    { 15821, vampire },
    
    /*
     **  Challenge
     */
    
    { 15858, BreathWeapon },
    { 15864, sisyphus },
    { 15868, snake },
    { 15879, BreathWeapon },
    
    /*
     **  abyss
     */
    
    { 25001, Keftab }, 
    { 25009, BreathWeapon },    /* hydra */
    { 25002, vampire },                /* Crimson */
    { 25003, StormGiant },      /* MistDaemon */
    { 25006, StormGiant },      /* Storm giant */
    { 25014, StormGiant },      /* DeathKnight */    
    { 25009, BreathWeapon },    /* hydra */
    { 25017, AbyssGateKeeper }, /* Abyss Gate Keeper */
    { 25025, acid_monster},
    { 25026, acid_monster},

    /*
     **  Paladins guild
     */
    
    { 3028, PaladinGuildGuard},
    { 21363, PaladinGuildmaster},    
    
    /*
     **  Abyss Fire Giants
     */
    
    { 25504, BreathWeapon},

    /*
     **  Temple Labrynth
     */

    { 10900, temple_labrynth_liar },
    { 10901, temple_labrynth_liar },
    { 10902, temple_labrynth_sentry},

    /*
     **  Gypsy Village
     */

    { 16106, fido},
    { 16107, CaravanGuildGuard},
    { 16108, CaravanGuildGuard},
    { 16109, CaravanGuildGuard},
    { 16110, CaravanGuildGuard},
    { 16111, WarriorGuildMaster},
    { 16112, MageGuildMaster},
    { 16113, ThiefGuildMaster},
    { 16114, ClericGuildMaster},
    { 16122, receptionist},
    { 16105, StatTeller},

    /*
     **  Draagdim
     */

    { 2500, PrisonGuard },  /* jailer */
    
    /*
     **  mordilnia
     */
    
    {18205, receptionist},
    {18206, MageGuildMaster},
    {18207, ClericGuildMaster},    
    {18208, ThiefGuildMaster},
    {18209, WarriorGuildMaster},    
    {18210, MordGuildGuard},  /*18266 3*/  
    {18211, MordGuildGuard},  /*18276 1*/
    {18212, MordGuildGuard},  /*18272 0*/
    {18213, MordGuildGuard},  /*18256 2*/
    {18215, MordGuard },    
    {18216, janitor},
    {18217, fido},    
    {18222, MordGuard},
    {18223, MordGuard},    

    /*
     **  Graecia:
     */

    {13726,LightningBreather},
    {13732, snake},
    {13765, acid_monster},
    
    /*
     **  Eastern Path
     */
    
    {16020, snake },
    {16037, DwarvenMiners },
    {16039, Tyrannosaurus_swallower},

    /*
     **  undercaves.. level 1
     */
    
    {16213, acid_monster},
    {16219, death_knight},

    /*
     ** Sauria
     */
    
    {21803, Tyrannosaurus_swallower},
    {21810, Tyrannosaurus_swallower},

    /*
     **  Bay Isle
     */
    
    {16610, Demon},
    {16620, BreathWeapon},

    /*
     **  Kings Mountain
     */
    
    {16700, BreathWeapon},
    {16702, shadow},
    {16709, vampire},
    {16710, Devil},
    {16711, Devil},
    {16712, Devil},
    {16713, ghoul},
    {16714, ghoul},
    {16715, wraith},
    {16720, Devil},
    {16721, Devil},
    {16724, Devil},
    {16727, Devil},
    {16728, Devil},
    {16730, Devil},
    {16731, Devil},
    {16732, Demon},
    {16733, Demon},
    {16734, Demon},
    {16735, Demon},
    {16738, BreathWeapon},

    /*
     **  Sewer Rats
     */
    
    {7002, attack_rats},
    {2531, DragonHunterLeader},
    {3063, HuntingMercenary},

    /*
     **  Mages Tower
     */
    
    { 1500, shadow },
    { 1507, Ezmerelda },
      
    
    /*
     ** Mobs vari 
     */

    { 12010, FireBreather    }, 
    { 12011, FireBreather    },

    { 6241, TreeThrowerMob   },
    { 1943, mage_specialist_guildmaster }, /* specialist gm */
    { 1900, avatar_celestian },     
   
    { 1906, village_woman      },
    { 1908, village_woman      },
    { 1920, snake              },
    { 1921, Lizardman          },
    { 1922, lizardman_shaman   },
    { 1923, snake              },
    { 1924, green_slime        },        /* green slime */
    { 1925, snake              },
    { 1926, snake              },
    { 1947, snake_avt          },        /* snake god avatar  */
    { 1953, snake_avt2         },
    { 1948, snake_guardian     },        /* snake guard       */
    { 1949, snake              },        /* pet snake         */
    { 1950, virgin_sac         },        /* virgin sac person */
    { 1951, snake              },        /* flying snake      */
  
    { 21366, PsiGuildmaster    },
    { 21367, RangerGuildmaster },
  
    /* 
     * Spider Haunt
     */
    {9601,goblin_sentry},
    {9602,goblin_sentry},
    {9611, snake} , /* bitch of a spider in the woods */
  

    /*
     * Blackmouths Tower
     */

    { 16738, FireBreather},
    { 5209,  raven_iron_golem},
    { 24000, Pungiglione },
        
    { 25012, fighter_mage },
        
    /*
     ** Ravenloft Area
     */ 


    { 30000, strahd_zombie  }, 
    { 30113, strahd_vampire },
    { 30004, banshee        }, /* BANSHEE */
    { 30102, vampire        }, /* helga */
    { 30103, wraith         }, /* wraith */
    { 30105, mad_gertruda   }, /* gertruda */
    { 30107, raven_iron_golem }, /* iron golem */
    { 30108, mad_cyrus        }, /* cyrus belview */
    { 30106, wraith                }, /* familiar cats */
    { 30112, Demon          }, /* shadow demon */
    { 30114, wraith                }, /* elf corpse */
    { 30115, ghoul                }, /* roo corpse */
    { 30116, vampire        }, /* spectre */
    { 30117, ghost                }, /* ghost */
    { 30118, shadow                }, /* lost souls */
  
    /*
     ***         Ators Mobs
     */        

    {3402,ghoul},
    {3404,wraith},
    {3408,ghost},
    {3409,ghoul},
    {3410,FireBreather},
    {3411,lich_church},         
    {3412,medusa},
    {3415,FireBreather},
    {3416,vampire},
    {3417,regenerator},
    {3424,Slavalis},        

    /*
     **  Forest of Rhowyn
     */

    {13901, ThrowerMob },

    /*
     ** Dwarf Village
     */
    {6502, wraith},
    {6517, snake},

    /*
     **  Alma Civitas 
     */

    { 3005, receptionist },
    { 3007, sailor },    /* Sailor */
    { 3008, PostMaster },            /* PostMaster */
    { 3020, MageGuildMaster }, 
    { 3021, ClericGuildMaster }, 
    { 3022, ThiefGuildMaster }, 
    { 3023, WarriorGuildMaster },
    { 3024, guild_guard }, 
    { 3025, guild_guard }, 
    { 3026, guild_guard },
    { 3027, guild_guard },
    { 3047, DogCatcher }, /* Seneca */
    { 3060, MidgaardCityguard }, 
    { 3061, janitor },
    { 3062, fido }, 
    { 3066, fido },
    { 3067, MidgaardCityguard }, 
    { 3068, ninja_master },
    { 3069, MidgaardCityguard },        /* post guard */
    { 3070, RepairGuy }, 
    { 3071, RepairGuy },
    { 3073, loremaster },
    { 3074, hunter },
    { 3076, archer_instructor },
    { 3077, barbarian_guildmaster},

    { 3143, mayor },
    { 7009, MidgaardCityguard },

    /*
     **   Hammors Stuff
     */
#if 0
    { 3900, eric_johnson }, 
    { 3901, andy_wilcox }, 
#endif

    { 3950, zombie_master },
    { 3952, BreathWeapon },

    /* 
     **  MORIA 
     */
    { 4000, snake }, 
    { 4001, snake }, 
    { 4053, snake },

    { 4101, regenerator },
    { 4102, snake },

    /*
     **  Pyramid
     */

    { 5308, RustMonster },
    { 5303, vampire },

    /*
     **  Arctica
     */
    { 6801, BreathWeapon },
    { 6802, BreathWeapon },
    { 6821, snake },
    { 6824, BreathWeapon },

    /* 
     ** SEWERS 
     */
    { 7006, snake },
    { 7008, snake },
    { 7040, BreathWeapon },     /* Red    */

    /* 
     ** FOREST 
     */

    { 6113, snake },
    { 6114, snake },
    { 6112, BreathWeapon }, /* green */

    /*
     **  Great Eastern Desert
     */
    { 5002, snake },        /* coral snake */
    { 5003, Pungiglione },        /* scorpion    */
    { 5004, acid_monster},/* purple worm  */
    { 5005, BreathWeapon }, /* brass */

    /*
     **  Drow (edition 1)
     */
    { 5101, Drow},
    { 5102, Drow},
    { 5105, Drow},
    { 5106, Drow},

    /*
     **   Thalos
     */
    { 5200, Beholder },        /* beholder    */

    /*
     **  Zoo
     */
    { 9021, SputoVelenoso },        /* Gila Monster */

    /*
     **  Castle Python
     */
    { 11016, receptionist },
    { 11017, NudgeNudge },

    /*
     **  miscellaneous
     */
    { 9061, vampire},        /* vampiress  */

    /*
     **  White Plume Mountain
     */

    { 17014, ghoul },        /* ghoul  */
    { 17009, geyser },        /* geyser  */
    { 17011, vampire },        /* vampire Amelia  */
    { 17002, wraith },        /* wight*/
    { 17005, shadow },        /* shadow */
    { 17010, green_slime }, /* green slime */

    /*
     **  Arachnos
     */

    { 20001, snake },        /* Young (large) spider */
    { 20003, snake },        /* wolf (giant) spider  */
    { 20005, Pungiglione },  /* queen wasp      */
    { 20006, snake },        /* drone spider    */
    { 20010, snake },        /* bird spider     */

    { 20002, BreathWeapon }, /* Yevaud */
    { 20017, BreathWeapon }, /* Elder  */
    { 20016, BreathWeapon }, /* Baby   */


    /*
     **  Elf area
     */
    { 22605, timnus },         /* timnus */
    { 22604, baby_bear},       /* mother bear */
    { 22624, baby_bear},       /* baby bears. */

    /*
     **   Abbarach
     */

    { 27006, Tytan },
    { 27007, replicant },
    { 27016, AbbarachDragon },
    { 27025, Samah}, 

    /* roo/Land down under */

    { 27429, AGGRESSIVE },
    { 27430, AGGRESSIVE },

    { 7526, winger},

    /* Legionari */
    { 12014, LegionariV },
    { 12015, LegionariV },
    { 12016, LegionariV },
    { 12017, LegionariV },
    { 12020, LegionariV },
    { 12021, LegionariV },
    { 12038, LegionariV },
    { 12045, LegionariV },

    { 50062, PrimoAlbero }, /* Il primo Albero */

    { 50102, receptionist }, /* receptionist degli Dei */
    
    { 60010, receptionist },
    { 60011, receptionist },
    { 60012, receptionist },
    { 60013, receptionist },
    { 60014, receptionist },
    { 60015, receptionist },

    { -1, NULL },
  };

  int        i, rnum;

  for (i=0; specials[i].vnum>=0; i++) 
  {
    rnum = real_mobile(specials[i].vnum);
    if (rnum<0) 
    {
      mudlog( LOG_ERROR, "mobile_assign: Mobile %d not found in database.",
              specials[i].vnum);
    } else {
      mob_index[rnum].func = CASTVF specials[i].proc;
    }
  }

  boot_the_shops();
  assign_the_shopkeepers();
}



/* assign special procedures to objects */
void assign_objects()
{
  obj_index[ real_object(    15 ) ].func = CASTVF SlotMachine;
  obj_index[ real_object(    30 ) ].func = CASTVF scraps;
  obj_index[ real_object(    23 ) ].func = CASTVF jive_box;
  obj_index[ real_object(    31 ) ].func = CASTVF portal;
  obj_index[ real_object(  1602 ) ].func = CASTVF Bilancia;
  obj_index[ real_object(  3097 ) ].func = CASTVF board;
  obj_index[ real_object(  3098 ) ].func = CASTVF board;
  obj_index[ real_object(  3099 ) ].func = CASTVF board;
  obj_index[ real_object( 20023 ) ].func = CASTVF board;
  obj_index[ real_object( 20203 ) ].func = CASTVF board;
  obj_index[ real_object( 20403 ) ].func = CASTVF board;
  obj_index[ real_object( 25015 ) ].func = CASTVF BerserkerItem;
  /* obj_index[ real_object( 25102 ) ].func = CASTVF board; Non attiva */
  obj_index[ real_object( 21122 ) ].func = CASTVF nodrop;
  obj_index[ real_object( 21130 ) ].func = CASTVF soap;
  obj_index[ real_object( 27038 ) ].func = CASTVF BerserkerItem;
  
#if EGO
  obj_index[ real_object( 35000 ) ].func = CASTVF EvilBlade;
  obj_index[ real_object( 35001 ) ].func = CASTVF GoodBlade;
  /*obj_index[real_object( 35002 ) ].func = CASTVF NeutralBlade;*/
        
#endif


}


/* assign special procedures to rooms */
void assign_rooms()
{
  int BlockWay( struct char_data *pChar, int nCmd, char *szArg,
                struct room_data *pRoom, int nType );

  static struct RoomSpecialProcEntry specials[] = 
  {
    {    99, Donation },
    {   500, druid_challenge_prep_room },
    {   501, druid_challenge_room },
    {   550, monk_challenge_prep_room },
    {   551, monk_challenge_room },
    {  3030, dump },
    {  3031, pet_shops },
#if 0
    {  4210, BlockWay },
#endif
    { 13547, dump },
    {  3054, pray_for_items },
    {  1911, CapannaVillaggio },
    {  1950, Rampicante },
    {  1951, Rampicante },
    {  1961, ColloSerpente },
#if 0
    {  2188, Magic_Fountain},
    {  2189, Magic_Fountain},
#endif
    {   721, Fountain },
    {  3005, Fountain }, /* Meeting SQ */
    { 13518, Fountain },
    { 11014, Fountain },
    {  5234, Fountain },
    {  3141, Fountain },
    { 13406, Fountain },
    { 22642, Fountain },
    { 22644, Fountain },
    { 22646, Fountain },
    { 22648, Fountain },
    /*{ 26010,        Fountain},*/
    { 13530, pet_shops },

    {  3059, bank },
    { 13521, bank },

#if 0
    {   250, House },
    {   715, House },
    {   716, House },
    {   718, House },
    {  3099, House },  /* Franz's house   */
    {  3143, House },  /* justine's house */
    {  3144, House },  /* zip's house     */
    {  3145, House },  /* crimson's house */
    {  3146, House },  /* hugh's house    */
    {  3147, House },  /* crimson's house */
    {  3148, House },  /* hugh's house    */
    {  3152, House },  /* Kojiro's house    */
    {  3154, House },  /* Magnus's house    */
    {  3155, House },  /* Rincewinds's house    */
    {  3156, House },  /* Comptons's house    */
    {  3158, House },
    {  3159, House },
    {  3160, House },
    {  3161, House }, /* rodgrim  */
    {  3162, House }, /* riffraff */
    {  3163, House }, /* fanchon  */
    { 13495, House }, /* conner */
    { 13496, House }, /* conner */
    { 13497, House }, /* conner */
    { 13498, House }, /* conner */
    {  3827, House },
    {  5198, House }, /*  Rambozo */
    {  5199, House }, /*  shadowspawns */
    {  5700, House }, /*  Dalamar */
    {  5896, House }, /*  Dalamar */
    {  6697, House },
    {  6698, House },
    {  6699, House },
    {  7697, House },
    {  7698, House },
    {  9260, House },
    {  9261, House },
    {  9262, House },
    {  9265, House },
    {  9266, House },
    { 13100, House },  /* SEDUCTIoN */
    { 13729, House },
    { 13730, House },
    { 13799, House },  /* Glopglyph's house   */
    { 17995, House },
    { 17998, House },
    { 17990, House },  /* Turnip */
    { 10100, House },  /* Darrel's house      */
    { 20200, House },  /* Bag's house         */
    { 21336, House }, /* House for Blah */
    { 21337, House },  /* House for Insane */
    { 24000, House },  /* Pat's House         */
    { 27081, House },
#endif
    {  3422, ChurchBell },

    { -1, NULL},
  };
  int i;
  struct room_data *rp;
  
  for (i=0; specials[i].vnum>=0; i++) 
  {
    rp = real_roomp(specials[i].vnum);
    if (rp==NULL)
    {
      mudlog( LOG_ERROR, "assign_rooms: Room %d not found in database.",
              specials[i].vnum);
    } 
    else
      rp->funct = specials[i].proc;
  }
}
