/****************************************************************************
 * In questo file ci sono i prototipi e le strutture usate per le procedure
 * nel modulo FIGHT
 * *************************************************************************/
#if !defined( _FIGHT_H)
#define _FIGHT_H

enum DamageResult 
{
  AllLiving, SubjectDead, VictimDead
};

void load_messages();
void update_pos( struct char_data *victim );
int check_peaceful(struct char_data *ch, char *msg);
void set_fighting(struct char_data *ch, struct char_data *vict);
void stop_fighting(struct char_data *ch);
void death_cry(struct char_data *ch);
int SetCharFighting(struct char_data *ch, struct char_data *v);
int SetVictFighting(struct char_data *ch, struct char_data *v);
int DamageTrivia(struct char_data *ch, struct char_data *v, int dam, int type);
void make_corpse(struct char_data *ch, int killedbytype);
void raw_kill(struct char_data *ch,int killedbytype);
void die(struct char_data *ch,int killedbytype);
DamageResult MissileDamage( struct char_data *ch, struct char_data *victim,
                            int dam, int attacktype );
DamageResult damage(struct char_data *ch, struct char_data *victim,
                  int dam, int attacktype);
int CalcThaco( struct char_data *ch, struct char_data *victim = NULL );
int HitOrMiss(struct char_data *ch, struct char_data *victim, int calc_thaco);
DamageResult hit(struct char_data *ch, struct char_data *victim, int type);
void perform_violence(int pulse);
struct char_data *FindVictim( struct char_data *ch);
struct char_data *FindAnyVictim( struct char_data *ch);
int PreProcDam(struct char_data *ch, int type, int dam);
int DamageOneItem( struct char_data *ch, int dam_type, struct obj_data *obj);
void MakeScrap( struct char_data *ch,struct char_data *v, 
                struct obj_data *obj);
void DamageAllStuff( struct char_data *ch, int dam_type);
int ItemSave( struct obj_data *i, int dam_type);
int WeaponCheck(struct char_data *ch, struct char_data *v, int type, int dam);
void DamageStuff( struct char_data *v, int type, int dam );
int SkipImmortals(struct char_data *v, int amnt, int attacktype);
struct char_data *FindAnAttacker(struct char_data *ch);
void shoot( struct char_data *ch, struct char_data *victim);
struct char_data *FindMetaVictim( struct char_data *ch);
void NailThisSucker( struct char_data *ch);
 
#endif 
