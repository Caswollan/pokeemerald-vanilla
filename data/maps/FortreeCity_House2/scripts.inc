FortreeCity_House2_MapScripts::
	.byte 0

FortreeCity_House2_EventScript_PetalDanceGiver::
	lock
	faceplayer
	goto_if_set FLAG_RECEIVED_TM_PETAL_DANCE, FortreeCity_House2_EventScript_ExplainPetalDance
	call_if_unset FLAG_MET_PETAL_DANCE_GIVER, FortreeCity_House2_EventScript_Greeting
	msgbox FortreeCity_House2_Text_TakeThisTM, MSGBOX_DEFAULT
	giveitem ITEM_TM_PETAL_DANCE
	goto_if_eq VAR_RESULT, 0, Common_EventScript_ShowBagIsFull
	setflag FLAG_RECEIVED_TM_PETAL_DANCE
	msgbox FortreeCity_House2_Text_ExplainPetalDance, MSGBOX_DEFAULT
	release
	end

FortreeCity_House2_EventScript_Greeting::
	msgbox FortreeCity_House2_Text_StrengthBlossomsInNature, MSGBOX_DEFAULT
	setflag FLAG_MET_PETAL_DANCE_GIVER
	return

FortreeCity_House2_EventScript_ExplainPetalDance::
	msgbox FortreeCity_House2_Text_ExplainPetalDance, MSGBOX_DEFAULT
	release
	end

FortreeCity_House2_Text_StrengthBlossomsInNature:
	.string "People… POKéMON…\p"
	.string "Their strength blossoms when they\n"
	.string "live close to nature…$"

FortreeCity_House2_Text_TakeThisTM:
	.string "I can feel a strength in you,\n"
	.string "ready to bloom like a flower…\p"
	.string "Here, take this and let your\n"
	.string "POKéMON blossom too.$"

FortreeCity_House2_Text_ExplainPetalDance:
	.string "PETAL DANCE hits hard, but it\n"
	.string "sharply lowers the user's SP. ATK.$"

