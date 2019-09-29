#include "structs.h"
#include "comm.h"
#include "magic_weapon.h"
#include "utils.h"
#include "spells.h"

void magic_weapon_hit( struct char_data *ch, struct char_data *victim,
                       struct obj_data *weapon )
{
    int d;

    d = (weapon->obj_flags.gpd==1)? 1: number(1,weapon->obj_flags.gpd);
    if(d==1){
        switch(weapon->obj_flags.value[0]){
        case WEAPON_SMASH: sweapon_smash(ch,victim,weapon); break;
        case WEAPON_FLAME: sweapon_flame(ch,victim,weapon); break;
        case WEAPON_ICE:   sweapon_ice  (ch,victim,weapon); break;
        case WEAPON_BOMBARD:  sweapon_bombard(ch,victim,weapon); break;
        case WEAPON_SHOT:  sweapon_shot (ch,victim,weapon);break;
        case WEAPON_DRAGON_SLAYER: sweapon_dragon_slayer(ch,victim,weapon); break;
        case WEAPON_ANTI_GOOD_WEAPON: sweapon_anti_good(ch,victim,weapon); break;
        case WEAPON_ANTI_EVIL_WEAPON: sweapon_anti_evil(ch,victim,weapon);
            break;
        case WEAPON_ALL_HEAL: sweapon_all_heal(ch, victim, weapon); break;
		case WEAPON_VORPAL_BLADE: sweapon_vorpal_blade(ch,victim,weapon); break;
        default: return;  
        }
    }
    else return;
}

void sweapon_vorpal_blade(ch,victim,weapon)
    struct char_data *ch, *victim;
	struct obj_data *weapon;
{
	int dam = 0;
	int extra_dam = 0;

	if (GET_LEVEL(ch) < IMO - 1) {
		// 단군이 아니라면 
		//
		act("$p 에서 엄청난 기운이 흘러나옵니다!.",TRUE,victim,weapon,0,TO_ROOM);
		act("당신은 감히 감당할 수가 없군요!.",TRUE,ch,weapon,0,TO_CHAR);

		spell_curse(GET_LEVEL(ch), ch, ch, 0);
		spell_blindness(GET_LEVEL(ch),ch,ch,0);	
		spell_blindness(GET_LEVEL(ch),ch,ch,0);	

		act("You stop using $p.", FALSE,ch,ch->equipment[WIELD],0,TO_CHAR);
		act("$n stops using $p.", TRUE,ch,ch->equipment[WIELD],0,TO_ROOM);

		obj_to_char(unequip_char(ch, WIELD), ch);

		return;
	}

	if (number(1,60) == 10) {
		act("$p emits a humming sound!.",TRUE,victim,weapon,0,TO_ROOM);
		act("$p drains $n's energy!!!",FALSE,ch,weapon,0,TO_ROOM);
		act("Your energy is drained from $p.", FALSE,ch,weapon,0,TO_CHAR);

		if (!IS_NPC(ch)) GET_HIT(ch) /= 2;

		act("$p concentrates the force!!!",TRUE,victim,weapon,0,TO_ROOM);
        act("$p glows with bright aura.",TRUE,victim,weapon,0,TO_ROOM);

		act("\n*** LIGHTNING!! ***\n",TRUE,victim,weapon,0,TO_ROOM);
		act("Light Solar  guooooooo!!", TRUE,victim,weapon,0,TO_CHAR);

		extra_dam = dice(30,100);
		if (GET_HIT(victim) < 6000 + extra_dam) 
			dam = number(GET_HIT(victim),6000 + extra_dam);
		else dam = 6000 + extra_dam;

		if (!IS_NPC(victim)) dam = GET_HIT(victim);

		damage(ch, victim, dam, SPELL_FULL_FIRE);

		if (IS_NPC(victim)) {
			act("\n$p slowly fade out of existence.\n",
				TRUE,victim,weapon,0,TO_ROOM);

			extract_obj(unequip_char(ch, WIELD));
		}
	}
	else if (number(1,20) == 1) {
		// 평상시엔 special
		dam=dice(GET_LEVEL(ch)/4,(GET_LEVEL(ch)+GET_LEVEL(victim))/2);

        act("$p glows with bright aura.",TRUE,victim,weapon,0,TO_ROOM);
        act("You feel sudden brightness. It really did hurt."
            ,TRUE,victim,weapon,0,TO_CHAR);
        damage(ch,victim,dam,TYPE_UNDEFINED);
	}
	else 
	{
		// 평상시엔?
		dam=number(GET_LEVEL(ch),GET_LEVEL(ch)+GET_LEVEL(victim));

        act("$p glows with bright aura.",TRUE,victim,weapon,0,TO_ROOM);
        act("You feel sudden brightness. It really did hurt."
            ,TRUE,victim,weapon,0,TO_CHAR);
        damage(ch,victim,dam,TYPE_UNDEFINED);
	}

	return;
}

void sweapon_all_heal(ch,victim,weapon)
    struct char_data *ch, *victim;
    struct obj_data *weapon;
{
    if (GET_LEVEL(ch) < IMO -1 ) {
        // if player is not dangun.

		act("$p glows with unholy aura.",TRUE,victim,weapon,0,TO_ROOM);
		switch (number(1,5)) {
		case 1:
		case 2:
        	spell_curse(GET_LEVEL(ch), ch, ch, 0);
			break;
		case 3:
		case 4:
			spell_blindness(GET_LEVEL(ch),ch,ch,0);	
			break;
		case 5:
			mob_teleport(ch, GET_LEVEL(ch));
			break;
		}
    }
    else {
		if (number(1,10) < 8) {
			act("$p glows with holy aura.",TRUE,victim,weapon,0,TO_ROOM);
			act("      +++++++++      ",TRUE,victim,weapon,0,TO_ROOM);
			spell_all_heal(GET_LEVEL(ch), ch, 0, 0);
		}
    }

    return;
}


void sweapon_smash(ch,victim,weapon)
    struct char_data *ch, *victim;
    struct obj_data *weapon;
{
    int dam=91;

    dam = dam - GET_INT(ch)*2 - GET_WIS(ch)*2;
    act("$p smashes $n with bone crushing sound.",TRUE,victim,weapon,0,TO_ROOM);
    act("You are smashed by $p. You are hard to stand.",TRUE,victim,weapon,0,TO_CHAR);
    damage(ch,victim,dam,TYPE_UNDEFINED);
    if(GET_POS(victim)>POSITION_SITTING) GET_POS(victim)=POSITION_SITTING;
}
 
void sweapon_flame(ch,victim,weapon)
    struct char_data *ch, *victim;
    struct obj_data *weapon;
{
    int dam;
    dam=number(20,35);
    act("Large flame from $o strikes $n. $n is burned."
        ,TRUE,victim,weapon,0,TO_ROOM);
    act("Large flame from $o strikes you. You are burned."
        ,TRUE,victim,weapon,0,TO_CHAR);
    WAIT_STATE(victim,PULSE_VIOLENCE);
    damage(ch,victim,dam,TYPE_UNDEFINED);
}
 
void sweapon_ice(ch,victim,weapon)
    struct char_data *ch, *victim;
    struct obj_data *weapon;
{
    int dam;
    dam=number(10,45);
    act("White beam from $o strikes $n. $n is frozen."
        ,TRUE,victim,weapon,0,TO_ROOM);
    act("White beam from $o strikes you. You are frozen."
        ,TRUE,victim,weapon,0,TO_CHAR);
    WAIT_STATE(victim,PULSE_VIOLENCE);
    damage(ch,victim,dam,TYPE_UNDEFINED);
}

void sweapon_bombard(ch,victim,weapon)
    struct char_data *ch, *victim;
    struct obj_data *weapon;
{
    int dam;
    dam=number(GET_LEVEL(ch),GET_LEVEL(ch)+GET_LEVEL(victim));
    act("   *** Bombard ***"
        ,TRUE,ch,weapon,0,TO_ROOM);
    WAIT_STATE(victim,PULSE_VIOLENCE);
    damage(ch,victim,dam,TYPE_UNDEFINED);
    if(GET_POS(victim)>POSITION_STUNNED) GET_POS(victim)=POSITION_STUNNED;
}

void sweapon_shot(ch,victim,weapon)
    struct char_data *ch, *victim;
    struct obj_data *weapon;
{
    int dam=50;
    dam=dam - GET_STR(ch) - GET_CON(ch);
    act("$p glows with striking aura.",TRUE,victim,weapon,0,TO_ROOM);
    act("You see $p glows. It HURTS you.",TRUE,victim,weapon,0,TO_CHAR);
    damage(ch,victim,dam,TYPE_UNDEFINED);
}

void sweapon_dragon_slayer(ch,victim,weapon)
    struct char_data *ch, *victim;
    struct obj_data *weapon;
{
    int dam;
    if(IS_NPC(victim) && !IS_SET(victim->specials.act,ACT_DRAGON)) return;
    dam=number(GET_LEVEL(ch)*2,5*GET_LEVEL(ch));
    act("$p PENETRATES the $n's tough skin.",TRUE,victim,weapon,0,TO_ROOM);
    damage(ch,victim,dam,TYPE_UNDEFINED);
}

void sweapon_anti_good(ch,victim,weapon)
    struct char_data *ch, *victim;
    struct obj_data *weapon;
{
    int dam;
    dam=number(GET_LEVEL(ch),GET_LEVEL(ch)+GET_LEVEL(victim));
    if(!IS_EVIL(ch)){
        act("$n screams in pain. $n looks like being shocked by something.",
            TRUE,ch,weapon,0,TO_ROOM);
        act("You feel sudden pain from your hands. You are shocked.",
            TRUE,ch,weapon,0,TO_CHAR);
        dam=MIN(GET_HIT(ch)-1,2*dam);
        damage(victim,ch,dam,TYPE_UNDEFINED);
        return;
    }
    if(IS_GOOD(victim)){
        act("$p glows with dark aura.",TRUE,victim,weapon,0,TO_ROOM);
        act("You feel sudden darkness. It really did hurt."
            ,TRUE,victim,weapon,0,TO_CHAR);
        damage(ch,victim,dam,TYPE_UNDEFINED);
        return;
    }
}

void sweapon_anti_evil(ch,victim,weapon)
    struct char_data *ch, *victim;
    struct obj_data *weapon;
{
    int dam;
    dam=number(GET_LEVEL(ch),GET_LEVEL(ch)+GET_LEVEL(victim));
    if(!IS_GOOD(ch)){
        act("$n screams in pain. $n looks like being shocked by something.",
            TRUE,ch,weapon,0,TO_ROOM);
        act("You feel sudden pain from your hands. You are shocked.",
            TRUE,ch,weapon,0,TO_CHAR);
        dam=MIN(GET_HIT(ch)-1,2*dam);
        damage(victim,ch,dam,TYPE_UNDEFINED);
        return;
    }
    if(IS_EVIL(victim)){
        act("$p glows with bright aura.",TRUE,victim,weapon,0,TO_ROOM);
        act("You feel sudden brightness. It really did hurt."
            ,TRUE,victim,weapon,0,TO_CHAR);
        damage(ch,victim,dam,TYPE_UNDEFINED);
        return;
    }
}

