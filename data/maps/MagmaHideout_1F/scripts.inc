MagmaHideout_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, MagmaHideout_1F_OnTransition
	.byte 0

MagmaHideout_1F_OnTransition:
	setvar VAR_JAGGED_PASS_ASH_WEATHER, 0
	call MagmaHideout_1F_EventScript_TryEnableCourtneyBattle
	end

@ Courtney waits just after the entrance until the player beats her
MagmaHideout_1F_EventScript_TryEnableCourtneyBattle::
	goto_if_set FLAG_HIDE_MAGMA_HIDEOUT_GRUNTS, MagmaHideout_1F_EventScript_TryEnableCourtneyBattleEnd
	goto_if_defeated TRAINER_COURTNEY_MAGMA_HIDEOUT, MagmaHideout_1F_EventScript_TryEnableCourtneyBattleEnd
	setvar VAR_TEMP_1, 1
MagmaHideout_1F_EventScript_TryEnableCourtneyBattleEnd:
	return

MagmaHideout_1F_EventScript_Grunt1::
	trainerbattle_single TRAINER_GRUNT_MAGMA_HIDEOUT_1, MagmaHideout_1F_Text_Grunt1Intro, MagmaHideout_1F_Text_Grunt1Defeat
	msgbox MagmaHideout_1F_Text_Grunt1PostBattle, MSGBOX_AUTOCLOSE
	end

MagmaHideout_1F_EventScript_Grunt2::
	trainerbattle_single TRAINER_GRUNT_MAGMA_HIDEOUT_2, MagmaHideout_1F_Text_Grunt2Intro, MagmaHideout_1F_Text_Grunt2Defeat
	msgbox MagmaHideout_1F_Text_Grunt2PostBattle, MSGBOX_AUTOCLOSE
	end

MagmaHideout_1F_Text_Grunt1Intro:
	.string "When TEAM MAGMA has roll call, we get\n"
	.string "important guarding assignments in\l"
	.string "the order that we line up.\p"
	.string "That's why I'm stuck off in this corner.\n"
	.string "I'm always late to roll call!$"

MagmaHideout_1F_Text_Grunt1Defeat:
	.string "I'm always late for training sessions,\n"
	.string "too!\p"
	.string "I hate to say it, but I'm wimpy…$"

MagmaHideout_1F_Text_Grunt1PostBattle:
	.string "Okay, I'll try to put a little more\n"
	.string "effort into things from now on…$"

MagmaHideout_1F_Text_Grunt2Intro:
	.string "Our leader told us to dig into\n"
	.string "MT. CHIMNEY, so we dug and dug.\p"
	.string "And in the course of digging, we came\n"
	.string "across something that blew our minds!\p"
	.string "What did we find?\p"
	.string "Fuhahaha!\n"
	.string "I'll tell you if you beat me!$"

MagmaHideout_1F_Text_Grunt2Defeat:
	.string "Arrgh!\n"
	.string "Taken down!$"

MagmaHideout_1F_Text_Grunt2PostBattle:
	.string "I won't tell you after all.\n"
	.string "You'll find out when you get there!\p"
	.string "It'd be better if you saved surprises\n"
	.string "to the end, don't you think?$"

MagmaHideout_1F_EventScript_CourtneyTrigger::
	lockall
	playse SE_PIN
	applywaitmovement LOCALID_MAGMA_HIDEOUT_COURTNEY, Common_Movement_ExclamationMark
	applywaitmovement LOCALID_PLAYER, Common_Movement_FaceUp
	goto MagmaHideout_1F_EventScript_CourtneyBattle
	end

MagmaHideout_1F_EventScript_Courtney::
	lockall
	faceplayer
	goto_if_defeated TRAINER_COURTNEY_MAGMA_HIDEOUT, MagmaHideout_1F_EventScript_CourtneyDefeated
MagmaHideout_1F_EventScript_CourtneyBattle::
	msgbox MagmaHideout_1F_Text_CourtneyIntro, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_COURTNEY_MAGMA_HIDEOUT, MagmaHideout_1F_Text_CourtneyDefeat
	setvar VAR_TEMP_1, 0
MagmaHideout_1F_EventScript_CourtneyDefeated::
	msgbox MagmaHideout_1F_Text_CourtneyPostBattle, MSGBOX_DEFAULT
	releaseall
	end

MagmaHideout_1F_Text_CourtneyIntro:
	.string "COURTNEY: …Ah. An intruder.\p"
	.string "I am COURTNEY.\n"
	.string "ADMIN of TEAM MAGMA.\p"
	.string "LEADER MAXIE is about to awaken\n"
	.string "GROUDON…\p"
	.string "…Nobody gets past this point.\n"
	.string "I will… analyze you… and erase you.$"

MagmaHideout_1F_Text_CourtneyDefeat:
	.string "…Unexpected.\n"
	.string "My analysis… was wrong.$"

MagmaHideout_1F_Text_CourtneyPostBattle:
	.string "COURTNEY: …Go, then.\p"
	.string "It is already too late.\n"
	.string "LEADER MAXIE cannot be stopped.$"
