#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"

extern void dprintf(const char * str, ...);
extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);

u8 clear_smog_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (src != user) return true;
    u8 defender = TARGET_OF(user);
    p_bank[defender]->b_data.attack = 0;
    p_bank[defender]->b_data.defense = 0;
    p_bank[defender]->b_data.speed = 0;
    p_bank[defender]->b_data.sp_atk = 0;
    p_bank[defender]->b_data.sp_def = 0;
    p_bank[defender]->b_data.accuracy = 0;
    p_bank[defender]->b_data.evasion = 0;
    p_bank[defender]->b_data.crit_mod = 0;
    enqueue_message(move, user, STRING_CLEAR_SMOG, 0);
    return true;
}
