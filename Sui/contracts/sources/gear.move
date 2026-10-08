/// Robot Rumble: gear NFTs (Weapon / Armor / Ability), CREDITS reward coin, admin-gated
/// reward minting with on-chain randomness, and credit-funded weapon upgrades.
module robot_rumble::gear;

use std::string::{Self, String};
use robot_rumble::credits::{Self, CREDITS, Vault};
use sui::coin::{Self, Coin};
use sui::event;
use sui::random::{Self, Random};

// === Errors ===
const E_NOT_WON: u64 = 1;
const E_INSUFFICIENT: u64 = 2;

// === Tunables ===
const WIN_CREDITS: u64 = 50;
const UPGRADE_COST_PER_LEVEL: u64 = 10;
const UPGRADE_DAMAGE_BONUS: u64 = 5;
const STAT_MIN: u64 = 10;
const STAT_MAX: u64 = 30;

// === Objects ===
public struct Weapon has key, store {
    id: UID,
    name: String,
    seed: u64,
    damage: u64,
    level: u64,
    wins: u64,
}

public struct Armor has key, store {
    id: UID,
    name: String,
    seed: u64,
    armor: u64,
    level: u64,
    wins: u64,
}

public struct Ability has key, store {
    id: UID,
    name: String,
    seed: u64,
    power: u64,
    level: u64,
    wins: u64,
}

/// Held by the backend; required to mint rewards.
public struct AdminCap has key { id: UID }

// === Events ===
public struct RewardMinted has copy, drop {
    to: address,
    kind: u8,
    gear_id: ID,
    seed: u64,
    stat: u64,
    credits: u64,
}

public struct WeaponUpgraded has copy, drop { weapon_id: ID, new_level: u64, new_damage: u64 }

// === Init ===
fun init(ctx: &mut TxContext) {
    transfer::transfer(AdminCap { id: object::new(ctx) }, ctx.sender());
}

// === Reward (admin only, randomness) ===
/// Must be `entry` and non-public: `Random` consumers may not be composable.
entry fun mint_reward(
    _cap: &AdminCap,
    vault: &mut Vault,
    r: &Random,
    to: address,
    won: bool,
    ctx: &mut TxContext,
) {
    assert!(won, E_NOT_WON);
    let mut g = random::new_generator(r, ctx);
    let kind = random::generate_u8_in_range(&mut g, 0, 2);
    let stat = random::generate_u64_in_range(&mut g, STAT_MIN, STAT_MAX);
    let seed = random::generate_u64(&mut g);
    let name_idx = random::generate_u8_in_range(&mut g, 0, 2);

    let gear_id = if (kind == 0) {
        let w = Weapon { id: object::new(ctx), name: weapon_name(name_idx), seed, damage: stat, level: 1, wins: 0 };
        let i = object::id(&w);
        transfer::public_transfer(w, to);
        i
    } else if (kind == 1) {
        let a = Armor { id: object::new(ctx), name: armor_name(name_idx), seed, armor: stat, level: 1, wins: 0 };
        let i = object::id(&a);
        transfer::public_transfer(a, to);
        i
    } else {
        let b = Ability { id: object::new(ctx), name: ability_name(name_idx), seed, power: stat, level: 1, wins: 0 };
        let i = object::id(&b);
        transfer::public_transfer(b, to);
        i
    };

    let reward = credits::mint(vault, WIN_CREDITS, ctx);
    transfer::public_transfer(reward, to);

    event::emit(RewardMinted { to, kind, gear_id, seed, stat, credits: WIN_CREDITS });
}

// === Upgrade (player-signed, spends credits) ===
public fun upgrade_weapon(
    w: &mut Weapon,
    vault: &mut Vault,
    payment: Coin<CREDITS>,
    _ctx: &mut TxContext,
) {
    let cost = UPGRADE_COST_PER_LEVEL * w.level;
    assert!(coin::value(&payment) >= cost, E_INSUFFICIENT);
    // Burn the whole payment: callers should split exact change first.
    credits::burn(vault, payment);
    w.level = w.level + 1;
    w.damage = w.damage + UPGRADE_DAMAGE_BONUS;
    event::emit(WeaponUpgraded { weapon_id: object::id(w), new_level: w.level, new_damage: w.damage });
}

/// Record a win on a weapon (admin-gated so wins can't be self-inflated).
public fun record_win(_cap: &AdminCap, w: &mut Weapon) {
    w.wins = w.wins + 1;
}

// === Names ===
fun weapon_name(i: u8): String {
    if (i == 0) string::utf8(b"Pulse Blade") else if (i == 1) string::utf8(b"Arc Hammer") else string::utf8(b"Rail Lance")
}
fun armor_name(i: u8): String {
    if (i == 0) string::utf8(b"Plate Chassis") else if (i == 1) string::utf8(b"Nano Weave") else string::utf8(b"Bulwark Core")
}
fun ability_name(i: u8): String {
    if (i == 0) string::utf8(b"Overdrive") else if (i == 1) string::utf8(b"Shockwave") else string::utf8(b"Phase Dash")
}
