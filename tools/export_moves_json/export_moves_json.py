#!/usr/bin/env python3
"""Exports every move of the ROM to a JSON file for the local website.

Reads the move data straight from the source files:
  include/constants/moves.h           move numbers
  src/data/battle_moves.h             type, category, power, accuracy, PP, priority, effect
  src/data/text/move_names.h          in-game names
  src/data/text/move_descriptions.h   descriptions
The effect constants (EFFECT_*) have no readable text in the game: EFFECT_TEXT below turns
them into a short description. Keep it in sync when a move effect is changed in the hack.

Usage (from the repository root):
  python3 tools/export_moves_json/export_moves_json.py [output.json]
The default output is tools/export_moves_json/moves.json.
"""
import json
import os
import re
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
DEFAULT_OUTPUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'moves.json')

# {c} = secondaryEffectChance, {p} = priority. None = no secondary effect.
EFFECT_TEXT = {
    'HIT': None,
    # Damage with a chance of a side effect
    'BURN_HIT': '{c}% Burn',
    'FREEZE_HIT': '{c}% Frostbite',
    'PARALYZE_HIT': '{c}% Paralyze',
    'POISON_HIT': '{c}% Poison',
    'POISON_FANG': '{c}% Bad poison',
    'CONFUSE_HIT': '{c}% Confusion',
    'FLINCH_HIT': '{c}% Flinch',
    'FLINCH_MINIMIZE_HIT': '{c}% Flinch, double damage against Minimize',
    'TRI_ATTACK': '{c}% Burn, Frostbite or Paralyze',
    'THAW_HIT': '{c}% Burn, cures the user\'s Frostbite',
    'BLAZE_KICK': 'High critical hit ratio, {c}% Burn',
    'POISON_TAIL': 'High critical hit ratio, {c}% Poison',
    'THUNDER': '{c}% Paralyze, never misses in rain',
    'TWISTER': '{c}% Flinch, double damage against Fly / Bounce',
    'ATTACK_DOWN_HIT': '{c}% Lower Attack',
    'DEFENSE_DOWN_HIT': '{c}% Lower Defense',
    'SPEED_DOWN_HIT': '{c}% Lower Speed',
    'SPECIAL_ATTACK_DOWN_HIT': '{c}% Lower Sp. Atk',
    'SPECIAL_DEFENSE_DOWN_HIT': '{c}% Lower Sp. Def',
    'ACCURACY_DOWN_HIT': '{c}% Lower accuracy',
    'ATTACK_UP_HIT': '{c}% Raise the user\'s Attack',
    'DEFENSE_UP_HIT': '{c}% Raise the user\'s Defense',
    'ALL_STATS_UP_HIT': '{c}% Raise all the user\'s stats',
    'VOLT_TACKLE': '{c}% Paralyze, the user takes 1/3 of the damage dealt',
    'TWINEEDLE': 'Hits twice, {c}% Poison each hit',
    # Damage with a fixed side effect
    'HIGH_CRITICAL': 'High critical hit ratio',
    'ALWAYS_HIT': 'Never misses',
    'VITAL_THROW': 'Never misses, moves last',
    'QUICK_ATTACK': 'Priority +{p}',
    'FAKE_OUT': 'Priority +{p}, always flinches, works only on the first turn',
    'ABSORB': 'Restores half the damage dealt',
    'DREAM_EATER': 'Restores half the damage dealt, works only on a sleeping target',
    'RECOIL': 'The user takes 1/4 of the damage dealt',
    'DOUBLE_EDGE': 'The user takes 1/3 of the damage dealt',
    'RECOIL_IF_MISS': 'The user takes damage if it misses',
    'RECHARGE': 'The user must recharge next turn',
    'OVERHEAT': 'Sharply lowers the user\'s Sp. Atk',
    'SUPERPOWER': 'Lowers the user\'s Attack and Defense',
    'EXPLOSION': 'The user faints, halves the target\'s Defense',
    'MULTI_HIT': 'Hits 2-5 times',
    'DOUBLE_HIT': 'Hits twice',
    'TRIPLE_KICK': 'Hits 3 times, power rises with each hit',
    'BEAT_UP': 'Each healthy party member attacks',
    'OHKO': 'One-hit KO',
    'TRAP': 'Traps and damages the target for 2-5 turns',
    'RAMPAGE': 'Lasts 2-3 turns, then confuses the user',
    'ROLLOUT': 'Lasts 5 turns, power doubles each hit',
    'UPROAR': 'Lasts 2-5 turns, prevents sleep',
    'RAZOR_WIND': 'Charges on the first turn',
    'SKULL_BASH': 'Charges on the first turn, raises Defense',
    'SKY_ATTACK': 'Charges on the first turn',
    'SOLAR_BEAM': 'Charges on the first turn, hits immediately in the sun',
    'SEMI_INVULNERABLE': 'Semi-invulnerable on the first turn, hits on the second',
    'FOCUS_PUNCH': 'Moves last, fails if the user is hit first',
    'FUTURE_SIGHT': 'Hits two turns later',
    'PURSUIT': 'Double power against a switching target',
    'RAPID_SPIN': 'Frees the user from Wrap, Leech Seed and Spikes, raises the user\'s Speed',
    'FLAME_BURST': 'In double battles, also hits the target\'s partner for 1/16 of its max HP',
    'KNOCK_OFF': 'Removes the target\'s item; 30% more damage if it holds one (not against Sticky Hold)',
    'THIEF': 'Steals the target\'s item',
    'PAY_DAY': 'Scatters coins picked up after battle',
    'FALSE_SWIPE': 'Always leaves the target with at least 1 HP',
    'RAGE': 'Attack rises when the user is hit',
    'SECRET_POWER': 'Side effect depends on the terrain',
    'BRICK_BREAK': 'Breaks Reflect and Light Screen',
    'REVENGE': 'Double power if the user was hit this turn',
    'SMELLINGSALT': 'Double power against a paralyzed target, cures its paralysis',
    'FACADE': 'Double power if the user is poisoned, burned, paralyzed or has Frostbite',
    'GUST': 'Double damage against Fly / Bounce',
    'EARTHQUAKE': 'Double damage against Dig',
    'SKY_UPPERCUT': 'Hits a target in the air',
    'SNORE': '{c}% Flinch, works only while asleep',
    # Variable or fixed damage
    'LEVEL_DAMAGE': 'Damage equal to the user\'s level',
    'PSYWAVE': 'Damage from 0.5x to 1.5x the user\'s level',
    'SONICBOOM': 'Always deals 20 damage',
    'DRAGON_RAGE': 'Always deals 40 damage',
    'SUPER_FANG': 'Halves the target\'s HP',
    'ENDEAVOR': 'Lowers the target\'s HP to the user\'s HP',
    'COUNTER': 'Returns double the physical damage taken',
    'MIRROR_COAT': 'Returns double the special damage taken',
    'BIDE': 'Waits 2 turns, then returns double the damage taken',
    'FLAIL': 'More power the lower the user\'s HP',
    'ERUPTION': 'More power the higher the user\'s HP',
    'LOW_KICK': 'More power the heavier the target',
    'RETURN': 'More power the higher the friendship (max 102)',
    'FRUSTRATION': 'More power the lower the friendship (max 102)',
    'MAGNITUDE': 'Random power from 10 to 150',
    'PRESENT': 'Random power, may heal the target',
    'HIDDEN_POWER': 'Type and power depend on the user\'s IVs',
    'WEATHER_BALL': 'Type changes with the weather, double power in weather',
    'FURY_CUTTER': 'Power doubles with each consecutive hit (max 160)',
    'SPIT_UP': 'Power depends on Stockpile',
    # Stat changes
    'ATTACK_UP': 'Raises the user\'s Attack',
    'ATTACK_UP_2': 'Sharply raises the user\'s Attack',
    'DEFENSE_UP': 'Raises the user\'s Defense',
    'DEFENSE_UP_2': 'Sharply raises the user\'s Defense',
    'DEFENSE_CURL': 'Raises the user\'s Defense, doubles Rollout\'s power',
    'SPECIAL_ATTACK_UP': 'Raises the user\'s Sp. Atk',
    'SPECIAL_ATTACK_UP_2': 'Sharply raises the user\'s Sp. Atk',
    'SPECIAL_DEFENSE_UP_2': 'Sharply raises the user\'s Sp. Def',
    'SPEED_UP_2': 'Sharply raises the user\'s Speed',
    'EVASION_UP': 'Raises the user\'s evasion',
    'MINIMIZE': 'Raises the user\'s evasion',
    'BULK_UP': 'Raises the user\'s Attack and Defense',
    'CALM_MIND': 'Raises the user\'s Sp. Atk and Sp. Def',
    'COSMIC_POWER': 'Raises the user\'s Defense and Sp. Def',
    'DRAGON_DANCE': 'Raises the user\'s Attack and Speed',
    'BELLY_DRUM': 'Maximizes Attack at the cost of half the user\'s HP',
    'FOCUS_ENERGY': 'Raises the user\'s critical hit ratio',
    'ATTACK_DOWN': 'Lowers the target\'s Attack',
    'ATTACK_DOWN_2': 'Sharply lowers the target\'s Attack',
    'DEFENSE_DOWN': 'Lowers the target\'s Defense',
    'DEFENSE_DOWN_2': 'Sharply lowers the target\'s Defense',
    'SPEED_DOWN': 'Lowers the target\'s Speed',
    'SPEED_DOWN_2': 'Sharply lowers the target\'s Speed',
    'SPECIAL_DEFENSE_DOWN_2': 'Sharply lowers the target\'s Sp. Def',
    'ACCURACY_DOWN': 'Lowers the target\'s accuracy',
    'EVASION_DOWN': 'Lowers the target\'s evasion',
    'TICKLE': 'Lowers the target\'s Attack and Defense',
    'HAZE': 'Resets all stat changes',
    'PSYCH_UP': 'Copies the target\'s stat changes',
    'MEMENTO': 'The user faints, sharply lowers the target\'s Attack and Sp. Atk',
    # Status conditions
    'SLEEP': 'Puts the target to sleep',
    'YAWN': 'The target falls asleep next turn',
    'POISON': 'Poisons the target',
    'TOXIC': 'Badly poisons the target',
    'PARALYZE': 'Paralyzes the target',
    'WILL_O_WISP': 'Burns the target',
    'CONFUSE': 'Confuses the target',
    'SWAGGER': 'Sharply raises the target\'s Attack and confuses it',
    'FLATTER': 'Raises the target\'s Sp. Atk and confuses it',
    'TEETER_DANCE': 'Confuses all other Pokemon',
    'ATTRACT': 'Infatuates a target of the opposite gender',
    'NIGHTMARE': 'A sleeping target loses 1/4 of its HP each turn',
    'LEECH_SEED': 'Drains 1/8 of the target\'s HP each turn',
    'CURSE': 'Ghost: cuts its own HP to curse the target. Others: raises Attack and Defense, lowers Speed',
    'PERISH_SONG': 'All Pokemon on the field faint in 3 turns',
    # Healing
    'RESTORE_HP': 'Restores half the user\'s max HP',
    'SOFTBOILED': 'Restores half the user\'s max HP',
    'MORNING_SUN': 'Restores HP: 2/3 in the sun, 1/2 without weather, 1/4 in other weather',
    'SYNTHESIS': 'Restores HP: 2/3 in the sun, 1/2 without weather, 1/4 in other weather',
    'MOONLIGHT': 'Restores HP: 2/3 in the sun, 1/2 without weather, 1/4 in other weather',
    'REST': 'Fully restores HP and sleeps for 2 turns',
    'WISH': 'Restores half the max HP next turn',
    'INGRAIN': 'Restores HP each turn, prevents switching',
    'SWALLOW': 'Restores HP depending on Stockpile',
    'STOCKPILE': 'Stores energy for Spit Up / Swallow (up to 3)',
    'PAIN_SPLIT': 'Shares HP with the target',
    'HEAL_BELL': 'Cures the status of the whole party',
    'REFRESH': 'Cures the user\'s poison, burn and paralysis',
    # Field and protection
    'PROTECT': 'Protects the user this turn',
    'ENDURE': 'Survives this turn with at least 1 HP',
    'SUBSTITUTE': 'Creates a substitute with 1/4 of the user\'s max HP',
    'REFLECT': 'Halves physical damage for 5 turns',
    'LIGHT_SCREEN': 'Halves special damage for 5 turns',
    'SAFEGUARD': 'Prevents status conditions for 5 turns',
    'MIST': 'Prevents stat drops for 5 turns',
    'SPIKES': 'Damages Pokemon switching in (up to 3 layers)',
    'RAIN_DANCE': 'Rain for 5 turns',
    'SUNNY_DAY': 'Sun for 5 turns',
    'SANDSTORM': 'Sandstorm for 5 turns',
    'HAIL': 'Hail for 5 turns',
    'MUD_SPORT': 'Weakens Electric moves',
    'WATER_SPORT': 'Weakens Fire moves',
    'MAGIC_COAT': 'Reflects status moves back',
    'SNATCH': 'Steals the target\'s healing or stat-raising move',
    'FOLLOW_ME': 'Draws the opponents\' attacks to the user',
    'HELPING_HAND': 'Boosts the ally\'s attack',
    'CHARGE': 'Doubles the power of the user\'s next Electric move',
    # Switching and trapping
    'ROAR': 'Forces the target to switch out',
    'MEAN_LOOK': 'Prevents the target from escaping',
    'BATON_PASS': 'Switches out, passing stat changes',
    'TELEPORT': 'Flees from a wild battle',
    # Other
    'TAUNT': 'The target can only use attacking moves',
    'TORMENT': 'The target can\'t use the same move twice in a row',
    'DISABLE': 'Disables the target\'s last move',
    'ENCORE': 'The target repeats its last move for 2-6 turns',
    'IMPRISON': 'The target can\'t use moves the user knows',
    'SPITE': 'Lowers the PP of the target\'s last move',
    'GRUDGE': 'If the user faints, the move that KO\'d it loses all PP',
    'DESTINY_BOND': 'If the user faints, the attacker faints too',
    'LOCK_ON': 'The next move surely hits',
    'FORESIGHT': 'Removes the target\'s evasion and Ghost immunities',
    'TRICK': 'Swaps items with the target',
    'RECYCLE': 'Restores a used item',
    'ROLE_PLAY': 'Copies the target\'s ability',
    'SKILL_SWAP': 'Swaps abilities with the target',
    'CONVERSION': 'Changes the user\'s type to one of its moves\' types',
    'CONVERSION_2': 'Changes the user\'s type to resist the last move it took',
    'CAMOUFLAGE': 'Changes the user\'s type depending on the terrain',
    'TRANSFORM': 'Transforms into the target',
    'MIMIC': 'Copies the target\'s last move',
    'SKETCH': 'Permanently copies the target\'s last move',
    'METRONOME': 'Uses a random move',
    'MIRROR_MOVE': 'Uses the target\'s last move',
    'ASSIST': 'Uses a random move of a party member',
    'NATURE_POWER': 'Uses a move depending on the terrain',
    'SLEEP_TALK': 'Uses a random move while asleep',
    'SPLASH': 'No effect',
}

# Readable names where the constant isn't enough (dashes, in-game spellings)
NAME_FIXES = {
    'DOUBLE_EDGE': 'Double-Edge', 'SOFT_BOILED': 'Soft-Boiled', 'WILL_O_WISP': 'Will-O-Wisp',
    'LOCK_ON': 'Lock-On', 'MUD_SLAP': 'Mud-Slap', 'SELF_DESTRUCT': 'Self-Destruct',
    'CONVERSION_2': 'Conversion 2', 'SAND_ATTACK': 'Sand-Attack',
}


def read(path):
    with open(os.path.join(ROOT, path), encoding='utf-8') as f:
        return f.read()


def strip(prefix, value):
    return value[len(prefix):] if value.startswith(prefix) else value


def readable(constant):
    return NAME_FIXES.get(constant, ' '.join(w.capitalize() for w in constant.split('_')))


def parse_move_numbers():
    return {int(n): name for name, n in re.findall(r'#define (MOVE_\w+)\s+(\d+)', read('include/constants/moves.h'))
            if name not in ('MOVE_NONE', 'MOVES_COUNT') and not name.startswith('MOVE_UNAVAILABLE')}


def parse_battle_moves():
    src = read('src/data/battle_moves.h')
    moves = {}
    for m in re.finditer(r'^    \[(MOVE_\w+)\] =\s*\n    \{\n(.*?)\n    \},?$', src, re.M | re.S):
        fields = dict(re.findall(r'\.(\w+)\s*=\s*([^,\n]+),', m.group(2)))
        moves[m.group(1)] = fields
    return moves


def parse_names():
    return dict(re.findall(r'\[(MOVE_\w+)\] = _\("(.*?)"\)', read('src/data/text/move_names.h')))


def parse_descriptions():
    src = read('src/data/text/move_descriptions.h')
    texts = {}
    for m in re.finditer(r'static const u8 (s\w+Description)\[\] = _\((.*?)\);', src, re.S):
        parts = re.findall(r'"((?:[^"\\]|\\.)*)"', m.group(2))
        texts[m.group(1)] = re.sub(r'\s+', ' ', ''.join(parts).replace('\\n', ' ')).strip()
    pointers = dict(re.findall(r'\[(MOVE_\w+) - 1\] = (s\w+Description)', src))
    return {move: texts.get(var) for move, var in pointers.items()}


def main():
    output = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_OUTPUT
    numbers = parse_move_numbers()
    data = parse_battle_moves()
    names = parse_names()
    descriptions = parse_descriptions()

    moves, unknown = [], set()
    for number in sorted(numbers):
        constant = numbers[number]
        f = data.get(constant)
        if f is None:
            continue
        effect = strip('EFFECT_', f['effect'])
        category = strip('SPLIT_', f.get('split', 'SPLIT_STATUS'))
        power = int(f['power'])
        accuracy = int(f['accuracy'])
        chance = int(f['secondaryEffectChance'])
        priority = int(f['priority'])
        if effect not in EFFECT_TEXT:
            unknown.add(effect)
        text = EFFECT_TEXT.get(effect)
        moves.append({
            'id': number,
            'move': strip('MOVE_', constant),
            'name': readable(strip('MOVE_', constant)),
            'gameName': names.get(constant),
            'type': strip('TYPE_', f['type']),
            'category': category,
            'power': power if power > 1 else None,            # null: status move or variable/fixed damage
            'accuracy': accuracy if accuracy > 0 else None,   # null: never misses / doesn't check accuracy
            'pp': int(f['pp']),
            'priority': priority,
            'description': descriptions.get(constant),
            'effect': effect,
            'effectChance': chance,
            'secondaryEffect': text.format(c=chance, p=priority) if text else None,
        })

    with open(output, 'w', encoding='utf-8', newline='\n') as out:
        json.dump(moves, out, ensure_ascii=False, indent=2)
        out.write('\n')
    print(f'{len(moves)} moves written to {output}')
    if unknown:
        print('Effects without a description in EFFECT_TEXT:', ', '.join(sorted(unknown)))


if __name__ == '__main__':
    main()
