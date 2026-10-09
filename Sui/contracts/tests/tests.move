#[test_only]
module robot_rumble::tests;

use std::string;
use sui::test_scenario as ts;
use sui::coin;
use robot_rumble::player::{Self, PlayerProfile};
use robot_rumble::gear::{Self, Weapon, AdminCap};
use robot_rumble::credits::{Self, Vault, CREDITS};
use robot_rumble::battle;

const ADMIN: address = @0xA11CE;
const PLAYER1: address = @0xB0B;

#[test]
fun test_player_create_and_battle() {
    let mut scenario = ts::begin(ADMIN);
    
    // Initialize gear module (mints AdminCap to ADMIN)
    {
        gear::init_for_testing(ts::ctx(&mut scenario));
    };

    // Player creates profile
    ts::next_tx(&mut scenario, PLAYER1);
    {
        player::create_player(ts::ctx(&mut scenario));
    };

    // Verify player profile exists
    ts::next_tx(&mut scenario, PLAYER1);
    {
        let profile = ts::take_from_sender<PlayerProfile>(&scenario);
        assert!(player::owner(&profile) == PLAYER1, 0);
        assert!(player::level(&profile) == 1, 1);
        assert!(player::wins(&profile) == 0, 2);
        assert!(player::xp(&profile) == 0, 3);
        ts::return_to_sender(&scenario, profile);
    };

    // Admin records a battle win with 3 kills
    ts::next_tx(&mut scenario, ADMIN);
    {
        let admin_cap = ts::take_from_sender<AdminCap>(&scenario);
        let mut profile = ts::take_from_address<PlayerProfile>(&scenario, PLAYER1);

        battle::record_battle(&admin_cap, &mut profile, 1001, true, 3);

        assert!(player::wins(&profile) == 1, 4);
        assert!(player::battles_played(&profile) == 1, 5);
        // Base XP 100 + 3*15 = 145 XP. Required for lv 2: 100. New level 2, remainder XP 45!
        assert!(player::level(&profile) == 2, 6);
        assert!(player::xp(&profile) == 45, 7);

        ts::return_to_sender(&scenario, admin_cap);
        ts::return_to_address(PLAYER1, profile);
    };

    ts::end(scenario);
}

#[test]
fun test_weapon_mint_and_upgrade() {
    let mut scenario = ts::begin(ADMIN);
    
    // Initialize gear & credits modules
    {
        gear::init_for_testing(ts::ctx(&mut scenario));
        credits::init_for_testing(ts::ctx(&mut scenario));
    };

    // Admin creates initial loadout weapon for Player1
    ts::next_tx(&mut scenario, ADMIN);
    {
        let admin_cap = ts::take_from_sender<AdminCap>(&scenario);
        gear::mint_weapon_direct(
            &admin_cap,
            string::utf8(b"Pulse Blade"),
            30, // Damage 30
            string::utf8(b"Rare"),
            12345,
            PLAYER1,
            ts::ctx(&mut scenario),
        );
        ts::return_to_sender(&scenario, admin_cap);
    };

    // Check weapon ownership by Player1
    ts::next_tx(&mut scenario, PLAYER1);
    {
        let w = ts::take_from_sender<Weapon>(&scenario);
        assert!(gear::weapon_damage(&w) == 30, 10);
        assert!(gear::weapon_level(&w) == 1, 11);
        ts::return_to_sender(&scenario, w);
    };

    // Vault mints 50 CREDITS to Player1
    ts::next_tx(&mut scenario, ADMIN);
    {
        let mut vault = ts::take_shared<Vault>(&scenario);
        let reward = credits::mint(&mut vault, 50, ts::ctx(&mut scenario));
        sui::transfer::public_transfer(reward, PLAYER1);
        ts::return_shared(vault);
    };

    // Player1 upgrades weapon from Level 1 -> Level 2
    ts::next_tx(&mut scenario, PLAYER1);
    {
        let mut w = ts::take_from_sender<Weapon>(&scenario);
        let mut vault = ts::take_shared<Vault>(&scenario);
        let mut payment = ts::take_from_sender<coin::Coin<CREDITS>>(&scenario);

        // Split 10 CREDITS for upgrade (Level 1 costs 10)
        let cost_coin = coin::split(&mut payment, 10, ts::ctx(&mut scenario));
        gear::upgrade_weapon(&mut w, &mut vault, cost_coin, ts::ctx(&mut scenario));

        // Assert level is now 2, damage is now 38 (30 + 8)
        assert!(gear::weapon_level(&w) == 2, 12);
        assert!(gear::weapon_damage(&w) == 38, 13);
        assert!(coin::value(&payment) == 40, 14); // 50 - 10 = 40 remaining

        ts::return_to_sender(&scenario, w);
        ts::return_to_sender(&scenario, payment);
        ts::return_shared(vault);
    };

    ts::end(scenario);
}

#[test]
#[expected_failure(abort_code = gear::E_INSUFFICIENT_PAYMENT)]
fun test_upgrade_insufficient_payment_fails() {
    let mut scenario = ts::begin(ADMIN);
    
    // Initialize modules
    {
        gear::init_for_testing(ts::ctx(&mut scenario));
        credits::init_for_testing(ts::ctx(&mut scenario));
    };

    // Admin creates weapon
    ts::next_tx(&mut scenario, ADMIN);
    {
        let admin_cap = ts::take_from_sender<AdminCap>(&scenario);
        gear::mint_weapon_direct(
            &admin_cap,
            string::utf8(b"Pulse Blade"),
            30,
            string::utf8(b"Common"),
            12345,
            PLAYER1,
            ts::ctx(&mut scenario),
        );
        ts::return_to_sender(&scenario, admin_cap);
    };

    // Mint only 5 credits (cost is 10)
    ts::next_tx(&mut scenario, ADMIN);
    {
        let mut vault = ts::take_shared<Vault>(&scenario);
        let underpaid = credits::mint(&mut vault, 5, ts::ctx(&mut scenario));
        sui::transfer::public_transfer(underpaid, PLAYER1);
        ts::return_shared(vault);
    };

    // Player1 tries to upgrade with 5 credits -> should abort with E_INSUFFICIENT_PAYMENT!
    ts::next_tx(&mut scenario, PLAYER1);
    {
        let mut w = ts::take_from_sender<Weapon>(&scenario);
        let mut vault = ts::take_shared<Vault>(&scenario);
        let payment = ts::take_from_sender<coin::Coin<CREDITS>>(&scenario);

        gear::upgrade_weapon(&mut w, &mut vault, payment, ts::ctx(&mut scenario));

        ts::return_to_sender(&scenario, w);
        ts::return_shared(vault);
    };

    ts::end(scenario);
}
