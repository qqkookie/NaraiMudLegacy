/*  Define for magic weapon */

#define WEAPON_SMASH 1
#define WEAPON_FLAME 2
#define WEAPON_ICE 3
#define WEAPON_BOMBARD 4
#define WEAPON_SHOT 5
#define WEAPON_DRAGON_SLAYER 6
#define WEAPON_ANTI_EVIL_WEAPON 7
#define WEAPON_ANTI_GOOD_WEAPON 8
#define WEAPON_GIANT_SLAYER 9
#define WEAPON_ALL_HEAL     10
#define WEAPON_VORPAL_BLADE  11

void magic_weapon_hit( struct char_data *ch, struct char_data *victim,
  struct obj_data *weapon );
void sweapon_smash( struct char_data *ch, struct char_data *victim,
  struct obj_data *weapon );
void sweapon_flame( struct char_data *ch, struct char_data *victim,
  struct obj_data *weapon );
void sweapon_ice( struct char_data *ch, struct char_data *victim,
  struct obj_data *weapon );
void sweapon_bombard( struct char_data *ch, struct char_data *victim,
  struct obj_data *weapon );
void sweapon_shot( struct char_data *ch, struct char_data *victim,
  struct obj_data *weapon );
void sweapon_dragon_slayer( struct char_data *ch, struct char_data *victim,
  struct obj_data *weapon );
void sweapon_anti_good( struct char_data *ch, struct char_data *victim,
  struct obj_data *weapon );
void sweapon_anti_evil( struct char_data *ch, struct char_data *victim,
  struct obj_data *weapon );
void sweapon_all_heal( struct char_data *ch, struct char_data *victim,
  struct obj_data *weapon );
void sweapon_vorpal_blade( struct char_data *ch, struct char_data *victim,
  struct obj_data *weapon );
