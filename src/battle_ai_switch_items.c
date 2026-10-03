#include "global.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_controllers.h"
#include "battle_main.h"
#include "battle_script_commands.h"
#include "data.h"
#include "pokemon.h"
#include "random.h"
#include "util.h"
#include "constants/abilities.h"
#include "constants/item_effects.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/species.h"

// this file's functions
static bool8 HasSuperEffectiveMoveAgainstOpponents(bool8 noRng);
static bool8 FindMonWithFlagsAndSuperEffective(u8 flags, u8 moduloPercent);
static bool8 ShouldUseItem(void);

static bool8 ShouldSwitchIfPerishSong(void)
{
    if (gStatuses3[gActiveBattler] & STATUS3_PERISH_SONG
        && gDisableStructs[gActiveBattler].perishSongTimer == 0)
    {
        *(gBattleStruct->AI_monToSwitchIntoId + gActiveBattler) = PARTY_SIZE;
        BtlController_EmitTwoReturnValues(B_COMM_TO_ENGINE, B_ACTION_SWITCH, 0);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static bool8 ShouldSwitchIfWonderGuard(void)
{
    u8 opposingPosition;
    u8 opposingBattler;
    u8 moveFlags;
    s32 i, j;
    s32 firstId;
    s32 lastId; // + 1
    struct Pokemon *party = NULL;
    u16 move;

    if (gBattleTypeFlags & BATTLE_TYPE_DOUBLE)
        return FALSE;

    opposingPosition = BATTLE_OPPOSITE(GetBattlerPosition(gActiveBattler));

    if (gBattleMons[GetBattlerAtPosition(opposingPosition)].ability != ABILITY_WONDER_GUARD)
        return FALSE;

    // Check if Pokémon has a super effective move.
    for (opposingBattler = GetBattlerAtPosition(opposingPosition), i = 0; i < MAX_MON_MOVES; i++)
    {
        move = gBattleMons[gActiveBattler].moves[i];
        if (move == MOVE_NONE)
            continue;

        moveFlags = AI_TypeCalc(move, gBattleMons[opposingBattler].species, gBattleMons[opposingBattler].ability);
        if (moveFlags & MOVE_RESULT_SUPER_EFFECTIVE)
            return FALSE;
    }

    // Get party information.
    if (gBattleTypeFlags & (BATTLE_TYPE_TWO_OPPONENTS | BATTLE_TYPE_TOWER_LINK_MULTI))
    {
        if ((gActiveBattler & BIT_FLANK) == B_FLANK_LEFT)
            firstId = 0, lastId = PARTY_SIZE / 2;
        else
            firstId = PARTY_SIZE / 2, lastId = PARTY_SIZE;
    }
    else
    {
        firstId = 0, lastId = PARTY_SIZE;
    }

    if (GetBattlerSide(gActiveBattler) == B_SIDE_PLAYER)
        party = gPlayerParty;
    else
        party = gEnemyParty;

    // Find a Pokémon in the party that has a super effective move.
    for (i = firstId; i < lastId; i++)
    {
        if (GetMonData(&party[i], MON_DATA_HP) == 0)
            continue;
        if (GetMonData(&party[i], MON_DATA_SPECIES_OR_EGG) == SPECIES_NONE)
            continue;
        if (GetMonData(&party[i], MON_DATA_SPECIES_OR_EGG) == SPECIES_EGG)
            continue;
        if (i == gBattlerPartyIndexes[gActiveBattler])
            continue;

        GetMonData(&party[i], MON_DATA_SPECIES); // Unused return value.
        GetMonData(&party[i], MON_DATA_ABILITY_NUM); // Unused return value.

        for (opposingBattler = GetBattlerAtPosition(opposingPosition), j = 0; j < MAX_MON_MOVES; j++)
        {
            move = GetMonData(&party[i], MON_DATA_MOVE1 + j);
            if (move == MOVE_NONE)
                continue;

            moveFlags = AI_TypeCalc(move, gBattleMons[opposingBattler].species, gBattleMons[opposingBattler].ability);
            if (moveFlags & MOVE_RESULT_SUPER_EFFECTIVE && Random() % 3 < 2)
            {
                // We found a mon.
                *(gBattleStruct->AI_monToSwitchIntoId + gActiveBattler) = i;
                BtlController_EmitTwoReturnValues(B_COMM_TO_ENGINE, B_ACTION_SWITCH, 0);
                return TRUE;
            }
        }
    }

    return FALSE; // There is not a single Pokémon in the party that has a super effective move against a mon with Wonder Guard.
}

static bool8 FindMonThatAbsorbsOpponentsMove(void)
{
    u8 battlerIn1, battlerIn2;
    u8 absorbingTypeAbility;
    s32 firstId;
    s32 lastId; // + 1
    struct Pokemon *party;
    s32 i;

    if (HasSuperEffectiveMoveAgainstOpponents(TRUE) && Random() % 3 != 0)
        return FALSE;
    if (gLastLandedMoves[gActiveBattler] == MOVE_NONE)
        return FALSE;
    if (gLastLandedMoves[gActiveBattler] == MOVE_UNAVAILABLE)
        return FALSE;
    if (gBattleMoves[gLastLandedMoves[gActiveBattler]].power == 0)
        return FALSE;

    if (gBattleTypeFlags & BATTLE_TYPE_DOUBLE)
    {
        battlerIn1 = gActiveBattler;
        if (gAbsentBattlerFlags & gBitTable[GetBattlerAtPosition(BATTLE_PARTNER(GetBattlerPosition(gActiveBattler)))])
            battlerIn2 = gActiveBattler;
        else
            battlerIn2 = GetBattlerAtPosition(BATTLE_PARTNER(GetBattlerPosition(gActiveBattler)));
    }
    else
    {
        battlerIn1 = gActiveBattler;
        battlerIn2 = gActiveBattler;
    }

    if (gBattleMoves[gLastLandedMoves[gActiveBattler]].type == TYPE_FIRE)
        absorbingTypeAbility = ABILITY_FLASH_FIRE;
    else if (gBattleMoves[gLastLandedMoves[gActiveBattler]].type == TYPE_WATER)
        absorbingTypeAbility = ABILITY_WATER_ABSORB;
    else if (gBattleMoves[gLastLandedMoves[gActiveBattler]].type == TYPE_ELECTRIC)
        absorbingTypeAbility = ABILITY_VOLT_ABSORB;
    else
        return FALSE;

    if (gBattleMons[gActiveBattler].ability == absorbingTypeAbility)
        return FALSE;

    if (gBattleTypeFlags & (BATTLE_TYPE_TWO_OPPONENTS | BATTLE_TYPE_TOWER_LINK_MULTI))
    {
        if ((gActiveBattler & BIT_FLANK) == B_FLANK_LEFT)
            firstId = 0, lastId = PARTY_SIZE / 2;
        else
            firstId = PARTY_SIZE / 2, lastId = PARTY_SIZE;
    }
    else
    {
        firstId = 0, lastId = PARTY_SIZE;
    }

    if (GetBattlerSide(gActiveBattler) == B_SIDE_PLAYER)
        party = gPlayerParty;
    else
        party = gEnemyParty;

    for (i = firstId; i < lastId; i++)
    {
        u16 species;
        u8 monAbility;

        if (GetMonData(&party[i], MON_DATA_HP) == 0)
            continue;
        if (GetMonData(&party[i], MON_DATA_SPECIES_OR_EGG) == SPECIES_NONE)
            continue;
        if (GetMonData(&party[i], MON_DATA_SPECIES_OR_EGG) == SPECIES_EGG)
            continue;
        if (i == gBattlerPartyIndexes[battlerIn1])
            continue;
        if (i == gBattlerPartyIndexes[battlerIn2])
            continue;
        if (i == *(gBattleStruct->monToSwitchIntoId + battlerIn1))
            continue;
        if (i == *(gBattleStruct->monToSwitchIntoId + battlerIn2))
            continue;

        species = GetMonData(&party[i], MON_DATA_SPECIES);
        if (GetMonData(&party[i], MON_DATA_ABILITY_NUM) != 0)
            monAbility = gSpeciesInfo[species].abilities[1];
        else
            monAbility = gSpeciesInfo[species].abilities[0];

        if (absorbingTypeAbility == monAbility && Random() & 1)
        {
            // we found a mon.
            *(gBattleStruct->AI_monToSwitchIntoId + gActiveBattler) = i;
            BtlController_EmitTwoReturnValues(B_COMM_TO_ENGINE, B_ACTION_SWITCH, 0);
            return TRUE;
        }
    }

    return FALSE;
}

static bool8 ShouldSwitchIfNaturalCure(void)
{
    if (!(gBattleMons[gActiveBattler].status1 & STATUS1_SLEEP))
        return FALSE;
    if (gBattleMons[gActiveBattler].ability != ABILITY_NATURAL_CURE)
        return FALSE;
    if (gBattleMons[gActiveBattler].hp < gBattleMons[gActiveBattler].maxHP / 2)
        return FALSE;

    if ((gLastLandedMoves[gActiveBattler] == MOVE_NONE
      || gLastLandedMoves[gActiveBattler] == MOVE_UNAVAILABLE)
     && Random() & 1)
    {
        *(gBattleStruct->AI_monToSwitchIntoId + gActiveBattler) = PARTY_SIZE;
        BtlController_EmitTwoReturnValues(B_COMM_TO_ENGINE, B_ACTION_SWITCH, 0);
        return TRUE;
    }
    else if (gBattleMoves[gLastLandedMoves[gActiveBattler]].power == 0
          && Random() & 1)
    {
        *(gBattleStruct->AI_monToSwitchIntoId + gActiveBattler) = PARTY_SIZE;
        BtlController_EmitTwoReturnValues(B_COMM_TO_ENGINE, B_ACTION_SWITCH, 0);
        return TRUE;
    }

    if (FindMonWithFlagsAndSuperEffective(MOVE_RESULT_DOESNT_AFFECT_FOE, 1))
        return TRUE;
    if (FindMonWithFlagsAndSuperEffective(MOVE_RESULT_NOT_VERY_EFFECTIVE, 1))
        return TRUE;

    if (Random() & 1)
    {
        *(gBattleStruct->AI_monToSwitchIntoId + gActiveBattler) = PARTY_SIZE;
        BtlController_EmitTwoReturnValues(B_COMM_TO_ENGINE, B_ACTION_SWITCH, 0);
        return TRUE;
    }

    return FALSE;
}

static bool8 HasSuperEffectiveMoveAgainstOpponents(bool8 noRng)
{
    u8 opposingPosition;
    u8 opposingBattler;
    s32 i;
    u8 moveFlags;
    u16 move;

    opposingPosition = BATTLE_OPPOSITE(GetBattlerPosition(gActiveBattler));
    opposingBattler = GetBattlerAtPosition(opposingPosition);

    if (!(gAbsentBattlerFlags & gBitTable[opposingBattler]))
    {
        for (i = 0; i < MAX_MON_MOVES; i++)
        {
            move = gBattleMons[gActiveBattler].moves[i];
            if (move == MOVE_NONE)
                continue;

            moveFlags = AI_TypeCalc(move, gBattleMons[opposingBattler].species, gBattleMons[opposingBattler].ability);
            if (moveFlags & MOVE_RESULT_SUPER_EFFECTIVE)
            {
                if (noRng)
                    return TRUE;
                if (Random() % 10 != 0)
                    return TRUE;
            }
        }
    }
    if (!(gBattleTypeFlags & BATTLE_TYPE_DOUBLE))
        return FALSE;

    opposingBattler = GetBattlerAtPosition(BATTLE_PARTNER(opposingPosition));

    if (!(gAbsentBattlerFlags & gBitTable[opposingBattler]))
    {
        for (i = 0; i < MAX_MON_MOVES; i++)
        {
            move = gBattleMons[gActiveBattler].moves[i];
            if (move == MOVE_NONE)
                continue;

            moveFlags = AI_TypeCalc(move, gBattleMons[opposingBattler].species, gBattleMons[opposingBattler].ability);
            if (moveFlags & MOVE_RESULT_SUPER_EFFECTIVE)
            {
                if (noRng)
                    return TRUE;
                if (Random() % 10 != 0)
                    return TRUE;
            }
        }
    }

    return FALSE;
}

static bool8 AreStatsRaised(void)
{
    u8 buffedStatsValue = 0;
    s32 i;

    for (i = 0; i < NUM_BATTLE_STATS; i++)
    {
        if (gBattleMons[gActiveBattler].statStages[i] > DEFAULT_STAT_STAGE)
            buffedStatsValue += gBattleMons[gActiveBattler].statStages[i] - DEFAULT_STAT_STAGE;
    }

    return (buffedStatsValue > 3);
}

static bool8 FindMonWithFlagsAndSuperEffective(u8 flags, u8 moduloPercent)
{
    u8 battlerIn1, battlerIn2;
    s32 firstId;
    s32 lastId; // + 1
    struct Pokemon *party;
    s32 i, j;
    u16 move;
    u8 moveFlags;

    if (gLastLandedMoves[gActiveBattler] == MOVE_NONE)
        return FALSE;
    if (gLastLandedMoves[gActiveBattler] == MOVE_UNAVAILABLE)
        return FALSE;
    if (gLastHitBy[gActiveBattler] == 0xFF)
        return FALSE;
    if (gBattleMoves[gLastLandedMoves[gActiveBattler]].power == 0)
        return FALSE;

    if (gBattleTypeFlags & BATTLE_TYPE_DOUBLE)
    {
        battlerIn1 = gActiveBattler;
        if (gAbsentBattlerFlags & gBitTable[GetBattlerAtPosition(BATTLE_PARTNER(GetBattlerPosition(gActiveBattler)))])
            battlerIn2 = gActiveBattler;
        else
            battlerIn2 = GetBattlerAtPosition(BATTLE_PARTNER(GetBattlerPosition(gActiveBattler)));
    }
    else
    {
        battlerIn1 = gActiveBattler;
        battlerIn2 = gActiveBattler;
    }

    if (gBattleTypeFlags & (BATTLE_TYPE_TWO_OPPONENTS | BATTLE_TYPE_TOWER_LINK_MULTI))
    {
        if ((gActiveBattler & BIT_FLANK) == 0)
            firstId = 0, lastId = PARTY_SIZE / 2;
        else
            firstId = PARTY_SIZE / 2, lastId = PARTY_SIZE;
    }
    else
    {
        firstId = 0, lastId = PARTY_SIZE;
    }

    if (GetBattlerSide(gActiveBattler) == B_SIDE_PLAYER)
        party = gPlayerParty;
    else
        party = gEnemyParty;

    for (i = firstId; i < lastId; i++)
    {
        u16 species;
        u8 monAbility;

        if (GetMonData(&party[i], MON_DATA_HP) == 0)
            continue;
        if (GetMonData(&party[i], MON_DATA_SPECIES_OR_EGG) == SPECIES_NONE)
            continue;
        if (GetMonData(&party[i], MON_DATA_SPECIES_OR_EGG) == SPECIES_EGG)
            continue;
        if (i == gBattlerPartyIndexes[battlerIn1])
            continue;
        if (i == gBattlerPartyIndexes[battlerIn2])
            continue;
        if (i == *(gBattleStruct->monToSwitchIntoId + battlerIn1))
            continue;
        if (i == *(gBattleStruct->monToSwitchIntoId + battlerIn2))
            continue;

        species = GetMonData(&party[i], MON_DATA_SPECIES);
        if (GetMonData(&party[i], MON_DATA_ABILITY_NUM) != 0)
            monAbility = gSpeciesInfo[species].abilities[1];
        else
            monAbility = gSpeciesInfo[species].abilities[0];

        moveFlags = AI_TypeCalc(gLastLandedMoves[gActiveBattler], species, monAbility);
        if (moveFlags & flags)
        {
            battlerIn1 = gLastHitBy[gActiveBattler];

            for (j = 0; j < MAX_MON_MOVES; j++)
            {
                move = GetMonData(&party[i], MON_DATA_MOVE1 + j);
                if (move == 0)
                    continue;

                moveFlags = AI_TypeCalc(move, gBattleMons[battlerIn1].species, gBattleMons[battlerIn1].ability);
                if (moveFlags & MOVE_RESULT_SUPER_EFFECTIVE && Random() % moduloPercent == 0)
                {
                    *(gBattleStruct->AI_monToSwitchIntoId + gActiveBattler) = i;
                    BtlController_EmitTwoReturnValues(B_COMM_TO_ENGINE, B_ACTION_SWITCH, 0);
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

static bool8 ShouldSwitch(void)
{
    u8 battlerIn1, battlerIn2;
    u8 *activeBattlerPtr; // Needed to match.
    s32 firstId;
    s32 lastId; // + 1
    struct Pokemon *party;
    s32 i;
    s32 availableToSwitch;

    if (gBattleMons[*(activeBattlerPtr = &gActiveBattler)].status2 & (STATUS2_WRAPPED | STATUS2_ESCAPE_PREVENTION))
        return FALSE;
    if (gStatuses3[gActiveBattler] & STATUS3_ROOTED)
        return FALSE;
    if (ABILITY_ON_OPPOSING_FIELD(gActiveBattler, ABILITY_SHADOW_TAG))
        return FALSE;
    if (ABILITY_ON_OPPOSING_FIELD(gActiveBattler, ABILITY_ARENA_TRAP)) // Misses the flying type and Levitate check.
        return FALSE;
    if (ABILITY_ON_FIELD2(ABILITY_MAGNET_PULL))
    {
        if (IS_BATTLER_OF_TYPE(gActiveBattler, TYPE_STEEL))
            return FALSE;
    }
    if (gBattleTypeFlags & BATTLE_TYPE_ARENA)
        return FALSE;

    availableToSwitch = 0;
    if (gBattleTypeFlags & BATTLE_TYPE_DOUBLE)
    {
        battlerIn1 = *activeBattlerPtr;
        if (gAbsentBattlerFlags & gBitTable[GetBattlerAtPosition(BATTLE_PARTNER(GetBattlerPosition(*activeBattlerPtr)))])
            battlerIn2 = *activeBattlerPtr;
        else
            battlerIn2 = GetBattlerAtPosition(BATTLE_PARTNER(GetBattlerPosition(*activeBattlerPtr)));
    }
    else
    {
        battlerIn1 = *activeBattlerPtr;
        battlerIn2 = *activeBattlerPtr;
    }

    if (gBattleTypeFlags & (BATTLE_TYPE_TWO_OPPONENTS | BATTLE_TYPE_TOWER_LINK_MULTI))
    {
        if ((gActiveBattler & BIT_FLANK) == B_FLANK_LEFT)
            firstId = 0, lastId = PARTY_SIZE / 2;
        else
            firstId = PARTY_SIZE / 2, lastId = PARTY_SIZE;
    }
    else
    {
        firstId = 0, lastId = PARTY_SIZE;
    }

    if (GetBattlerSide(gActiveBattler) == B_SIDE_PLAYER)
        party = gPlayerParty;
    else
        party = gEnemyParty;

    for (i = firstId; i < lastId; i++)
    {
        if (GetMonData(&party[i], MON_DATA_HP) == 0)
            continue;
        if (GetMonData(&party[i], MON_DATA_SPECIES_OR_EGG) == SPECIES_NONE)
            continue;
        if (GetMonData(&party[i], MON_DATA_SPECIES_OR_EGG) == SPECIES_EGG)
            continue;
        if (i == gBattlerPartyIndexes[battlerIn1])
            continue;
        if (i == gBattlerPartyIndexes[battlerIn2])
            continue;
        if (i == *(gBattleStruct->monToSwitchIntoId + battlerIn1))
            continue;
        if (i == *(gBattleStruct->monToSwitchIntoId + battlerIn2))
            continue;

        availableToSwitch++;
    }

    if (availableToSwitch == 0)
        return FALSE;
    if (ShouldSwitchIfPerishSong())
        return TRUE;
    if (ShouldSwitchIfWonderGuard())
        return TRUE;
    if (FindMonThatAbsorbsOpponentsMove())
        return TRUE;
    if (ShouldSwitchIfNaturalCure())
        return TRUE;
    if (HasSuperEffectiveMoveAgainstOpponents(FALSE))
        return FALSE;
    if (AreStatsRaised())
        return FALSE;
    if (FindMonWithFlagsAndSuperEffective(MOVE_RESULT_DOESNT_AFFECT_FOE, 2)
        || FindMonWithFlagsAndSuperEffective(MOVE_RESULT_NOT_VERY_EFFECTIVE, 3))
        return TRUE;

    return FALSE;
}

void AI_TrySwitchOrUseItem(void)
{
    struct Pokemon *party;
    u8 battlerIn1, battlerIn2;
    s32 firstId;
    s32 lastId; // + 1
    u8 battlerIdentity = GetBattlerPosition(gActiveBattler);

    if (GetBattlerSide(gActiveBattler) == B_SIDE_PLAYER)
        party = gPlayerParty;
    else
        party = gEnemyParty;

    if (gBattleTypeFlags & BATTLE_TYPE_TRAINER)
    {
        if (ShouldSwitch())
        {
            if (*(gBattleStruct->AI_monToSwitchIntoId + gActiveBattler) == PARTY_SIZE)
            {
                s32 monToSwitchId = GetMostSuitableMonToSwitchInto();
                if (monToSwitchId == PARTY_SIZE)
                {
                    if (!(gBattleTypeFlags & BATTLE_TYPE_DOUBLE))
                    {
                        battlerIn1 = GetBattlerAtPosition(battlerIdentity);
                        battlerIn2 = battlerIn1;
                    }
                    else
                    {
                        battlerIn1 = GetBattlerAtPosition(battlerIdentity);
                        battlerIn2 = GetBattlerAtPosition(BATTLE_PARTNER(battlerIdentity));
                    }

                    if (gBattleTypeFlags & (BATTLE_TYPE_TWO_OPPONENTS | BATTLE_TYPE_TOWER_LINK_MULTI))
                    {
                        if ((gActiveBattler & BIT_FLANK) == B_FLANK_LEFT)
                            firstId = 0, lastId = PARTY_SIZE / 2;
                        else
                            firstId = PARTY_SIZE / 2, lastId = PARTY_SIZE;
                    }
                    else
                    {
                        firstId = 0, lastId = PARTY_SIZE;
                    }

                    for (monToSwitchId = firstId; monToSwitchId < lastId; monToSwitchId++)
                    {
                        if (GetMonData(&party[monToSwitchId], MON_DATA_HP) == 0)
                            continue;
                        if (monToSwitchId == gBattlerPartyIndexes[battlerIn1])
                            continue;
                        if (monToSwitchId == gBattlerPartyIndexes[battlerIn2])
                            continue;
                        if (monToSwitchId == *(gBattleStruct->monToSwitchIntoId + battlerIn1))
                            continue;
                        if (monToSwitchId == *(gBattleStruct->monToSwitchIntoId + battlerIn2))
                            continue;

                        break;
                    }
                }

                *(gBattleStruct->AI_monToSwitchIntoId + gActiveBattler) = monToSwitchId;
            }

            *(gBattleStruct->monToSwitchIntoId + gActiveBattler) = *(gBattleStruct->AI_monToSwitchIntoId + gActiveBattler);
            return;
        }
        else if (ShouldUseItem())
        {
            return;
        }
    }

    BtlController_EmitTwoReturnValues(B_COMM_TO_ENGINE, B_ACTION_USE_MOVE, BATTLE_OPPOSITE(gActiveBattler) << 8);
}

// Battle state changed while simulating a party mon in the AI's battler slot
struct SwitchInSimulation
{
    struct BattlePokemon mon;
    struct DisableStruct disableStruct;
    struct ProtectStruct protectStruct;
    u32 status3;
    u16 choicedMove;
    u16 currentMove;
    s32 battleMoveDamage;
    u16 dynamicBasePower;
    u8 dynamicMoveType;
    u8 dmgMultiplier;
    u8 moveResultFlags;
    u8 critMultiplier;
    u8 potentialItemEffectBattler;
};

static void SaveSwitchInSimulationState(u8 battler, struct SwitchInSimulation *backup)
{
    backup->mon = gBattleMons[battler];
    backup->disableStruct = gDisableStructs[battler];
    backup->protectStruct = gProtectStructs[battler];
    backup->status3 = gStatuses3[battler];
    backup->choicedMove = gBattleStruct->choicedMove[battler];
    backup->currentMove = gCurrentMove;
    backup->battleMoveDamage = gBattleMoveDamage;
    backup->dynamicBasePower = gDynamicBasePower;
    backup->dynamicMoveType = gBattleStruct->dynamicMoveType;
    backup->dmgMultiplier = gBattleScripting.dmgMultiplier;
    backup->moveResultFlags = gMoveResultFlags;
    backup->critMultiplier = gCritMultiplier;
    backup->potentialItemEffectBattler = gPotentialItemEffectBattler;
}

static void RestoreSwitchInSimulationState(u8 battler, const struct SwitchInSimulation *backup)
{
    gBattleMons[battler] = backup->mon;
    gDisableStructs[battler] = backup->disableStruct;
    gProtectStructs[battler] = backup->protectStruct;
    gStatuses3[battler] = backup->status3;
    gBattleStruct->choicedMove[battler] = backup->choicedMove;
    gCurrentMove = backup->currentMove;
    gBattleMoveDamage = backup->battleMoveDamage;
    gDynamicBasePower = backup->dynamicBasePower;
    gBattleStruct->dynamicMoveType = backup->dynamicMoveType;
    gBattleScripting.dmgMultiplier = backup->dmgMultiplier;
    gMoveResultFlags = backup->moveResultFlags;
    gCritMultiplier = backup->critMultiplier;
    gPotentialItemEffectBattler = backup->potentialItemEffectBattler;
}

// Puts a party mon in the battler slot, as it would be right after switching in
static void SimulateMonSwitchIn(u8 battler, struct Pokemon *mon)
{
    struct BattlePokemon *dst = &gBattleMons[battler];
    u8 *bytes;
    s32 i;

    bytes = (u8 *)dst;
    for (i = 0; i < (s32)sizeof(*dst); i++)
        bytes[i] = 0;
    bytes = (u8 *)&gDisableStructs[battler];
    for (i = 0; i < (s32)sizeof(gDisableStructs[battler]); i++)
        bytes[i] = 0;
    bytes = (u8 *)&gProtectStructs[battler];
    for (i = 0; i < (s32)sizeof(gProtectStructs[battler]); i++)
        bytes[i] = 0;
    gStatuses3[battler] = 0;
    gBattleStruct->choicedMove[battler] = MOVE_NONE;

    dst->species = GetMonData(mon, MON_DATA_SPECIES);
    dst->level = GetMonData(mon, MON_DATA_LEVEL);
    dst->hp = GetMonData(mon, MON_DATA_HP);
    dst->maxHP = GetMonData(mon, MON_DATA_MAX_HP);
    dst->attack = GetMonData(mon, MON_DATA_ATK);
    dst->defense = GetMonData(mon, MON_DATA_DEF);
    dst->speed = GetMonData(mon, MON_DATA_SPEED);
    dst->spAttack = GetMonData(mon, MON_DATA_SPATK);
    dst->spDefense = GetMonData(mon, MON_DATA_SPDEF);
    dst->hpIV = GetMonData(mon, MON_DATA_HP_IV);
    dst->attackIV = GetMonData(mon, MON_DATA_ATK_IV);
    dst->defenseIV = GetMonData(mon, MON_DATA_DEF_IV);
    dst->speedIV = GetMonData(mon, MON_DATA_SPEED_IV);
    dst->spAttackIV = GetMonData(mon, MON_DATA_SPATK_IV);
    dst->spDefenseIV = GetMonData(mon, MON_DATA_SPDEF_IV);
    dst->abilityNum = GetMonData(mon, MON_DATA_ABILITY_NUM);
    dst->ability = GetAbilityBySpecies(dst->species, dst->abilityNum);
    dst->types[0] = gSpeciesInfo[dst->species].types[0];
    dst->types[1] = gSpeciesInfo[dst->species].types[1];
    dst->item = GetMonData(mon, MON_DATA_HELD_ITEM);
    dst->friendship = GetMonData(mon, MON_DATA_FRIENDSHIP);
    dst->personality = GetMonData(mon, MON_DATA_PERSONALITY);
    dst->status1 = GetMonData(mon, MON_DATA_STATUS);
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        dst->moves[i] = GetMonData(mon, MON_DATA_MOVE1 + i);
        dst->pp[i] = GetMonData(mon, MON_DATA_PP1 + i);
    }
    for (i = 0; i < NUM_BATTLE_STATS; i++)
        dst->statStages[i] = DEFAULT_STAT_STAGE;
}

// Switch-in score of the mon in 'battler' against 'opposingBattler' (Run & Bun post-KO switch AI).
// Damage is compared as a percentage of the current HP, with max damage rolls.
static s32 GetSwitchInScore(u8 battler, u8 opposingBattler)
{
    u16 species = gBattleMons[battler].species;
    bool32 isFaster = GetBattlerTurnOrderSpeed(battler) >= GetBattlerTurnOrderSpeed(opposingBattler); // speed ties count as faster
    s32 damageDealt = AI_GetBestDamage(battler, opposingBattler, 100);
    s32 damageTaken = AI_GetBestDamage(opposingBattler, battler, 100);
    bool32 ohkos = damageDealt >= gBattleMons[opposingBattler].hp;
    bool32 isOhkod = damageTaken >= gBattleMons[battler].hp;
    s32 dealtPercent = min(damageDealt * 100 / gBattleMons[opposingBattler].hp, 100);
    s32 takenPercent = min(damageTaken * 100 / gBattleMons[battler].hp, 100);

    if (species == SPECIES_DITTO)
        return 2;
    if ((species == SPECIES_WOBBUFFET || species == SPECIES_WYNAUT) && !(!isFaster && isOhkod))
        return 2;

    if (isFaster && ohkos)
        return 5;
    if (!isFaster && ohkos && !isOhkod)
        return 4;
    if (isFaster && dealtPercent > takenPercent)
        return 3;
    if (!isFaster && dealtPercent > takenPercent)
        return 2;
    if (isFaster)
        return 1;
    if (isOhkod)
        return -1;
    return 0;
}

// Opponent a switched-in mon will face. In doubles each slot looks at the slot in front of it.
static u8 GetSwitchInOpponent(u8 battler)
{
    u8 opposingBattler = GetBattlerAtPosition(BATTLE_OPPOSITE(GetBattlerPosition(battler)));

    if ((gBattleTypeFlags & BATTLE_TYPE_DOUBLE)
        && ((gAbsentBattlerFlags & gBitTable[opposingBattler]) || gBattleMons[opposingBattler].hp == 0))
        opposingBattler = GetBattlerAtPosition(BATTLE_PARTNER(GetBattlerPosition(opposingBattler)));

    return opposingBattler;
}

u8 GetMostSuitableMonToSwitchInto(void)
{
    u8 opposingBattler;
    u8 bestMonId;
    u8 battlerIn1, battlerIn2;
    s32 firstId;
    s32 lastId; // + 1
    struct Pokemon *party;
    s32 i, score, bestScore;
    bool32 hasOpponent;
    struct SwitchInSimulation backup;

    if (*(gBattleStruct->monToSwitchIntoId + gActiveBattler) != PARTY_SIZE)
        return *(gBattleStruct->monToSwitchIntoId + gActiveBattler);
    if (gBattleTypeFlags & BATTLE_TYPE_ARENA)
        return gBattlerPartyIndexes[gActiveBattler] + 1;

    if (gBattleTypeFlags & BATTLE_TYPE_DOUBLE)
    {
        battlerIn1 = gActiveBattler;
        if (gAbsentBattlerFlags & gBitTable[GetBattlerAtPosition(BATTLE_PARTNER(GetBattlerPosition(gActiveBattler)))])
            battlerIn2 = gActiveBattler;
        else
            battlerIn2 = GetBattlerAtPosition(BATTLE_PARTNER(GetBattlerPosition(gActiveBattler)));
    }
    else
    {
        battlerIn1 = gActiveBattler;
        battlerIn2 = gActiveBattler;
    }

    if (gBattleTypeFlags & (BATTLE_TYPE_TWO_OPPONENTS | BATTLE_TYPE_TOWER_LINK_MULTI))
    {
        if ((gActiveBattler & BIT_FLANK) == B_FLANK_LEFT)
            firstId = 0, lastId = PARTY_SIZE / 2;
        else
            firstId = PARTY_SIZE / 2, lastId = PARTY_SIZE;
    }
    else
    {
        firstId = 0, lastId = PARTY_SIZE;
    }

    if (GetBattlerSide(gActiveBattler) == B_SIDE_PLAYER)
        party = gPlayerParty;
    else
        party = gEnemyParty;

    opposingBattler = GetSwitchInOpponent(gActiveBattler);
    hasOpponent = !(gAbsentBattlerFlags & gBitTable[opposingBattler]) && gBattleMons[opposingBattler].hp != 0;

    SaveSwitchInSimulationState(gActiveBattler, &backup);

    bestScore = 0;
    bestMonId = PARTY_SIZE;
    for (i = firstId; i < lastId; i++)
    {
        u16 species = GetMonData(&party[i], MON_DATA_SPECIES_OR_EGG);

        if (species == SPECIES_NONE || species == SPECIES_EGG)
            continue;
        if (GetMonData(&party[i], MON_DATA_HP) == 0)
            continue;
        if (gBattlerPartyIndexes[battlerIn1] == i)
            continue;
        if (gBattlerPartyIndexes[battlerIn2] == i)
            continue;
        if (i == *(gBattleStruct->monToSwitchIntoId + battlerIn1))
            continue;
        if (i == *(gBattleStruct->monToSwitchIntoId + battlerIn2))
            continue;

        // No opponent on the field to compare against: send out the first available mon
        if (!hasOpponent)
        {
            bestMonId = i;
            break;
        }

        SimulateMonSwitchIn(gActiveBattler, &party[i]);
        score = GetSwitchInScore(gActiveBattler, opposingBattler);

        // On ties the first mon in party order is kept
        if (bestMonId == PARTY_SIZE || score > bestScore)
        {
            bestScore = score;
            bestMonId = i;
        }
    }

    RestoreSwitchInSimulationState(gActiveBattler, &backup);

    return bestMonId;
}

static u8 GetAI_ItemType(u8 itemId, const u8 *itemEffect) // NOTE: should take u16 as item Id argument
{
    if (itemId == ITEM_FULL_RESTORE)
        return AI_ITEM_FULL_RESTORE;
    else if (itemEffect[4] & ITEM4_HEAL_HP)
        return AI_ITEM_HEAL_HP;
    else if (itemEffect[3] & ITEM3_STATUS_ALL)
        return AI_ITEM_CURE_CONDITION;
    else if (itemEffect[0] & (ITEM0_DIRE_HIT | ITEM0_X_ATTACK) || itemEffect[1] != 0 || itemEffect[2] != 0)
        return AI_ITEM_X_STAT;
    else if (itemEffect[3] & ITEM3_GUARD_SPEC)
        return AI_ITEM_GUARD_SPEC;
    else
        return AI_ITEM_NOT_RECOGNIZABLE;
}

static bool8 ShouldUseItem(void)
{
    struct Pokemon *party;
    s32 i;
    u8 validMons = 0;
    bool8 shouldUse = FALSE;

    if (gBattleTypeFlags & BATTLE_TYPE_INGAME_PARTNER && GetBattlerPosition(gActiveBattler) == B_POSITION_PLAYER_RIGHT)
        return FALSE;

    if (GetBattlerSide(gActiveBattler) == B_SIDE_PLAYER)
        party = gPlayerParty;
    else
        party = gEnemyParty;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&party[i], MON_DATA_HP) != 0
            && GetMonData(&party[i], MON_DATA_SPECIES_OR_EGG) != SPECIES_NONE
            && GetMonData(&party[i], MON_DATA_SPECIES_OR_EGG) != SPECIES_EGG)
        {
            validMons++;
        }
    }

    for (i = 0; i < MAX_TRAINER_ITEMS; i++)
    {
        u16 item;
        const u8 *itemEffects;
        u8 paramOffset;
        u8 battlerSide;

        if (i != 0 && validMons > (gBattleResources->battleHistory->itemsNo - i) + 1)
            continue;
        item = gBattleResources->battleHistory->trainerItems[i];
        if (item == ITEM_NONE)
            continue;
        if (gItemEffectTable[item - ITEM_POTION] == NULL)
            continue;

        if (item == ITEM_ENIGMA_BERRY)
            itemEffects = gSaveBlock1Ptr->enigmaBerry.itemEffect;
        else
            itemEffects = gItemEffectTable[item - ITEM_POTION];

        *(gBattleStruct->AI_itemType + gActiveBattler / 2) = GetAI_ItemType(item, itemEffects);

        switch (*(gBattleStruct->AI_itemType + gActiveBattler / 2))
        {
        case AI_ITEM_FULL_RESTORE:
            if (gBattleMons[gActiveBattler].hp >= gBattleMons[gActiveBattler].maxHP / 4)
                break;
            if (gBattleMons[gActiveBattler].hp == 0)
                break;
            shouldUse = TRUE;
            break;
        case AI_ITEM_HEAL_HP:
            paramOffset = GetItemEffectParamOffset(item, 4, ITEM4_HEAL_HP);
            if (paramOffset == 0)
                break;
            if (gBattleMons[gActiveBattler].hp == 0)
                break;
            if (gBattleMons[gActiveBattler].hp < gBattleMons[gActiveBattler].maxHP / 4 || gBattleMons[gActiveBattler].maxHP - gBattleMons[gActiveBattler].hp > itemEffects[paramOffset])
                shouldUse = TRUE;
            break;
        case AI_ITEM_CURE_CONDITION:
            *(gBattleStruct->AI_itemFlags + gActiveBattler / 2) = 0;
            if (itemEffects[3] & ITEM3_SLEEP && gBattleMons[gActiveBattler].status1 & STATUS1_SLEEP)
            {
                *(gBattleStruct->AI_itemFlags + gActiveBattler / 2) |= (1 << AI_HEAL_SLEEP);
                shouldUse = TRUE;
            }
            if (itemEffects[3] & ITEM3_POISON && (gBattleMons[gActiveBattler].status1 & STATUS1_POISON
                                               || gBattleMons[gActiveBattler].status1 & STATUS1_TOXIC_POISON))
            {
                *(gBattleStruct->AI_itemFlags + gActiveBattler / 2) |= (1 << AI_HEAL_POISON);
                shouldUse = TRUE;
            }
            if (itemEffects[3] & ITEM3_BURN && gBattleMons[gActiveBattler].status1 & STATUS1_BURN)
            {
                *(gBattleStruct->AI_itemFlags + gActiveBattler / 2) |= (1 << AI_HEAL_BURN);
                shouldUse = TRUE;
            }
            if (itemEffects[3] & ITEM3_FREEZE && gBattleMons[gActiveBattler].status1 & STATUS1_FREEZE)
            {
                *(gBattleStruct->AI_itemFlags + gActiveBattler / 2) |= (1 << AI_HEAL_FREEZE);
                shouldUse = TRUE;
            }
            if (itemEffects[3] & ITEM3_PARALYSIS && gBattleMons[gActiveBattler].status1 & STATUS1_PARALYSIS)
            {
                *(gBattleStruct->AI_itemFlags + gActiveBattler / 2) |= (1 << AI_HEAL_PARALYSIS);
                shouldUse = TRUE;
            }
            if (itemEffects[3] & ITEM3_CONFUSION && gBattleMons[gActiveBattler].status2 & STATUS2_CONFUSION)
            {
                *(gBattleStruct->AI_itemFlags + gActiveBattler / 2) |= (1 << AI_HEAL_CONFUSION);
                shouldUse = TRUE;
            }
            break;
        case AI_ITEM_X_STAT:
            *(gBattleStruct->AI_itemFlags + gActiveBattler / 2) = 0;
            if (gDisableStructs[gActiveBattler].isFirstTurn == 0)
                break;
            if (itemEffects[0] & ITEM0_X_ATTACK)
                *(gBattleStruct->AI_itemFlags + gActiveBattler / 2) |= (1 << AI_X_ATTACK);
            if (itemEffects[1] & ITEM1_X_DEFEND)
                *(gBattleStruct->AI_itemFlags + gActiveBattler / 2) |= (1 << AI_X_DEFEND);
            if (itemEffects[1] & ITEM1_X_SPEED)
                *(gBattleStruct->AI_itemFlags + gActiveBattler / 2) |= (1 << AI_X_SPEED);
            if (itemEffects[2] & ITEM2_X_SPATK)
                *(gBattleStruct->AI_itemFlags + gActiveBattler / 2) |= (1 << AI_X_SPATK);
            if (itemEffects[2] & ITEM2_X_ACCURACY)
                *(gBattleStruct->AI_itemFlags + gActiveBattler / 2) |= (1 << AI_X_ACCURACY);
            if (itemEffects[0] & ITEM0_DIRE_HIT)
                *(gBattleStruct->AI_itemFlags + gActiveBattler / 2) |= (1 << AI_DIRE_HIT);
            shouldUse = TRUE;
            break;
        case AI_ITEM_GUARD_SPEC:
            battlerSide = GetBattlerSide(gActiveBattler);
            if (gDisableStructs[gActiveBattler].isFirstTurn != 0 && gSideTimers[battlerSide].mistTimer == 0)
                shouldUse = TRUE;
            break;
        case AI_ITEM_NOT_RECOGNIZABLE:
            return FALSE;
        }

        if (shouldUse)
        {
            BtlController_EmitTwoReturnValues(B_COMM_TO_ENGINE, B_ACTION_USE_ITEM, 0);
            *(gBattleStruct->chosenItem + (gActiveBattler / 2) * 2) = item;
            gBattleResources->battleHistory->trainerItems[i] = ITEM_NONE;
            return shouldUse;
        }
    }

    return FALSE;
}
