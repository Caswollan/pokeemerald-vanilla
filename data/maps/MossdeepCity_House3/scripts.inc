MossdeepCity_House3_MapScripts::
	.byte 0

@ Fisherman: no longer gives the Super Rod (it is now given on Route 118)
MossdeepCity_House3_EventScript_SuperRodFisherman::
	msgbox MossdeepCity_House3_Text_SeafloorPokemon, MSGBOX_NPC
	end

MossdeepCity_House3_Text_SeafloorPokemon:
	.string "Hey there, TRAINER!\n"
	.string "Do you have a SUPER ROD?\p"
	.string "Say all you want, but that baby can\n"
	.string "catch POKéMON off the seafloor!$"
