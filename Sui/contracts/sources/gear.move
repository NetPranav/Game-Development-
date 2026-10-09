/// Robot Rumble: gear NFTs (Weapon / Armor / Ability), CREDITS reward coin, admin-gated
/// reward minting with on-chain randomness, and credit-funded weapon/armor/ability upgrades.
module robot_rumble::gear;

use std::string::{Self, String};
use robot_rumble::credits::{Self, CREDITS, Vault};
use sui::coin::{Self, Coin};
use sui::event;
use sui::random::{Self, Random};

// === Errors ===
const E_NOT_WON: u64 = 1;
const E_INSUFFICIENT_PAYMENT: u64 = 2;
const E_MAX_LEVEL: u64 = 3;

// === Tunables ===
const WIN_CREDITS: u64 = 50;
const UPGRADE_COST_BASE: u64 = 10;
const UPGRADE_DAMAGE_BONUS: u64 = 8;
const UPGRADE_ARMOR_BONUS: u64 = 6;
const UPGRADE_POWER_BONUS: u64 = 7;
const MAX_UPGRADE_LEVEL: u64 = 20;

const STAT_MIN: u64 = 15;
const STAT_MAX: u64 = 40;

// === Objects ===
public struct Weapon has key, store {
    id: UID,
    name: String,
    weapon_type: String,
    damage: u64,
    level: u64,
    durability: u64,
    wins: u64,
    rarity: String,
    seed: u64,
}

public struct Armor has key, store {
    id: UID,
    name: String,
    armor_type: String,
    armor: u64,
    level: u64,
    durability: u64,
    wins: u64,
    rarity: String,
    seed: u64,
}

public struct Ability has key, store {
    id: UID,
    name: String,
    ability_type: String,
    power: u64,
    level: u64,
    cooldown: u64,
    wins: u64,
    rarity: String,
    seed: u64,
}

/// Admin capability held by game authority to execute verified game actions
public struct AdminCap has key, store { id: UID }

// === Events ===
public struct GearMinted has copy, drop {
    recipient: address,
    kind: u8, // 0: Weapon, 1: Armor, 2: Ability
    gear_id: ID,
    name: String,
    rarity: String,
    stat: u64,
    seed: u64,
}

public struct RewardClaimed has copy, drop {
    recipient: address,
    credits: u64,
    gear_id: ID,
}

public struct GearUpgraded has copy, drop {
    gear_id: ID,
    kind: u8,
    new_level: u64,
    new_stat: u64,
    cost_paid: u64,
}

public struct GearRepaired has copy, drop {
    gear_id: ID,
    new_durability: u64,
}

// === Init ===
fun init(ctx: &mut TxContext) {
    transfer::transfer(AdminCap { id: object::new(ctx) }, ctx.sender());
}

#[test_only]
public fun init_for_testing(ctx: &mut TxContext) {
    init(ctx);
}

// === Reward Minting (Admin-authorized with on-chain VRF randomness) ===
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
    let base_stat = random::generate_u64_in_range(&mut g, STAT_MIN, STAT_MAX);
    let seed = random::generate_u64(&mut g);
    let name_idx = random::generate_u8_in_range(&mut g, 0, 2);
    let rarity_roll = random::generate_u8_in_range(&mut g, 0, 100);

    let (rarity_str, rarity_multiplier) = if (rarity_roll > 90) {
        (string::utf8(b"Legendary"), 20)
    } else if (rarity_roll > 70) {
        (string::utf8(b"Epic"), 12)
    } else if (rarity_roll > 40) {
        (string::utf8(b"Rare"), 6)
    } else {
        (string::utf8(b"Common"), 0)
    };

    let stat = base_stat + rarity_multiplier;

    let gear_id = if (kind == 0) {
        let w_name = weapon_name(name_idx);
        let w = Weapon {
            id: object::new(ctx),
            name: w_name,
            weapon_type: string::utf8(b"Melee"),
            damage: stat,
            level: 1,
            durability: 100,
            wins: 1,
            rarity: rarity_str,
            seed,
        };
        let gid = object::id(&w);
        event::emit(GearMinted {
            recipient: to,
            kind: 0,
            gear_id: gid,
            name: w_name,
            rarity: rarity_str,
            stat,
            seed,
        });
        transfer::public_transfer(w, to);
        gid
    } else if (kind == 1) {
        let a_name = armor_name(name_idx);
        let a = Armor {
            id: object::new(ctx),
            name: a_name,
            armor_type: string::utf8(b"Chassis"),
            armor: stat,
            level: 1,
            durability: 100,
            wins: 1,
            rarity: rarity_str,
            seed,
        };
        let gid = object::id(&a);
        event::emit(GearMinted {
            recipient: to,
            kind: 1,
            gear_id: gid,
            name: a_name,
            rarity: rarity_str,
            stat,
            seed,
        });
        transfer::public_transfer(a, to);
        gid
    } else {
        let ab_name = ability_name(name_idx);
        let b = Ability {
            id: object::new(ctx),
            name: ab_name,
            ability_type: string::utf8(b"System"),
            power: stat,
            level: 1,
            cooldown: 8,
            wins: 1,
            rarity: rarity_str,
            seed,
        };
        let gid = object::id(&b);
        event::emit(GearMinted {
            recipient: to,
            kind: 2,
            gear_id: gid,
            name: ab_name,
            rarity: rarity_str,
            stat,
            seed,
        });
        transfer::public_transfer(b, to);
        gid
    };

    // Mint RRC reward coins to player
    let reward = credits::mint(vault, WIN_CREDITS, ctx);
    transfer::public_transfer(reward, to);

    event::emit(RewardClaimed {
        recipient: to,
        credits: WIN_CREDITS,
        gear_id,
    });
}

// === Direct Admin Minting (Deterministic for Unit Testing & Initial Loadout) ===
public fun mint_weapon_direct(
    _cap: &AdminCap,
    name: String,
    damage: u64,
    rarity: String,
    seed: u64,
    to: address,
    ctx: &mut TxContext,
): ID {
    let w = Weapon {
        id: object::new(ctx),
        name,
        weapon_type: string::utf8(b"Melee"),
        damage,
        level: 1,
        durability: 100,
        wins: 0,
        rarity,
        seed,
    };
    let gid = object::id(&w);
    transfer::public_transfer(w, to);
    gid
}

// === Upgrades (Player-signed, spends and burns RRC credits) ===
public fun upgrade_weapon(
    w: &mut Weapon,
    vault: &mut Vault,
    payment: Coin<CREDITS>,
    _ctx: &mut TxContext,
) {
    assert!(w.level < MAX_UPGRADE_LEVEL, E_MAX_LEVEL);
    let cost = UPGRADE_COST_BASE * w.level;
    assert!(coin::value(&payment) >= cost, E_INSUFFICIENT_PAYMENT);

    // Burn the payment in Vault
    credits::burn(vault, payment);

    w.level = w.level + 1;
    w.damage = w.damage + UPGRADE_DAMAGE_BONUS;

    event::emit(GearUpgraded {
        gear_id: object::id(w),
        kind: 0,
        new_level: w.level,
        new_stat: w.damage,
        cost_paid: cost,
    });
}

public fun upgrade_armor(
    a: &mut Armor,
    vault: &mut Vault,
    payment: Coin<CREDITS>,
    _ctx: &mut TxContext,
) {
    assert!(a.level < MAX_UPGRADE_LEVEL, E_MAX_LEVEL);
    let cost = UPGRADE_COST_BASE * a.level;
    assert!(coin::value(&payment) >= cost, E_INSUFFICIENT_PAYMENT);

    credits::burn(vault, payment);

    a.level = a.level + 1;
    a.armor = a.armor + UPGRADE_ARMOR_BONUS;

    event::emit(GearUpgraded {
        gear_id: object::id(a),
        kind: 1,
        new_level: a.level,
        new_stat: a.armor,
        cost_paid: cost,
    });
}

public fun upgrade_ability(
    b: &mut Ability,
    vault: &mut Vault,
    payment: Coin<CREDITS>,
    _ctx: &mut TxContext,
) {
    assert!(b.level < MAX_UPGRADE_LEVEL, E_MAX_LEVEL);
    let cost = UPGRADE_COST_BASE * b.level;
    assert!(coin::value(&payment) >= cost, E_INSUFFICIENT_PAYMENT);

    credits::burn(vault, payment);

    b.level = b.level + 1;
    b.power = b.power + UPGRADE_POWER_BONUS;

    event::emit(GearUpgraded {
        gear_id: object::id(b),
        kind: 2,
        new_level: b.level,
        new_stat: b.power,
        cost_paid: cost,
    });
}

public fun repair_weapon(
    w: &mut Weapon,
    vault: &mut Vault,
    payment: Coin<CREDITS>,
    _ctx: &mut TxContext,
) {
    let repair_cost = 10;
    assert!(coin::value(&payment) >= repair_cost, E_INSUFFICIENT_PAYMENT);
    credits::burn(vault, payment);
    w.durability = 100;
    event::emit(GearRepaired {
        gear_id: object::id(w),
        new_durability: 100,
    });
}

public fun record_win(_cap: &AdminCap, w: &mut Weapon) {
    w.wins = w.wins + 1;
}

// === Names ===
fun weapon_name(i: u8): String {
    if (i == 0) string::utf8(b"Pulse Blade")
    else if (i == 1) string::utf8(b"Arc Hammer")
    else string::utf8(b"Rail Lance")
}

fun armor_name(i: u8): String {
    if (i == 0) string::utf8(b"Plate Chassis")
    else if (i == 1) string::utf8(b"Nano Weave")
    else string::utf8(b"Bulwark Core")
}

fun ability_name(i: u8): String {
    if (i == 0) string::utf8(b"Overdrive")
    else if (i == 1) string::utf8(b"Shockwave")
    else string::utf8(b"Phase Dash")
}

// === Getters ===
public fun weapon_damage(w: &Weapon): u64 { w.damage }
public fun weapon_level(w: &Weapon): u64 { w.level }
public fun weapon_durability(w: &Weapon): u64 { w.durability }
public fun armor_defense(a: &Armor): u64 { a.armor }
public fun armor_level(a: &Armor): u64 { a.level }
public fun ability_power(b: &Ability): u64 { b.power }
public fun ability_level(b: &Ability): u64 { b.level }
