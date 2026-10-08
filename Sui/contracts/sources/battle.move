module robot_rumble::battle;

use sui::event;
use robot_rumble::gear::AdminCap;
use robot_rumble::player::{Self, PlayerProfile};

// === Events ===
public struct BattleCompleted has copy, drop {
    battle_id: u64,
    player: address,
    won: bool,
    enemies_killed: u64,
    xp_gained: u64,
}

// === Authorized Game Authority Functions ===
public fun record_battle(
    _cap: &AdminCap,
    profile: &mut PlayerProfile,
    battle_id: u64,
    won: bool,
    enemies_killed: u64,
) {
    let base_xp = if (won) 100 else 25;
    let xp_gained = base_xp + (enemies_killed * 15);

    player::record_battle_result(profile, won, xp_gained);

    event::emit(BattleCompleted {
        battle_id,
        player: player::owner(profile),
        won,
        enemies_killed,
        xp_gained,
    });
}
