@ Nature Changer: an NPC found in every Pokemon Center that lets the player pick a party mon's nature for free
Common_EventScript_NatureChanger::
	lock
	faceplayer
	msgbox NatureChanger_Text_Intro, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, NatureChanger_EventScript_ComeBack
	msgbox NatureChanger_Text_WhichMon, MSGBOX_DEFAULT
	special ChoosePartyMon
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, NatureChanger_EventScript_ComeBack
	specialvar VAR_RESULT, ScriptGetPartyMonSpecies
	goto_if_eq VAR_RESULT, SPECIES_EGG, NatureChanger_EventScript_Egg
	copyvar VAR_0x8008, VAR_0x8004
	special BufferNatureChangerMonInfo
	message NatureChanger_Text_WhichNature
	waitmessage
	setvar VAR_0x8004, SCROLL_MULTI_NATURES
	special ShowScrollableMultichoice
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, NatureChanger_EventScript_ComeBack
	goto_if_eq VAR_RESULT, NATURE_CHANGER_NUM_OPTIONS, NatureChanger_EventScript_ComeBack
	special ChangeMonNature
	playse SE_M_PSYBEAM
	waitse
	msgbox NatureChanger_Text_Done, MSGBOX_DEFAULT
	release
	end

NatureChanger_EventScript_Egg::
	msgbox NatureChanger_Text_Egg, MSGBOX_DEFAULT
	release
	end

NatureChanger_EventScript_ComeBack::
	msgbox NatureChanger_Text_ComeBack, MSGBOX_DEFAULT
	release
	end

NatureChanger_Text_Intro:
	.string "I can reshape a POKéMON's nature\n"
	.string "with the power of my mind.\p"
	.string "Would you like me to change the\n"
	.string "nature of one of your POKéMON?$"

NatureChanger_Text_WhichMon:
	.string "Which POKéMON should I work on?$"

NatureChanger_Text_WhichNature:
	.string "{STR_VAR_1} has a {STR_VAR_2} nature.\n"
	.string "Which nature should it have?$"

NatureChanger_Text_Done:
	.string "… … … … … …\p"
	.string "{STR_VAR_1}'s nature is now\n"
	.string "{STR_VAR_2}!$"

NatureChanger_Text_Egg:
	.string "I can't read the mind of an EGG…$"

NatureChanger_Text_ComeBack:
	.string "Come back whenever you like.$"
