module robot_rumble::player;

use sui::event;

// === Structs ===
public struct PlayerProfile has key {
    id: UID,
    owner: address,
    level: u64,
    xp: u64,
    wins: u64,
    losses: u64,
    battles_played: u64,
}

// === Events ===
public struct PlayerCreated has copy, drop {
    player_id: ID,
    owner: address,
}

public struct PlayerProgressed has copy, drop {
    player_id: ID,
    owner: address,
    level: u64,
    xp: u64,
    wins: u64,
    losses: u64,
    battles_played: u64,
}

// === Entry / Public Functions ===
public fun create_player(ctx: &mut TxContext) {
    let sender = ctx.sender();
    let profile = PlayerProfile {
        id: object::new(ctx),
        owner: sender,
        level: 1,
        xp: 0,
        wins: 0,
        losses: 0,
        battles_played: 0,
    };
    let player_id = object::id(&profile);
    transfer::transfer(profile, sender);
    event::emit(PlayerCreated { player_id, owner: sender });
}

// === Package-Internal Functions ===
public(package) fun record_battle_result(
    profile: &mut PlayerProfile,
    won: bool,
    xp_gained: u64,
) {
    profile.battles_played = profile.battles_played + 1;
    if (won) {
        profile.wins = profile.wins + 1;
    } else {
        profile.losses = profile.losses + 1;
    };
    profile.xp = profile.xp + xp_gained;

    // Level-up threshold
    let required_xp = profile.level * 100;
    while (profile.xp >= required_xp) {
        profile.xp = profile.xp - required_xp;
        profile.level = profile.level + 1;
    };

    event::emit(PlayerProgressed {
        player_id: object::id(profile),
        owner: profile.owner,
        level: profile.level,
        xp: profile.xp,
        wins: profile.wins,
        losses: profile.losses,
        battles_played: profile.battles_played,
    });
}

// === Public Getters ===
public fun owner(profile: &PlayerProfile): address { profile.owner }
public fun level(profile: &PlayerProfile): u64 { profile.level }
public fun xp(profile: &PlayerProfile): u64 { profile.xp }
public fun wins(profile: &PlayerProfile): u64 { profile.wins }
public fun losses(profile: &PlayerProfile): u64 { profile.losses }
public fun battles_played(profile: &PlayerProfile): u64 { profile.battles_played }
