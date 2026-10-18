MtPyre_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, MtPyre_1F_OnTransition
	.byte 0

@ Until Wally in Lilycove is defeated, a woman blocks the door to the outer path (the only way to the summit).
@ The guard object is hidden by FLAG_MET_RIVAL_LILYCOVE, the trigger is on while VAR_TEMP_1 is 1.
MtPyre_1F_OnTransition:
	call_if_unset FLAG_MET_RIVAL_LILYCOVE, MtPyre_1F_EventScript_EnableGuardTrigger
	end

MtPyre_1F_EventScript_EnableGuardTrigger::
	setvar VAR_TEMP_1, 1
	return

MtPyre_1F_EventScript_Guard::
	msgbox MtPyre_1F_Text_OuterPathClosed, MSGBOX_NPC
	end

@ Walking towards the door: sent back. Coming in from the outer path (old saves), the player can go on.
MtPyre_1F_EventScript_GuardTrigger::
	specialvar VAR_RESULT, GetPlayerFacingDirection
	goto_if_ne VAR_RESULT, DIR_WEST, MtPyre_1F_EventScript_GuardLetPass
	lockall
	applymovement LOCALID_MT_PYRE_1F_GUARD, Common_Movement_WalkInPlaceFasterUp
	waitmovement 0
	msgbox MtPyre_1F_Text_OuterPathClosed, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, MtPyre_1F_Movement_PlayerStepBackEast
	waitmovement 0
	applymovement LOCALID_MT_PYRE_1F_GUARD, Common_Movement_WalkInPlaceFasterLeft
	waitmovement 0
	releaseall
	end

MtPyre_1F_EventScript_GuardLetPass::
	end

MtPyre_1F_Movement_PlayerStepBackEast:
	walk_right
	step_end

MtPyre_1F_Text_OuterPathClosed:
	.string "Wait, please!\n"
	.string "The outer path to the summit is closed.$"

MtPyre_1F_EventScript_CleanseTagWoman::
	lock
	faceplayer
	goto_if_set FLAG_RECEIVED_CLEANSE_TAG, MtPyre_1F_EventScript_ReceivedCleanseTag
	msgbox MtPyre_1F_Text_TakeThisForYourOwnGood, MSGBOX_DEFAULT
	giveitem ITEM_CLEANSE_TAG
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_RECEIVED_CLEANSE_TAG
	release
	end

MtPyre_1F_EventScript_ReceivedCleanseTag::
	msgbox MtPyre_1F_Text_ExplainCleanseTag, MSGBOX_DEFAULT
	release
	end

MtPyre_1F_EventScript_PokefanF::
	msgbox MtPyre_1F_Text_ComeToPayRespects, MSGBOX_NPC
	end

MtPyre_1F_EventScript_Man::
	msgbox MtPyre_1F_Text_RestingPlaceOfZigzagoon, MSGBOX_NPC
	end

MtPyre_1F_Text_TakeThisForYourOwnGood:
	.string "All sorts of beings wander the slopes\n"
	.string "of MT. PYRE…\p"
	.string "There is no telling what may happen.\n"
	.string "Take this. It's for your own good.$"

MtPyre_1F_Text_ExplainCleanseTag:
	.string "Have a POKéMON hold that\n"
	.string "CLEANSE TAG.\p"
	.string "It will help ward off wild POKéMON.$"

MtPyre_1F_Text_ComeToPayRespects:
	.string "Did you come to pay your respect\n"
	.string "to the spirits of departed POKéMON?\p"
	.string "You must care for your POKéMON a lot.$"

MtPyre_1F_Text_RestingPlaceOfZigzagoon:
	.string "This is the final resting place of my\n"
	.string "ZIGZAGOON. I cherished it…$"

