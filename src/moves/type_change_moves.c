#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"

extern void dprintf(const char * str, ...);
extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);
extern bool b_pkmn_has_type(u8 bank, enum PokemonType type);
extern bool b_pkmn_set_type(u8 bank, enum PokemonType type);
extern bool b_pkmn_add_type(u8 bank, enum PokemonType type);
extern u16 rand_range(u16, u16);
extern bool add_anon_cb(u8 CB_id, s8 priority, u8 delay, u8 dur, u8 src, u32 func);

/* Conversion */
u8 conversion_on_effect(u8 attacker, u8 defender, u16 move)
{
    // get type of move in first slot
    u16 first_move = pokemon_getattr(B_PKMN(attacker), REQUEST_MOVE1, NULL);
    u8 type = moves[first_move].type;
    if (b_pkmn_has_type(attacker, type)) {
        return false;
    }

    // X transformed\ninto the Y type!
    enqueue_message(move, attacker, STRING_CONVERSION_TYPE, type);
    return b_pkmn_set_type(attacker, type);
}


/* Conversion 2 */
u8 conversion_two_on_effect(u8 attacker, u8 defender, u16 move)
{
    u8 last_type = MOVE_TYPE(LAST_MOVE(TARGET_OF(attacker)));
    u8 possible_types[MTYPE_NONE];
    memset(&possible_types, MTYPE_NONE, MTYPE_NONE);
    if (last_type == MTYPE_EGG)
        return false;
    u8 array_index = 0;
    for (u8 i = 0; i < MTYPES_MAX; i++) {
        if (b_pkmn_has_type(attacker, i))
            continue;
        u16 effectiveness = MOVE_EFFECTIVENESS(i, last_type);
        if (effectiveness < 100) {
            possible_types[array_index] = i;
            array_index++;
        }
    }
    u8 type = possible_types[rand_range(0, array_index)];
    enqueue_message(move, attacker, STRING_CONVERSION_TYPE, type);
    return b_pkmn_set_type(attacker, type);
}


/* Reflect type */
u8 reflect_type_on_effect(u8 attacker, u8 defender, u16 move)
{
    u8 types[3];
    types[0] = B_PKMN_TYPE(defender, 0);
    types[1] = B_PKMN_TYPE(defender, 1);
    types[2] = B_PKMN_TYPE(defender, 2);
    // fail if target lost it's types
    if ((types[0] == MTYPE_EGG) && (types[1] == MTYPE_EGG) && (types[2] == MTYPE_EGG)) {
        return false;
    }
    // fail if arceus or silvally
    u16 species = p_bank[attacker]->b_data.species;
    if ((species == SPECIES_ARCEUS) || (species == SPECIES_SILVALLY)) {
        return false;
    }

    if ((types[0] == MTYPE_EGG) && (types[1] == MTYPE_EGG) && (types[2] != MTYPE_EGG)) {
        B_PKMN_TYPE(attacker, 0) = MTYPE_NORMAL;
        B_PKMN_TYPE(attacker, 1) = MTYPE_EGG;
        B_PKMN_TYPE(attacker, 2) = types[2];
    } else {
        B_PKMN_TYPE(attacker, 0) = types[0];
        B_PKMN_TYPE(attacker, 1) = types[1];
        B_PKMN_TYPE(attacker, 2) = types[2];
    }

    //X's type\nchanged to match the Foe Y's!
    enqueue_message(0, attacker, STRING_REFLECT_TYPE_MATCHED, 0);
    return true;
}


/* Soak */
u8 soak_on_effect(u8 attacker, u8 defender, u16 move)
{
    if (b_pkmn_set_type(attacker, MTYPE_WATER))
        enqueue_message(move, attacker, STRING_CONVERSION_TYPE, MTYPE_WATER);
    else
        return false;
    return true;
}


/* Flying Press */
u8 flying_press_on_modify_move(u8 attacker, u8 defender, u16 move)
{
    B_MOVE_TYPE(attacker, 1) = MTYPE_FLYING;
    if (HAS_VOLATILE(defender, VOLATILE_MINIMIZE)) {
        B_MOVE_ACCURACY(attacker) = 101;
    }
    return true;
}

u8 flying_press_on_base_power(u8 base_power, u8 user, u8 target, u16 move)
{
    if (HAS_VOLATILE(target, VOLATILE_MINIMIZE)) {
        if ((base_power << 1) < base_power) {
            return 255;
        } else {
            return base_power << 1;
        }
    }
    return base_power;
}


/* Trick-or-Treat */
u8 trick_or_treat_on_effect(u8 attacker, u8 defender, u16 move)
{
    // The Type type was added to\nthe foe defender!
    if (b_pkmn_has_type(defender, MTYPE_GHOST))
        return false;
    B_PKMN_TYPE(defender, 2) = MTYPE_GHOST;
    enqueue_message(move, attacker, STRING_TYPE_ADDED, MTYPE_GHOST);
    return true;
}


/* Ion Deluge */
u8 ion_deluge_on_modify_move_anon(u8 attacker, u8 source, u16 move, struct anonymous_callback* acb)
{
    if (B_MOVE_TYPE(attacker, 0) == MTYPE_NORMAL) {
        B_MOVE_TYPE(attacker, 0) = MTYPE_ELECTRIC;
    } else if (B_MOVE_TYPE(attacker, 1) == MTYPE_NORMAL) {
        B_MOVE_TYPE(attacker, 0) = MTYPE_ELECTRIC;
    }
    return true;
}

u8 ion_deluge_on_effect(u8 attacker, u8 defender, u16 move)
{
    // a deluge of ions showers the battlefield!
    enqueue_message(move, attacker, STRING_ION_DELUGE, 0);
    return add_anon_cb(CB_ON_TRYHIT_MOVE, -2, 0, 0, attacker, (u32)(ion_deluge_on_modify_move_anon));
}


/* Forest's Curse */
u8 forests_curse_on_effect(u8 attacker, u8 defender, u16 move)
{
    // The Type type was added to\nthe foe defender!
    if (b_pkmn_has_type(defender, MTYPE_GRASS))
        return false;
    B_PKMN_TYPE(defender, 2) = MTYPE_GRASS;
    enqueue_message(move, attacker, STRING_TYPE_ADDED, MTYPE_GRASS);
    return true;
}


/* Electrify */
u8 electrify_on_tryhit(u8 attacker, u8 defender, u16 move)
{
    return (attacker == battle_master->first_bank);
}

u8 electrify_on_modify_move_anon(u8 attacker, u8 source, u16 move, struct anonymous_callback* acb)
{
    if (attacker == TARGET_OF(source)) {
        B_MOVE_TYPE(attacker, 0) = MTYPE_ELECTRIC;
        acb->in_use = false;
    }
    return true;
}

u8 electrify_on_effect(u8 attacker, u8 defender, u16 move)
{
    enqueue_message(move, attacker, STRING_ELECTRIFIED, MTYPE_GRASS);
    return add_anon_cb(CB_ON_TRYHIT_MOVE, -2, 0, 0, attacker, (u32)(electrify_on_modify_move_anon));
}


/* Burn up */
u8 burn_up_on_tryhit(u8 attacker, u8 defender, u16 move)
{
    return b_pkmn_has_type(attacker, MTYPE_FIRE);
}

u8 burn_up_on_effect(u8 attacker, u8 defender, u16 move)
{
    for (u8 i = 0; i < sizeof(p_bank[attacker]->b_data.type); i++) {
        if (B_PKMN_TYPE(attacker, i) == MTYPE_FIRE) {
            B_PKMN_TYPE(attacker, i) = MTYPE_EGG;
        }
    }
    return true;
}
