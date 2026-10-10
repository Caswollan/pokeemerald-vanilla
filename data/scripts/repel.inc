EventScript_RepelWoreOff::
	lockall
	goto_if_eq VAR_LAST_REPEL_USED, ITEM_NONE, EventScript_RepelWoreOffNoMore
	checkitem VAR_LAST_REPEL_USED
	goto_if_eq VAR_RESULT, FALSE, EventScript_RepelWoreOffNoMore
	bufferitemname STR_VAR_2, VAR_LAST_REPEL_USED
	msgbox Text_RepelWoreOffUseAnother, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_RepelWoreOffEnd
	removeitem VAR_LAST_REPEL_USED
	special RestartLastRepel
	playse SE_REPEL
	msgbox Text_UsedAnotherRepel, MSGBOX_DEFAULT
	waitse
EventScript_RepelWoreOffEnd:
	releaseall
	end

EventScript_RepelWoreOffNoMore:
	msgbox Text_RepelWoreOff, MSGBOX_DEFAULT
	releaseall
	end

Text_RepelWoreOff:
	.string "REPEL's effect wore off…$"

Text_RepelWoreOffUseAnother:
	.string "REPEL's effect wore off…\p"
	.string "Use another {STR_VAR_2}?$"

Text_UsedAnotherRepel:
	.string "{PLAYER} used the\n"
	.string "{STR_VAR_2}.$"
