CaveOfOrigin_B1F_MapScripts::
	.byte 0

CaveOfOrigin_B1F_EventScript_Wallace::
	lock
	faceplayer
	msgbox CaveOfOrigin_B1F_Text_WallaceStory, MSGBOX_DEFAULT
	closemessage
	playse SE_EXIT
	fadescreenspeed FADE_TO_BLACK, 4
	setflag FLAG_WALLACE_GOES_TO_SKY_PILLAR
	setvar VAR_SOOTOPOLIS_CITY_STATE, 3
	removeobject LOCALID_CAVE_OF_ORIGIN_WALLACE
	clearflag FLAG_HIDE_SKY_PILLAR_WALLACE
	fadescreen FADE_FROM_BLACK
	release
	end

CaveOfOrigin_B1F_Text_WallaceStory:
	.string "Ah, so you are {PLAYER}{KUN}?\n"
	.string "I've heard tales of your exploits.\p"
	.string "My name is WALLACE.\p"
	.string "I am the GYM LEADER of SOOTOPOLIS,\n"
	.string "but my GYM must wait for now.\p"
	.string "I've closed its doors until this\n"
	.string "crisis is over.\p"
	.string "GROUDON and KYOGRE, the two POKéMON\n"
	.string "wreaking havoc here, are considered\l"
	.string "to be super-ancient POKéMON.\p"
	.string "But there is a third one:\n"
	.string "RAYQUAZA.\p"
	.string "Long ago, it descended from the sky\n"
	.string "and becalmed the two combatants.\p"
	.string "… … … … … …\p"
	.string "Every GYM LEADER of SOOTOPOLIS is\n"
	.string "sworn to guard its resting place:\l"
	.string "the SKY PILLAR.\p"
	.string "It's an ancient tower rising from\n"
	.string "the sea on ROUTE 131, west of\l"
	.string "PACIFIDLOG TOWN.\p"
	.string "Its doors must never be opened…\n"
	.string "unless all hope is lost.\p"
	.string "That time has come.\p"
	.string "{PLAYER}{KUN}, there's not a moment to lose!\n"
	.string "We'll head to the SKY PILLAR right away!$"

