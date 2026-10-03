#!/usr/bin/env python3
"""Exports every Pokemon species of the ROM to a JSON file for the local website.

Reads the species data straight from the source files:
  src/data/pokemon/species_info.h        types, abilities, base stats
  src/data/text/species_names.h          names
  src/data/pokemon/evolution.h           evolutions
  src/data/pokemon/level_up_learnsets.h  level-up moves
  src/data/pokemon/tmhm_learnsets.h      TM/HM moves
  src/pokemon.c, include/constants/pokedex.h   National Dex numbers

Usage (from the repository root):
  python3 tools/export_pokemon_json/export_pokemon_json.py [output.json]
The default output is tools/export_pokemon_json/pokemon.json.
"""
import json
import os
import re
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
DEFAULT_OUTPUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'pokemon.json')


def read(path):
    with open(os.path.join(ROOT, path), encoding='utf-8') as f:
        return f.read()


def strip(prefix, value):
    return value[len(prefix):] if value.startswith(prefix) else value


def display_name(raw):
    # "MR. MIME" -> "Mr. Mime", "HO-OH" -> "Ho-Oh", "FARFETCH'D" -> "Farfetch'd"
    return re.sub(r"[A-Za-z][A-Za-z']*", lambda m: m.group(0)[0].upper() + m.group(0)[1:].lower(), raw)


def item_name(item):
    return ' '.join(w.capitalize() for w in strip('ITEM_', item).split('_'))


def parse_species_info():
    src = read('src/data/pokemon/species_info.h')
    species = {}
    for m in re.finditer(r'^    \[(SPECIES_\w+)\] =\s*\n    \{\n(.*?)\n    \},?$', src, re.M | re.S):
        body = m.group(2)
        stat = lambda name: int(re.search(r'\.' + name + r'\s*=\s*(\d+)', body).group(1))
        types = re.search(r'\.types = \{\s*TYPE_(\w+),\s*TYPE_(\w+)', body).groups()
        abilities = re.search(r'\.abilities = \{\s*ABILITY_(\w+),\s*ABILITY_(\w+)', body).groups()
        species[m.group(1)] = {
            'types': list(dict.fromkeys(types)),
            'abilities': {
                'ability1': abilities[0] if abilities[0] != 'NONE' else None,
                'ability2': abilities[1] if abilities[1] != 'NONE' else None,
            },
            'stats': {
                'hp': stat('baseHP'),
                'attack': stat('baseAttack'),
                'defense': stat('baseDefense'),
                'spAttack': stat('baseSpAttack'),
                'spDefense': stat('baseSpDefense'),
                'speed': stat('baseSpeed'),
            },
        }
    return species


def parse_names():
    src = read('src/data/text/species_names.h')
    return dict(re.findall(r'\[(SPECIES_\w+)\] = _\("(.*?)"\)', src))


def parse_national_dex():
    numbers = {}
    enum = read('include/constants/pokedex.h')
    enum = enum[enum.index('NATIONAL_DEX_NONE'):]
    for i, name in enumerate(re.findall(r'^\s*(NATIONAL_DEX_\w+)\s*,', enum, re.M)):
        numbers[name] = i
    # src/pokemon.c maps each species with SPECIES_TO_NATIONAL(NAME) -> NATIONAL_DEX_NAME
    out = {}
    for name in re.findall(r'SPECIES_TO_NATIONAL\((\w+)\)', read('src/pokemon.c')):
        if f'NATIONAL_DEX_{name}' in numbers:
            out[f'SPECIES_{name}'] = (name, numbers[f'NATIONAL_DEX_{name}'])
    return out


EVO_TEXT = {
    'LEVEL': ('', 'at Level {p}'),
    'ITEM': ('', 'using {item}'),
    'FRIENDSHIP': ('', 'with high Friendship'),
    'FRIENDSHIP_DAY': ('during the day', 'with high Friendship during the day'),
    'FRIENDSHIP_NIGHT': ('at night', 'with high Friendship at night'),
    'TRADE': ('', 'by Trade'),
    'TRADE_ITEM': ('', 'by Trade holding {item}'),
    'BEAUTY': ('Beauty >= {p}', 'with Beauty of at least {p}'),
    'LEVEL_ATK_GT_DEF': ('Attack > Defense', 'at Level {p} if Attack > Defense'),
    'LEVEL_ATK_EQ_DEF': ('Attack = Defense', 'at Level {p} if Attack = Defense'),
    'LEVEL_ATK_LT_DEF': ('Attack < Defense', 'at Level {p} if Attack < Defense'),
    'LEVEL_SILCOON': ('random, based on personality', 'at Level {p} (random)'),
    'LEVEL_CASCOON': ('random, based on personality', 'at Level {p} (random)'),
    'LEVEL_NINJASK': ('', 'at Level {p}'),
    'LEVEL_SHEDINJA': ('empty party slot and a Poke Ball in the bag', 'at Level {p}, with an empty party slot and a Poke Ball'),
}


def parse_evolutions():
    src = read('src/data/pokemon/evolution.h')
    evolutions = {}
    for m in re.finditer(r'\[(SPECIES_\w+)\]\s*=\s*(\{.*?\}\}),', src, re.S):
        out = []
        for method, param, into in re.findall(r'\{EVO_(\w+),\s*(\w+),\s*(SPECIES_\w+)\}', m.group(2)):
            conditions, display = EVO_TEXT.get(method, ('', method.replace('_', ' ').lower()))
            out.append({
                'method': method,
                'param': strip('ITEM_', param),
                'into': strip('SPECIES_', into),
                'conditions': conditions.format(p=param),
                'display': display.format(p=param, item=item_name(param)),
            })
        evolutions[m.group(1)] = out
    return evolutions


def parse_level_up():
    pointers = dict(re.findall(r'\[(SPECIES_\w+)\]\s*=\s*(s\w+LevelUpLearnset)',
                               read('src/data/pokemon/level_up_learnset_pointers.h')))
    src = read('src/data/pokemon/level_up_learnsets.h')
    tables = {m.group(1): m.group(2) for m in re.finditer(r'static const u16 (s\w+LevelUpLearnset)\[\] = \{(.*?)LEVEL_UP_END', src, re.S)}
    out = {}
    for sp, var in pointers.items():
        out[sp] = [{'level': int(lv), 'move': mv}
                   for lv, mv in re.findall(r'LEVEL_UP_MOVE\(\s*(\d+),\s*MOVE_(\w+)\)', tables.get(var, ''))]
    return out


def parse_tmhm():
    src = read('src/data/pokemon/tmhm_learnsets.h')
    out = {}
    for m in re.finditer(r'^    \[(SPECIES_\w+)\] = \{ \.learnset = \{\n((?:        .*\n)*)    \} \},', src, re.M):
        out[m.group(1)] = sorted('MOVE_' + tm for tm in re.findall(r'\.(\w+) = TRUE', m.group(2)))
    return out


def main():
    output = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_OUTPUT
    info = parse_species_info()
    names = parse_names()
    dex = parse_national_dex()
    evolutions = parse_evolutions()
    level_up = parse_level_up()
    tmhm = parse_tmhm()

    pokemon = []
    for sp, data in info.items():
        if sp == 'SPECIES_NONE' or sp.startswith('SPECIES_OLD_UNOWN') or sp not in dex:
            continue
        nat_name, nat_num = dex[sp]
        pokemon.append({
            'species': strip('SPECIES_', sp),
            'name': display_name(names.get(sp, strip('SPECIES_', sp))),
            'natDexNum': nat_name,
            'types': data['types'],
            'abilities': data['abilities'],
            'stats': data['stats'],
            'evolutions': evolutions.get(sp, []),
            'levelUpLearnset': level_up.get(sp, []),
            '_dexNum': nat_num,
            'tmLearnset': tmhm.get(sp, []),
            '_sprite': f'sprites/pokemon/{nat_num}.png',
        })
    pokemon.sort(key=lambda p: p['_dexNum'])

    with open(output, 'w', encoding='utf-8', newline='\n') as f:
        json.dump(pokemon, f, ensure_ascii=False, indent=2)
        f.write('\n')
    print(f'{len(pokemon)} Pokemon written to {output}')


if __name__ == '__main__':
    main()
