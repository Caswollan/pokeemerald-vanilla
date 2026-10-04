#!/usr/bin/env python3
"""Exports every ability of the ROM to a JSON file for the local website.

Reads the ability data straight from the source files:
  include/constants/abilities.h    ability numbers
  src/data/text/abilities.h        in-game names and descriptions
  src/data/pokemon/species_info.h  which Pokemon have each ability
The in-game descriptions are very short: EFFECT_TEXT below explains what each ability does,
including the changes made in the hack. Keep it in sync when an ability is changed.

Usage (from the repository root):
  python3 tools/export_abilities_json/export_abilities_json.py [output.json]
The default output is tools/export_abilities_json/abilities.json.
"""
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'export_pokemon_json'))
from export_pokemon_json import read, strip, parse_species_info, parse_national_dex  # noqa: E402

DEFAULT_OUTPUT = os.path.join(HERE, 'abilities.json')

EFFECT_TEXT = {
    'STENCH': 'Halves the wild encounter rate when the Pokemon leads the party. No effect in battle.',
    'DRIZZLE': 'Summons rain for 8 turns when the Pokemon enters battle.',
    'SPEED_BOOST': 'Raises Speed by one stage at the end of each turn.',
    'BATTLE_ARMOR': 'The Pokemon can\'t be hit by critical hits.',
    'STURDY': 'At full HP the Pokemon survives any hit with 1 HP. Immune to one-hit KO moves.',
    'DAMP': 'Prevents Self-Destruct and Explosion from being used by any Pokemon on the field.',
    'LIMBER': 'The Pokemon can\'t be paralyzed.',
    'SAND_VEIL': 'Raises evasion in a sandstorm and prevents sandstorm damage.',
    'STATIC': '30% chance to paralyze a Pokemon that makes contact with it.',
    'VOLT_ABSORB': 'Electric moves restore 1/4 of max HP instead of dealing damage.',
    'WATER_ABSORB': 'Water moves restore 1/4 of max HP instead of dealing damage.',
    'OBLIVIOUS': 'The Pokemon can\'t be infatuated or taunted and is immune to Intimidate.',
    'CLOUD_NINE': 'Cancels the effects of the weather.',
    'COMPOUND_EYES': 'Raises the accuracy of the Pokemon\'s moves by 30%.',
    'INSOMNIA': 'The Pokemon can\'t fall asleep.',
    'COLOR_CHANGE': 'The Pokemon\'s type changes to the type of the move that hits it.',
    'IMMUNITY': 'The Pokemon can\'t be poisoned.',
    'FLASH_FIRE': 'Fire moves don\'t affect the Pokemon and power up its own Fire moves by 50%.',
    'SHIELD_DUST': 'Blocks the secondary effects of moves that hit the Pokemon.',
    'OWN_TEMPO': 'The Pokemon can\'t be confused and is immune to Intimidate.',
    'SUCTION_CUPS': 'The Pokemon can\'t be forced to switch out.',
    'INTIMIDATE': 'Lowers the opponents\' Attack by one stage when the Pokemon enters battle.',
    'SHADOW_TAG': 'Opponents can\'t flee or switch out.',
    'ROUGH_SKIN': 'A Pokemon that makes contact with it loses 1/8 of its max HP.',
    'WONDER_GUARD': 'Only super effective moves can hit the Pokemon.',
    'LEVITATE': 'The Pokemon is immune to Ground moves.',
    'EFFECT_SPORE': '30% chance to poison, paralyze or put to sleep a Pokemon that makes contact with it. Grass types are immune.',
    'SYNCHRONIZE': 'When the Pokemon is burned, paralyzed or poisoned, the opponent gets the same status.',
    'CLEAR_BODY': 'Opponents can\'t lower the Pokemon\'s stats.',
    'NATURAL_CURE': 'Status conditions are cured when the Pokemon switches out.',
    'LIGHTNING_ROD': 'Draws Electric moves, is immune to them and raises Sp. Atk by one stage when hit by one.',
    'SERENE_GRACE': 'Doubles the chance of the secondary effects of the Pokemon\'s moves.',
    'SWIFT_SWIM': 'Doubles Speed in the rain.',
    'CHLOROPHYLL': 'Doubles Speed in the sun.',
    'ILLUMINATE': 'Raises the wild encounter rate when the Pokemon leads the party. No effect in battle.',
    'TRACE': 'Copies the opponent\'s ability when the Pokemon enters battle.',
    'HUGE_POWER': 'Doubles the Pokemon\'s Attack.',
    'POISON_POINT': '30% chance to poison a Pokemon that makes contact with it.',
    'INNER_FOCUS': 'The Pokemon can\'t flinch and is immune to Intimidate.',
    'MAGMA_ARMOR': 'The Pokemon can\'t get Frostbite.',
    'WATER_VEIL': 'The Pokemon can\'t be burned.',
    'MAGNET_PULL': 'Steel-type opponents can\'t flee or switch out.',
    'SOUNDPROOF': 'The Pokemon is immune to sound-based moves.',
    'RAIN_DISH': 'Restores 1/16 of max HP each turn in the rain.',
    'SAND_STREAM': 'Summons a sandstorm for 8 turns when the Pokemon enters battle.',
    'PRESSURE': 'Moves used against the Pokemon lose 2 PP instead of 1.',
    'THICK_FAT': 'Halves the damage of Fire and Ice moves.',
    'EARLY_BIRD': 'The Pokemon wakes up twice as fast.',
    'FLAME_BODY': '30% chance to burn a Pokemon that makes contact with it.',
    'RUN_AWAY': 'The Pokemon can always flee from wild battles.',
    'KEEN_EYE': 'Opponents can\'t lower the Pokemon\'s accuracy, and it ignores the target\'s evasion boosts.',
    'HYPER_CUTTER': 'Opponents can\'t lower the Pokemon\'s Attack.',
    'PICKUP': 'May pick up an item after a battle.',
    'TRUANT': 'The Pokemon can only act every other turn.',
    'HUSTLE': 'Raises Attack by 50% but lowers the accuracy of physical moves by 20%.',
    'CUTE_CHARM': '30% chance to infatuate a Pokemon of the opposite gender that makes contact with it.',
    'PLUS': 'Raises Sp. Atk by 50% if a Pokemon with Minus is on the field.',
    'MINUS': 'Raises Sp. Atk by 50% if a Pokemon with Plus is on the field.',
    'FORECAST': 'Castform changes type and form with the weather.',
    'STICKY_HOLD': 'The Pokemon\'s held item can\'t be removed or stolen.',
    'SHED_SKIN': '1/3 chance to cure its status condition at the end of each turn.',
    'GUTS': 'Raises Attack by 50% when the Pokemon has a status condition.',
    'MARVEL_SCALE': 'Raises Defense by 50% when the Pokemon has a status condition.',
    'LIQUID_OOZE': 'Moves that drain HP from the Pokemon hurt the attacker instead.',
    'OVERGROW': 'Raises the power of Grass moves by 50% when HP is at 1/3 or less.',
    'BLAZE': 'Raises the power of Fire moves by 50% when HP is at 1/3 or less.',
    'TORRENT': 'Raises the power of Water moves by 50% when HP is at 1/3 or less.',
    'SWARM': 'Raises the power of Bug moves by 50% when HP is at 1/3 or less.',
    'ROCK_HEAD': 'The Pokemon takes no recoil damage.',
    'DROUGHT': 'Summons sun for 8 turns when the Pokemon enters battle.',
    'ARENA_TRAP': 'Grounded opponents can\'t flee or switch out.',
    'VITAL_SPIRIT': 'The Pokemon can\'t fall asleep.',
    'WHITE_SMOKE': 'Opponents can\'t lower the Pokemon\'s stats.',
    'PURE_POWER': 'Doubles the Pokemon\'s Attack.',
    'SHELL_ARMOR': 'The Pokemon can\'t be hit by critical hits.',
    'CACOPHONY': 'The Pokemon is immune to sound-based moves.',
    'AIR_LOCK': 'Cancels the effects of the weather.',
    'SNOW_WARNING': 'Summons hail for 8 turns when the Pokemon enters battle.',
}

NAME_FIXES = {}


def readable(constant):
    return NAME_FIXES.get(constant, ' '.join(w.capitalize() for w in constant.split('_')))


def main():
    output = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_OUTPUT
    numbers = {int(n): name for name, n in re.findall(r'#define (ABILITY_\w+) (\d+)', read('include/constants/abilities.h'))}
    text = read('src/data/text/abilities.h')
    names = dict(re.findall(r'\[(ABILITY_\w+)\] = _\("(.*?)"\)', text))
    desc_texts = dict(re.findall(r'static const u8 (s\w+Description)\[\] = _\("(.*?)"\);', text))
    desc_pointers = dict(re.findall(r'\[(ABILITY_\w+)\] = (s\w+Description)', text))

    # Which Pokemon have each ability, in National Dex order
    dex = parse_national_dex()
    holders = {}
    for sp, data in sorted(parse_species_info().items(), key=lambda kv: dex.get(kv[0], ('', 9999))[1]):
        if sp not in dex:
            continue
        for slot in ('ability1', 'ability2'):
            ability = data['abilities'][slot]
            if ability:
                holders.setdefault(ability, []).append(strip('SPECIES_', sp))

    abilities, missing = [], []
    for number in sorted(numbers):
        constant = strip('ABILITY_', numbers[number])
        if constant == 'NONE':
            continue
        if constant not in EFFECT_TEXT:
            missing.append(constant)
        abilities.append({
            'id': number,
            'ability': constant,
            'name': readable(constant),
            'gameName': names.get(numbers[number]),
            'description': desc_texts.get(desc_pointers.get(numbers[number])),
            'effect': EFFECT_TEXT.get(constant),
            'pokemon': holders.get(constant, []),
        })

    with open(output, 'w', encoding='utf-8', newline='\n') as f:
        json.dump(abilities, f, ensure_ascii=False, indent=2)
        f.write('\n')
    print(f'{len(abilities)} abilities written to {output}')
    if missing:
        print('Abilities without a description in EFFECT_TEXT:', ', '.join(missing))


if __name__ == '__main__':
    main()
