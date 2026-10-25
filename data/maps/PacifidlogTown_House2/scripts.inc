PacifidlogTown_House2_MapScripts::
	.byte 0

@ The Fan Club Chairman's younger brother: he no longer gives TMs, he just talks about his vacation
PacifidlogTown_House2_EventScript_FanClubYoungerBrother::
	msgbox PacifidlogTown_House2_Text_ChairmansYoungerBrotherOnVacation, MSGBOX_NPC
	end

PacifidlogTown_House2_EventScript_HappyAzurill::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_AZURILL, CRY_MODE_NORMAL
	msgbox PacifidlogTown_House2_Text_Rurii, MSGBOX_DEFAULT
	waitmoncry
	msgbox PacifidlogTown_House2_Text_VeryFriendlyWithTrainer, MSGBOX_DEFAULT
	release
	end

PacifidlogTown_House2_EventScript_UnhappyAzurill::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_AZURILL, CRY_MODE_ENCOUNTER
	msgbox PacifidlogTown_House2_Text_Rururi, MSGBOX_DEFAULT
	waitmoncry
	msgbox PacifidlogTown_House2_Text_DoesntLikeTrainerVeryMuch, MSGBOX_DEFAULT
	release
	end

PacifidlogTown_House2_Text_ChairmansYoungerBrotherOnVacation:
	.string "Er-hem!\p"
	.string "I am the POKéMON FAN CLUB's most\n"
	.string "important person, the CHAIRMAN's\l"
	.string "younger brother.\p"
	.string "I'm here enjoying my vacation with\n"
	.string "POKéMON, yes, indeed.$"

PacifidlogTown_House2_Text_Rurii:
	.string "AZURILL: Rurii.$"

PacifidlogTown_House2_Text_VeryFriendlyWithTrainer:
	.string "It appears to be very friendly with the\n"
	.string "TRAINER.$"

PacifidlogTown_House2_Text_Rururi:
	.string "AZURILL: Rururi!$"

PacifidlogTown_House2_Text_DoesntLikeTrainerVeryMuch:
	.string "It doesn't appear to like the TRAINER\n"
	.string "very much.$"

