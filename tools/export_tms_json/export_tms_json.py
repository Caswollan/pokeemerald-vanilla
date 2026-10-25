#!/usr/bin/env python3
"""Exports every TM/HM of the ROM with all the ways to obtain it, for the local website.

Reads straight from the source files:
  include/constants/tms_hms.h                       TM/HM numbers
  data/maps/*/map.json                              item balls and hidden items (with map coordinates)
  data/scripts/item_ball_scripts.inc                which item each item ball gives
  data/maps/*/scripts.inc, data/scripts/*.inc       gifts (giveitem / givemon), shops (.2byte lists)
  data/maps/MauvilleCity_GameCorner/scripts.inc     Game Corner prizes and coin prices
  src/data/lilycove_lady.h                          Quiz Lady prizes
  src/data/items.h                                  shop prices
Place names, NPC notes and the story phase of each place can't be read from the game: PLACES, NOTES and
PHASE_RULES below hold them. Keep them in sync when a TM is moved in the hack.

Usage (from the repository root):
  python3 tools/export_tms_json/export_tms_json.py [output.json]
The default output is tools/export_tms_json/tms.json.
"""
import json
import os
import re
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
DEFAULT_OUTPUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'tms.json')

# Readable names of maps and scripts (the others are built from the map name)
PLACES = {
    'Route110_TrickHouseEnd': 'Route 110 · Trick House', 'Route110_TrickHouseEntrance': 'Route 110 · Trick House',
    'secret_power_tm.inc': 'Route 111', 'SSTidalRooms': 'S.S. Tidal · cabins',
    'AbandonedShip_HiddenFloorRooms': 'Abandoned Ship · hidden floor', 'MeteorFalls_B1F_2R': 'Meteor Falls B1F · room 2',
    'MeteorFalls_1F_1R': 'Meteor Falls 1F · room 1', 'MtPyre_Exterior': 'Mt. Pyre · exterior', 'MtPyre_6F': 'Mt. Pyre 6F',
    'SafariZone_Northwest': 'Safari Zone · northwest', 'SeafloorCavern_Room9': 'Seafloor Cavern · room 9',
    'ShoalCave_LowTideIceRoom': 'Shoal Cave · ice room (low tide)', 'ScorchedSlab': 'Scorched Slab (Route 120)',
    'GraniteCave_StevensRoom': "Granite Cave · Steven's room", 'GraniteCave_1F': 'Granite Cave 1F',
    'LilycoveCity_DepartmentStore_4F': 'Lilycove Dept. Store 4F', 'SlateportCity': 'Slateport City · market',
    'MauvilleCity_GameCorner': 'Mauville Game Corner', 'RustboroCity_CuttersHouse': "Rustboro City · Cutter's house",
    'PetalburgCity_WallysHouse': "Petalburg City · Wally's house", 'MossdeepCity_StevensHouse': "Mossdeep City · Steven's house",
    'FallarborTown_CozmosHouse': "Fallarbor Town · Cozmo's house", 'Route114_FossilManiacsHouse': "Route 114 · Fossil Maniac's house",
    'FortreeCity_Mart': 'Fortree City · Poké Mart', 'FortreeCity_House2': 'Fortree City · house 2',
    'LilycoveCity_House2': 'Lilycove City · house 2', 'SootopolisCity_House1': 'Sootopolis City · house 1',
    'MauvilleCity_House1': 'Mauville City · house 1', 'SlateportCity_OceanicMuseum_1F': 'Slateport City · Oceanic Museum 1F',
    'SlateportCity_BattleTentLobby': 'Slateport City · Battle Tent', 'VerdanturfTown_BattleTentLobby': 'Verdanturf Town · Battle Tent',
    'Route119_WeatherInstitute_2F': 'Route 119 · Weather Institute 2F',
    'MagmaHideout_1F': 'Magma Hideout 1F', 'VictoryRoad_B1F': 'Victory Road B1F', 'RusturfTunnel': 'Rusturf Tunnel',
    'QUIZ_LADY': 'Lilycove City · Pokémon Center',
}
GYMS = {'RustboroCity_Gym': ('Rustboro City Gym', 'Roxanne', 1), 'DewfordTown_Gym': ('Dewford Town Gym', 'Brawly', 2),
        'MauvilleCity_Gym': ('Mauville City Gym', 'Wattson', 3), 'LavaridgeTown_Gym_1F': ('Lavaridge Town Gym', 'Flannery', 4),
        'PetalburgCity_Gym': ('Petalburg City Gym', 'Norman', 5), 'FortreeCity_Gym': ('Fortree City Gym', 'Winona', 6),
        'MossdeepCity_Gym': ('Mossdeep City Gym', 'Tate & Liza', 7), 'SootopolisCity_Gym_1F': ('Sootopolis City Gym', 'Wallace', 8)}
ORDINALS = ['1st', '2nd', '3rd', '4th', '5th', '6th', '7th', '8th']

# 'TMxx|map or script file': note shown under the place
NOTES = {
    'TM05|SlateportCity_OceanicMuseum_1F': 'The Team Aqua Grunt who recognizes you on the ground floor.',
    'TM07|SlateportCity_BattleTentLobby': 'NPC in the Battle Tent lobby.',
    'TM08|Route110_TrickHouseEnd': 'Reward for clearing the 5th Trick House puzzle.',
    'TM12|Route110_TrickHouseEnd': 'Reward for clearing the 2nd Trick House puzzle (with Fire Punch and Ice Punch).',
    'TM22|Route110_TrickHouseEnd': 'Reward for clearing the 2nd Trick House puzzle (with ThunderPunch and Ice Punch).',
    'TM44|Route110_TrickHouseEnd': 'Reward for clearing the 2nd Trick House puzzle (with ThunderPunch and Fire Punch).',
    'TM15|MauvilleCity': 'Wattson, after you shut down the New Mauville generator.',
    'TM20|SootopolisCity_House1': 'The Black Belt in the house.',
    'TM36|Route123': 'The girl who gives it if you have a Grass-type Pokémon in your party.',
    'TM39|FortreeCity_House2': 'The old woman gives it as soon as you talk to her.',
    'TM40|Route114_FossilManiacsHouse': "The Fossil Maniac's brother.",
    'TM46|VerdanturfTown_BattleTentLobby': 'The girl in the Battle Tent lobby.',
    'TM50|FallarborTown_CozmosHouse': 'Bring the Meteorite back to Prof. Cozmo.',
    'TM52|secret_power_tm.inc': 'The NPC who explains Secret Bases.',
    'TM53|Route119_WeatherInstitute_2F': 'Held by the Castform you receive after saving the Weather Institute.',
    'TM67|LilycoveCity_House2': 'The man in the house.',
    'TM71|Route114': 'The gentleman on the route.',
    'TM72|Route111': 'In the desert (Go-Goggles needed).',
    'TM73|Route111': 'In the desert (Go-Goggles needed).',
    'TM74|GraniteCave_StevensRoom': 'Steven, when you deliver the letter.',
    'TM79|AbandonedShip_HiddenFloorRooms': 'In one of the locked rooms (Dive and Storage Key needed).',
    'TM28|MtPyre_Exterior': 'In front of the grave at the end of the corridor.',
    'TM48|ShoalCave_LowTideIceRoom': 'Reachable at low tide.',
    'TM49|ShoalCave_LowTideIceRoom': 'Reachable at low tide.',
    'TM45|Route125': 'Surf needed.',
    'TM38|SafariZone_Northwest': 'Mach Bike needed.',
    'TM09|MeteorFalls_B1F_2R': 'Waterfall needed.',
    'TM24|FieryPath': 'Strength needed.',
    'TM61|FieryPath': 'Strength needed.',
    'HM01|RustboroCity_CuttersHouse': 'The Cutter.',
    'HM02|Route119': 'Your rival, after the battle near the Weather Institute.',
    'HM03|PetalburgCity_WallysHouse': "Wally's father, after the 5th badge.",
    'HM04|RusturfTunnel': "Wanda's boyfriend, once the tunnel is open.",
    'HM05|GraniteCave_1F': 'The Hiker at the cave entrance.',
    'HM06|MauvilleCity_House1': 'The man in the first house of Mauville.',
    'HM07|SootopolisCity': 'Wallace, after Rayquaza wakes up.',
    'HM08|MossdeepCity_StevensHouse': 'Steven, after the 7th badge.',
}

# Story phases, in order. A place belongs to the first rule whose keywords appear in its name.
PHASES = ['Start, before Roxanne', 'Rustboro City (1st badge)', 'Dewford Town & Granite Cave (2nd badge)', 'Slateport City',
          'Mauville City & Route 110 (3rd badge)', 'Towards Lavaridge (Routes 111-115, Meteor Falls)',
          'Lavaridge Town (4th badge, Strength, Go-Goggles)', 'Petalburg City (5th badge, Surf)', 'Fortree City (6th badge)',
          'Lilycove City & Mt. Pyre', 'Magma Hideout & Mossdeep City (7th badge, Dive)',
          'Sootopolis City (8th badge, Waterfall) & Victory Road', 'Post-game']
PHASE_RULES = [(1, ['Route 104']), (2, ['Rustboro', 'Rusturf']), (3, ['Dewford', 'Granite Cave']), (4, ['Slateport']),
               (5, ['Mauville', 'Verdanturf', 'Route 117']), (7, ['Lavaridge', 'Fiery Path', 'Route 111']),
               (6, ['Route 113', 'Route 114', 'Route 115', 'Meteor Falls 1F', 'Fallarbor']),
               (8, ['Petalburg', 'Route 119', 'Route 123', 'Pacifidlog']), (9, ['Fortree', 'Scorched Slab']),
               (10, ['Lilycove', 'Safari', 'Mt. Pyre']),
               (11, ['Magma Hideout', 'Mossdeep', 'Route 125', 'Shoal Cave', 'hidden floor', 'Seafloor']),
               (12, ['Sootopolis', 'Meteor Falls B1F', 'Victory Road'])]


def read(path):
    with open(os.path.join(ROOT, path), encoding='utf-8') as f:
        return f.read()


def pretty(name):
    parts = [re.sub(r'(?<=[a-z])(?=[A-Z0-9])', ' ', p) for p in name.split('_')]
    out = parts[0]
    for p in parts[1:]:
        out += (' ' if re.fullmatch(r'B?\d+F|\d+R', p) else ' · ') + p
    return out


def phase(tm, s):
    """Story phase (1-13) from which this source is available."""
    place, key = s['place'], s['key']
    if place.startswith('S.S. Tidal'):
        return 13
    if 'Trick House' in place:
        return 9 if tm == 'TM08' else 5
    if key == 'MauvilleCity':
        return 8          # Wattson's New Mauville request needs Surf
    if key == 'MauvilleCity_GameCorner':
        return 7          # the Game Corner only opens after Flannery (4th badge)
    if key == 'secret_power_tm.inc':
        return 6
    for n, keys in PHASE_RULES:
        if any(k in place for k in keys):
            return n
    raise ValueError(f'No story phase for {tm} at {place}: add it to PHASE_RULES')


def main():
    output = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_OUTPUT
    text = read('include/constants/tms_hms.h')
    numbers = {}
    for macro, prefix in (('FOREACH_TM', 'TM'), ('FOREACH_HM', 'HM')):
        body = re.search(r'#define %s\(F\)(.*?)\n\n' % macro, text, re.S).group(1)
        for i, move in enumerate(re.findall(r'F\((\w+)\)', body), 1):
            numbers[move] = f'{prefix}{i:02d}'
    prices = {m.group(1): int(m.group(2))
              for m in re.finditer(r'\[ITEM_(?:TM|HM)_(\w+)\] =\s*\{[^}]*?\.price = (\d+)', read('src/data/items.h'))}

    found = {}

    def add(move, how, key, coords=None, cost=None, note=''):
        if move not in numbers:
            return
        tm = numbers[move]
        place = PLACES.get(key, pretty(key))
        if key in GYMS:
            place, leader, badge = GYMS[key]
            note = f'{leader}, after the battle ({ORDINALS[badge - 1]} badge).'
        sources = found.setdefault(tm, [])
        if any(s['how'] == how and s['place'] == place for s in sources):
            return
        sources.append({'how': how, 'place': place, 'key': key, 'coords': coords, 'cost': cost,
                        'note': NOTES.get(f'{tm}|{key}', note)})

    balls = dict(re.findall(r'^(\w+)::\n\tfinditem ITEM_(?:TM|HM)_(\w+)', read('data/scripts/item_ball_scripts.inc'), re.M))
    maps_dir = os.path.join(ROOT, 'data', 'maps')
    for mp in sorted(os.listdir(maps_dir)):
        path = os.path.join(maps_dir, mp, 'map.json')
        if not os.path.exists(path):
            continue
        with open(path, encoding='utf-8') as f:
            m = json.load(f)
        for o in m.get('object_events') or []:
            if o.get('script') in balls:
                add(balls[o['script']], 'Item ball', mp, [o['x'], o['y']])
        for b in m.get('bg_events') or []:
            h = re.match(r'ITEM_(?:TM|HM)_(\w+)$', b.get('item', '')) if b.get('type') == 'hidden_item' else None
            if h:
                add(h.group(1), 'Hidden item', mp, [b['x'], b['y']])

    coins = dict(re.findall(r'\.set TM_(\w+)_COINS, (\d+)', read('data/maps/MauvilleCity_GameCorner/scripts.inc')))
    files = [(f'data/maps/{d}/scripts.inc', d) for d in sorted(os.listdir(maps_dir))
             if os.path.exists(os.path.join(maps_dir, d, 'scripts.inc'))]
    files += [(f'data/scripts/{f}', f) for f in sorted(os.listdir(os.path.join(ROOT, 'data', 'scripts')))
              if f != 'item_ball_scripts.inc']
    for path, key in files:
        t = read(path)
        for mv in re.findall(r'\bgiveitem ITEM_(?:TM|HM)_(\w+)', t):
            add(mv, 'Gym reward' if key in GYMS else 'Gift', key, note='Given by an NPC.')
        for mv in re.findall(r'\bgivemon \w+, \d+, ITEM_TM_(\w+)', t):
            add(mv, 'Gift', key)
        for mv in re.findall(r'\.2byte ITEM_(?:TM|HM)_(\w+)', t):
            add(mv, 'Shop', key, cost='₽{:,}'.format(prices.get(mv, 0)))
        if key == 'MauvilleCity_GameCorner':
            for mv in re.findall(r'setvar VAR_0x8004, ITEM_TM_(\w+)', t):
                add(mv, 'Game Corner', key, cost='{:,} coins'.format(int(coins[mv])), note='TM prize counter.')
    quiz = re.search(r'sQuizLadyPrizes\[\] =\s*\{(.*?)\};', read('src/data/lilycove_lady.h'), re.S).group(1)
    for mv in re.findall(r'ITEM_TM_(\w+)', quiz):
        add(mv, 'Prize', 'QUIZ_LADY', note="One of the Quiz Lady's possible prizes.")

    # order in which they can be obtained: earliest story phase, then TMs before HMs, then number
    earliest = {tm: min((phase(tm, s) for s in found.get(tm, [])), default=None) for tm in numbers.values()}
    ranked = sorted(numbers.values(), key=lambda tm: (earliest[tm] or 99, tm.startswith('HM'), tm))
    tms = []
    for move, tm in numbers.items():
        first = earliest[tm]
        tms.append({
            'tm': tm,
            'move': move,
            'order': ranked.index(tm) + 1,
            'phase': first,
            'phaseName': PHASES[first - 1] if first else None,
            'sources': [{k: v for k, v in s.items() if k != 'key'} for s in found.get(tm, [])],
        })

    with open(output, 'w', encoding='utf-8', newline='\n') as out:
        json.dump(tms, out, ensure_ascii=False, indent=2)
        out.write('\n')
    missing = [t['tm'] for t in tms if not t['sources']]
    print(f'{len(tms)} TM/HM written to {output}')
    if missing:
        print('TM/HM with no way to obtain them:', ', '.join(missing))


if __name__ == '__main__':
    main()
