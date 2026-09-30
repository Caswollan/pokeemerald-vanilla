#ifndef GUARD_CONSTANTS_TMS_HMS_H
#define GUARD_CONSTANTS_TMS_HMS_H

#define FOREACH_TM(F) \
    F(PIN_MISSILE) \
    F(TWINEEDLE) \
    F(LEECH_LIFE) \
    F(SIGNAL_BEAM) \
    F(TORMENT) \
    F(TAUNT) \
    F(KNOCK_OFF) \
    F(CRUNCH) \
    F(DRAGON_DANCE) \
    F(DRAGON_BREATH) \
    F(DRAGON_CLAW) \
    F(THUNDER_WAVE) \
    F(SHOCK_WAVE) \
    F(THUNDER_PUNCH) \
    F(THUNDERBOLT) \
    F(THUNDER) \
    F(BULK_UP) \
    F(ARM_THRUST) \
    F(MACH_PUNCH) \
    F(BRICK_BREAK) \
    F(SUPERPOWER) \
    F(SUNNY_DAY) \
    F(WILL_O_WISP) \
    F(FIRE_PUNCH) \
    F(FLAMETHROWER) \
    F(HEAT_WAVE) \
    F(FIRE_BLAST) \
    F(OVERHEAT) \
    F(AERIAL_ACE) \
    F(SHADOW_PUNCH) \
    F(SHADOW_BALL) \
    F(BULLET_SEED) \
    F(GIGA_DRAIN) \
    F(EGG_BOMB) \
    F(SOLAR_BEAM) \
    F(DIG) \
    F(EARTHQUAKE) \
    F(HAIL) \
    F(ICICLE_SPEAR) \
    F(ICY_WIND) \
    F(ICE_PUNCH) \
    F(ICE_BEAM) \
    F(BLIZZARD) \
    F(SWORDS_DANCE) \
    F(DOUBLE_TEAM) \
    F(PROTECT) \
    F(SAFEGUARD) \
    F(RETURN) \
    F(WEATHER_BALL) \
    F(FACADE) \
    F(SECRET_POWER) \
    F(HYPER_VOICE) \
    F(HYPER_BEAM) \
    F(TOXIC) \
    F(POISON_FANG) \
    F(SLUDGE_BOMB) \
    F(AGILITY) \
    F(LIGHT_SCREEN) \
    F(REFLECT) \
    F(REST) \
    F(CALM_MIND) \
    F(PSYCHIC) \
    F(SANDSTORM) \
    F(ROCK_BLAST) \
    F(ROCK_TOMB) \
    F(ROCK_SLIDE) \
    F(STEEL_WING) \
    F(IRON_TAIL) \
    F(RAIN_DANCE) \
    F(HYDRO_PUMP)

#define FOREACH_HM(F) \
    F(CUT) \
    F(FLY) \
    F(SURF) \
    F(STRENGTH) \
    F(FLASH) \
    F(ROCK_SMASH) \
    F(WATERFALL) \
    F(DIVE)

#define FOREACH_TMHM(F) \
    FOREACH_TM(F) \
    FOREACH_HM(F)

#endif
