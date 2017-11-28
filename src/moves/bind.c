#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"
#include "../battle_loop/move_chain_states.h"
#include "../volatile/residual_effects.h"

extern void dprintf(const char * str, ...);
extern u8 add_residual_cb(ResidualFunc to_add);
extern void run_decision(void);
extern u16 rand_range(u16 min, u16 max);
extern void apply_residual_dmg(u8 id);


u8 bind_on_effect_cb(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    u8 defender = TARGET_OF(user);
    // the bind effect timer does not reset upon using bind again
    if (HAS_VOLATILE(defender, VOLATILE_BIND)) {
        return true;
    }
    ADD_VOLATILE(defender, VOLATILE_BIND);
    u8 id = add_residual_cb(apply_residual_dmg);
    residual_callbacks[id].effective_bank = defender;
    residual_callbacks[id].active_counter = rand_range(4, 6);
    residual_callbacks[id].move_id = move;
    residual_callbacks[id].status = VOLATILE_BIND;
    residual_callbacks[id].hp_delta = MAX(1, TOTAL_HP(defender) / 8);
    return true;
}
