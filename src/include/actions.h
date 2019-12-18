/****************************************************************
 *  File : actions.h : Command procedure related def.		*
 ****************************************************************/
/*
 * Command Number Definitions
 */

#define CMD_NORTH 		1
#define CMD_EAST 		2
#define CMD_SOUTH 		3
#define CMD_WEST 		4
#define CMD_UP 			5
#define CMD_DOWN 		6
#define CMD_ENTER 		7
#define CMD_EXITS 		8
#define CMD_GET 		10
#define CMD_DRINK 		11
#define CMD_EAT 		12
#define CMD_WEAR 		13
#define CMD_WIELD 		14
#define CMD_LOOK 		15
#define CMD_SCORE 		16
#define CMD_SAY 		17
#define CMD_SHOUT 		18
#define CMD_TELL 		19
#define CMD_INVENTORY 		20
#define CMD_SMILE 		22
#define CMD_KILL 		25
#define CMD_INSULT 		33
#define CMD_HELP 		38
#define CMD_WHO 		39
#define CMD_EMOTE 		40
#define CMD_ECHO 		41
#define CMD_STAND 		42
#define CMD_SIT 		43
#define CMD_REST 		44
#define CMD_SLEEP 		45
#define CMD_WAKE 		46
#define CMD_FORCE 		47
#define CMD_TRANSFER 		48
#define CMD_NEWS 		54
#define CMD_EQUIPMENT 		55
#define CMD_BUY 		56
#define CMD_SELL 		57
#define CMD_VALUE 		58
#define CMD_LIST 		59
#define CMD_DROP 		60
#define CMD_GOTO 		61
#define CMD_WEATHER 		62
#define CMD_READ 		63
#define CMD_POUR 		64
#define CMD_GRAB 		65
#define CMD_REMOVE 		66
#define CMD_PUT 		67
/* #define CMD_SHUTDOW 		68 */
#define CMD_SAVE 		69
#define CMD_HIT 		70
#define CMD_STRING 		71
#define CMD_GIVE 		72
#define CMD_QUIT 		73
#define CMD_STAT 		74
#define CMD_TIME 		76
#define CMD_LOAD 		77
#define CMD_PURGE 		78
#define CMD_SHUTDOWN 		79
#define CMD_REPLACERENT 	82
#define CMD_WHISPER 		83
#define CMD_CAST 		84
#define CMD_AT 			85
#define CMD_ASK 		86
#define CMD_ORDER 		87
#define CMD_SIP 		88
#define CMD_TASTE 		89
#define CMD_SNOOP 		90
#define CMD_FOLLOW 		91
#define CMD_RENT 		92
#define CMD_JUNK 		93
#define CMD_ADVANCE 		95
#define CMD_BOW 		98
#define CMD_OPEN 		99
#define CMD_CLOSE 		100
#define CMD_LOCK 		101
#define CMD_UNLOCK 		102
#define CMD_LEAVE 		103
#define CMD_EXTRACTRENT 	149
#define CMD_HOLD 		150
#define CMD_FLEE 		151
#define CMD_SNEAK 		152
#define CMD_HIDE 		153
#define CMD_BACKSTAB 		154
#define CMD_PICK 		155
#define CMD_STEAL 		156
#define CMD_BASH 		157
#define CMD_RESCUE 		158
#define CMD_KICK 		159
#define CMD_PRACTICE 		164
#define CMD_EXAMINE 		166
#define CMD_TAKE 		167
#define CMD_QUOTE 		169
/* #define CMD_PRACTISE 		170 */
#define CMD_USE 		172
#define CMD_WHERE 		173
#define CMD_LEVELS 		174
#define CMD_REROLL 		175
#define CMD_TILDE 		177
#define CMD_POINT 		187
#define CMD_SHOOT 		192
#define CMD_BRIEF 		199
#define CMD_WIZNET 		200
#define CMD_CONSIDER 		201
#define CMD_GROUP 		202
#define CMD_RESTORE 		203
#define CMD_RETURN 		204
#define CMD_SWITCH 		205
#define CMD_QUAFF 		206
#define CMD_RECITE 		207
#define CMD_USERS 		208
#define CMD_FLAG 		209
#define CMD_NOSHOUT 		210
#define CMD_WIZHELP 		211
#define CMD_CREDITS 		212
#define CMD_COMPACT 		213
#define CMD_FLICK 		214
#define CMD_WALL 		215
#define CMD_WIZSET 		216
#define CMD_POLICE 		217
#define CMD_WIZLOCK 		218
#define CMD_NOAFFECT 		219
#define CMD_INVIS 		220
#define CMD_NOTELL 		221
#define CMD_BANISH 		222
#define CMD_RELOAD 		223
#define CMD_DATA 		224
#define CMD_CHECKRENT 		225
#define CMD_CHAT 		226
#define CMD_BALANCE 		227
#define CMD_DEPOSIT 		228
#define CMD_WITHDRAW 		229
#define CMD_SYS 		230
#define CMD_LOG 		231
#define CMD_MSTAT 		232
#define CMD_PSTAT 		233
#define CMD_TORNADO 		234
#define CMD_LIGHT 		235
#define CMD_TITLE 		236
#define CMD_REPORT 		237
#define CMD_SPELLS 		238
#define CMD_FLASH 		239
#define CMD_MULTI_KICK 		240
#define CMD_DEMOTE 		241
#define CMD_NOCHAT 		242
#define CMD_WIMPY 		243
#define CMD_GTELL 		244
#define CMD_SEND 		245
#define CMD_WRITE 		246
#define CMD_POST 		247
#define CMD_HLOOK 		252
#define CMD_ASSIST 		262
#define CMD_UNGROUP 		263
#define CMD_WIZLIST 		264
#define CMD_HANGUL 		265
/* #define CMD_NEWS 		266 */
#define CMD_SET 		267
#define CMD_DISARM 		268
#define CMD_SHOURYUKEN 		269
#define CMD_THROW 		270
#define CMD_PUNCH 		271
#define CMD_ASSAULT 		272
#define CMD_JOIN 		273
/* #define CMD_LEAVE 		274 */
#define CMD_TRAIN 		275
#define CMD_CANT 		276
/* #define CMD_SAVE 		277 */
/* #define CMD_LOAD 		278 */
#define CMD_QUERY 		279
#define CMD_BROADCAST 		280
#define CMD_SIMULTANEOUS 	281
#define CMD_ARREST 		282
#define CMD_ANGRY_YELL 		283
#define CMD_SOLACE 		284
#define CMD_UNWIELD 		285
#define CMD_UNHOLD 		286
#define CMD_TEMPTATION 		287
#define CMD_SHADOW_FIGURE 	288
#define CMD_SMOKE 		289
#define CMD_INJECT 		290
#define CMD_SHOW 		291
#define CMD_POWER_BASH 		292
#define CMD_EVIL_STRIKE 	293
#define CMD_TAXI 		294
#define CMD_CHARGE 		295
#define CMD_SOLO 		296
#define CMD_DUMB 		297
#define CMD_SPIN_BIRD_KICK 	298
#define CMD_VIEW 		299
#define CMD_REPLY 		300
#define CMD_QUEST 		302
#define CMD_REQUEST 		303
#define CMD_HINT 		304
#define CMD_PULL 		305
#define CMD_CHANGE 		306
#define CMD_COLON 		307
#define CMD_LASTCHAT 		308

/* NOTE: NEW! 'house' and 'hcontrol' command */
#define CMD_HOUSE 		309
#define CMD_HCONTROL 		310
/* NOTE: NEW! 'track' command */
#define CMD_TRACK 		311

/*
 *  File : actions.h :  ALL command handler do_*'s             *
 */

extern void do_shoot(struct char_data *ch, char *argument, int cmd);
extern void do_wimpy(struct char_data *ch, char *argument, int cmd);
extern void do_move(struct char_data *ch, char *argument, int cmd);
extern void do_look(struct char_data *ch, char *argument, int cmd);
extern void do_read(struct char_data *ch, char *argument, int cmd);
extern void do_write(struct char_data *ch, char *argument, int cmd);
extern void do_say(struct char_data *ch, char *argument, int cmd);
extern void do_exit(struct char_data *ch, char *argument, int cmd);
extern void do_snoop(struct char_data *ch, char *argument, int cmd);
extern void do_insult(struct char_data *ch, char *argument, int cmd);
extern void do_quit(struct char_data *ch, char *argument, int cmd);
extern void do_help(struct char_data *ch, char *argument, int cmd);
extern void do_who(struct char_data *ch, char *argument, int cmd);
extern void do_emote(struct char_data *ch, char *argument, int cmd);
extern void do_echo(struct char_data *ch, char *argument, int cmd);
extern void do_trans(struct char_data *ch, char *argument, int cmd);
extern void do_kill(struct char_data *ch, char *argument, int cmd);
extern void do_stand(struct char_data *ch, char *argument, int cmd);
extern void do_sit(struct char_data *ch, char *argument, int cmd);
extern void do_rest(struct char_data *ch, char *argument, int cmd);
extern void do_sleep(struct char_data *ch, char *argument, int cmd);
extern void do_wake(struct char_data *ch, char *argument, int cmd);
extern void do_force(struct char_data *ch, char *argument, int cmd);
extern void do_get(struct char_data *ch, char *argument, int cmd);
extern void do_drop(struct char_data *ch, char *argument, int cmd);
/*  extern void do_news(struct char_data *ch, char *argument, int cmd);  */
extern void do_score(struct char_data *ch, char *argument, int cmd);
/* extern void do_title(struct char_data *ch, char *argument, int cmd); */
extern void do_spells(struct char_data *ch, char *argument, int cmd);
extern void do_report(struct char_data *ch, char *argument, int cmd);
extern void do_inventory(struct char_data *ch, char *argument, int cmd);
extern void do_equipment(struct char_data *ch, char *argument, int cmd);
extern void do_shout(struct char_data *ch, char *argument, int cmd);
extern void do_not_here(struct char_data *ch, char *argument, int cmd);
extern void do_tell(struct char_data *ch, char *argument, int cmd);
extern void do_send(struct char_data *ch, char *argument, int cmd);
extern void do_gtell(struct char_data *ch, char *argument, int cmd);
extern void do_wear(struct char_data *ch, char *argument, int cmd);
extern void do_wield(struct char_data *ch, char *argument, int cmd);
extern void do_grab(struct char_data *ch, char *argument, int cmd);
extern void do_remove(struct char_data *ch, char *argument, int cmd);
extern void do_put(struct char_data *ch, char *argument, int cmd);
extern void do_shutdown(struct char_data *ch, char *argument, int cmd);
extern void do_save(struct char_data *ch, char *argument, int cmd);
extern void do_hit(struct char_data *ch, char *argument, int cmd);
extern void do_string(struct char_data *ch, char *arg, int cmd);
extern void do_give(struct char_data *ch, char *arg, int cmd);
extern void do_stat(struct char_data *ch, char *arg, int cmd);
extern void do_time(struct char_data *ch, char *arg, int cmd);
extern void do_weather(struct char_data *ch, char *arg, int cmd);
extern void do_load(struct char_data *ch, char *arg, int cmd);
extern void do_purge(struct char_data *ch, char *arg, int cmd);
/* extern void do_shutdow(struct char_data *ch, char *arg, int cmd); */
extern void do_whisper(struct char_data *ch, char *arg, int cmd);
extern void do_cast(struct char_data *ch, char *arg, int cmd);
extern void do_at(struct char_data *ch, char *arg, int cmd);
extern void do_goto(struct char_data *ch, char *arg, int cmd);
/* NOTE: extern void do_ask(struct char_data *ch, char *arg, int cmd); */
extern void do_drink(struct char_data *ch, char *arg, int cmd);
extern void do_eat(struct char_data *ch, char *arg, int cmd);
extern void do_pour(struct char_data *ch, char *arg, int cmd);
/* NOTE: do_sip(), do_taste() are replaced by do_drink(), do_eat() */
/* extern void do_sip(struct char_data *ch, char *arg, int cmd); */
/* extern void do_taste(struct char_data *ch, char *arg, int cmd); */
extern void do_order(struct char_data *ch, char *arg, int cmd);
extern void do_follow(struct char_data *ch, char *arg, int cmd);
extern void do_rent(struct char_data *ch, char *arg, int cmd);
extern void do_junk(struct char_data *ch, char *arg, int cmd);
extern void do_advance(struct char_data *ch, char *arg, int cmd);
extern void do_close(struct char_data *ch, char *arg, int cmd);
extern void do_open(struct char_data *ch, char *arg, int cmd);
extern void do_lock(struct char_data *ch, char *arg, int cmd);
extern void do_unlock(struct char_data *ch, char *arg, int cmd);
extern void do_exits(struct char_data *ch, char *arg, int cmd);
extern void do_enter(struct char_data *ch, char *arg, int cmd);
extern void do_leave(struct char_data *ch, char *arg, int cmd);
extern void do_flee(struct char_data *ch, char *arg, int cmd);
extern void do_sneak(struct char_data *ch, char *arg, int cmd);
extern void do_hide(struct char_data *ch, char *arg, int cmd);
extern void do_backstab(struct char_data *ch, char *arg, int cmd);
extern void do_pick(struct char_data *ch, char *arg, int cmd);
extern void do_steal(struct char_data *ch, char *arg, int cmd);
extern void do_bash(struct char_data *ch, char *arg, int cmd);
extern void do_rescue(struct char_data *ch, char *arg, int cmd);
extern void do_kick(struct char_data *ch, char *arg, int cmd);
extern void do_punch(struct char_data *ch, char *arg, int cmd);
extern void do_examine(struct char_data *ch, char *arg, int cmd);
extern void do_users(struct char_data *ch, char *arg, int cmd);
extern void do_where(struct char_data *ch, char *arg, int cmd);
extern void do_levels(struct char_data *ch, char *arg, int cmd);
extern void do_reroll(struct char_data *ch, char *arg, int cmd);
/* extern void do_brief(struct char_data *ch, char *arg, int cmd); */
extern void do_wiznet(struct char_data *ch, char *arg, int cmd);
extern void do_consider(struct char_data *ch, char *arg, int cmd);
extern void do_group(struct char_data *ch, char *arg, int cmd);
extern void do_restore(struct char_data *ch, char *arg, int cmd);
extern void do_return(struct char_data *ch, char *argument, int cmd);
extern void do_switch(struct char_data *ch, char *argument, int cmd);
extern void do_quaff(struct char_data *ch, char *argument, int cmd);
extern void do_recite(struct char_data *ch, char *argument, int cmd);
extern void do_use(struct char_data *ch, char *argument, int cmd);
extern void do_flag(struct char_data *ch, char *argument, int cmd);
extern void do_noshout(struct char_data *ch, char *argument, int cmd);
extern void do_wizhelp(struct char_data *ch, char *argument, int cmd);
/*  extern void do_credits(struct char_data *ch, char *argument, int cmd);  */
/* extern void do_compact(struct char_data *ch, char *argument, int cmd); */
extern void do_social(struct char_data *ch, char *arg, int cmd);
extern void do_practice(struct char_data *ch, char *arg, int cmd);
extern void do_flick(struct char_data *ch, char *arg, int cmd);
extern void do_wall(struct char_data *ch, char *arg, int cmd);
extern void do_wizset(struct char_data *ch, char *arg, int cmd);
extern void do_police(struct char_data *ch, char *arg, int cmd);
extern void do_wizlock(struct char_data *ch, char *arg, int cmd);
extern void do_noaffect(struct char_data *ch, char *arg, int cmd);
extern void do_invis(struct char_data *ch, char *arg, int cmd);
/* extern void do_notell(struct char_data *ch, char *arg, int cmd); */
extern void do_banish(struct char_data *ch, char *arg, int cmd);
extern void do_reload(struct char_data *ch, char *arg, int cmd);
extern void do_data(struct char_data *ch, char *arg, int cmd);
extern void do_checkrent(struct char_data *ch, char *arg, int cmd);
extern void do_chat(struct char_data *ch, char *arg, int cmd);
extern void do_lastchat(struct char_data *ch, char *arg, int cmd);
extern void do_bank(struct char_data *ch, char *arg, int cmd);
extern void do_sys(struct char_data *ch, char *arg, int cmd);
extern void do_extractrent(struct char_data *ch, char *arg, int cmd);
extern void do_replacerent(struct char_data *ch, char *arg, int cmd);
extern void do_tornado(struct char_data *ch, char *arg, int cmd);
extern void do_light_move(struct char_data *ch, char *arg, int cmd);
extern void do_flash(struct char_data *ch, char *arg, int cmd);
extern void do_multi_kick(struct char_data *ch, char *arg, int cmd);
extern void do_demote(struct char_data *ch, char *arg, int cmd);
/* extern void do_nochat(struct char_data *ch, char *arg, int cmd); */
extern void do_post(struct char_data *ch, char *arg, int cmd);
/* extern void do_sayh(struct char_data *ch, char *argument, int cmd); */
extern void do_assist(struct char_data *ch, char *argument, int cmd);
extern void do_ungroup(struct char_data *ch, char *argument, int cmd);
/* extern void do_wizards(struct char_data *ch, char *argument, int cmd);  */
/* extern void do_hangul(struct char_data *ch, char *arg, int cmd); */
/*  extern void do_version(struct char_data *ch, char *arg, int cmd);  */
extern void do_set(struct char_data *ch, char *arg, int cmd);
extern void do_disarm(struct char_data *ch, char *arg, int cmd);	/* chase */
extern void do_shouryuken(struct char_data *ch, char *arg, int cmd);	/* chase */
extern void do_throw_object(struct char_data *ch, char *arg, int cmd);	/* chase */
extern void do_assault(struct char_data *ch, char *arg, int cmd);    /* by process */
extern void do_cant(struct char_data *ch, char *arg, int cmd);	      /* by process */
extern void do_query(struct char_data *ch, char *arg, int cmd);      /* by process */
extern void do_whistle(struct char_data *ch, char *arg, int cmd);	/* by process */
extern void do_simultaneous(struct char_data *ch, char *arg, int cmd);	/* by process */
extern void do_arrest(struct char_data *ch, char *arg, int cmd);     /* by process */
extern void do_angry_yell(struct char_data *ch, char *arg, int cmd); /* by process */
extern void do_solace(struct char_data *ch, char *arg, int cmd);     /* by process */
/* extern void do_unwield(struct char_data *ch, char *arg, int cmd); */    /* by process */
/* extern void do_unhold(struct char_data *ch, char *arg, int cmd); */     /* by process */
extern void do_temptation(struct char_data *ch, char *arg, int cmd); /* by process */
extern void do_shadow(struct char_data *ch, char *arg, int cmd);     /* by process */
extern void do_smoke(struct char_data *ch, char *arg, int cmd);      /* by process */
extern void do_inject(struct char_data *ch, char *arg, int cmd);     /* by process */
/* extern void do_plan(struct char_data *ch, char *arg, int cmd); */ /* by process */
extern void do_power_bash(struct char_data *ch, char *arg, int cmd); /* by process */
extern void do_evil_strike(struct char_data *ch, char *arg, int cmd);/* by process */
extern void do_charge(struct char_data *ch, char *arg, int cmd);     /* by process */
/* extern void do_solo(struct char_data *ch, char *arg, int cmd); */	/* by process */
extern void do_auto_assist(struct char_data *ch, char *arg, int cmd);/* by process */
extern void do_spin_bird_kick(struct char_data *ch, char *arg, int cmd);/* by ares */
extern void do_reply(struct char_data *ch, char *arg, int cmd);      /* by process */
/* quest */
extern void do_quest(struct char_data *ch, char *arg, int cmd);	/* by atre */
extern void do_request(struct char_data *ch, char *arg, int cmd);	/* by atre */
extern void do_hint(struct char_data *ch, char *arg, int cmd);		/* by atre */

/* ----------   NEW / MODIFIED commands   -------------- */

/* NOTE: NEW! do_point() in act.special.c		*/
extern void do_point(struct char_data *ch, char *argument, int cmd);
/* NOTE: NEW! do_show() in act.info.c		*/
extern void do_show(struct char_data *ch, char *argument, int cmd);

/* NOTE: NEW! 'house' and 'hcontrol' command */
extern void do_house(struct char_data *ch, char *argument, int cmd);
extern void do_hcontrol(struct char_data *ch, char *argument, int cmd);
/* NOTE: NEW! 'track' command */
extern void do_track(struct char_data *ch, char *argument, int cmd);

/* --------------------------------------------------------------- */
