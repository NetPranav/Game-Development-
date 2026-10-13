#[test_only]
module robot_rumble::settlement_tests;

use sui::test_scenario as ts;
use sui::coin;
use robot_rumble::settlement::{Self, SettlementRegistry};
use robot_rumble::credits::{Self, Vault, CREDITS};
use robot_rumble::gear::{Self, AdminCap};

const ADMIN: address = @0xA11CE;
const PLAYER1: address = @0xB0B;
const PLAYER2: address = @0xCAFE;

#[test]
fun test_batch_settlement_success() {
    let mut scenario = ts::begin(ADMIN);
    
    // 1. Initialize modules
    {
        gear::init_for_testing(ts::ctx(&mut scenario));
        credits::init_for_testing(ts::ctx(&mut scenario));
        settlement::create_registry_for_testing(ts::ctx(&mut scenario));
    };

    // 2. Admin settles Match #1001 with 2 players in ONE batched transaction
    // Player 1 gets 45 RRC, Player 2 gets 30 RRC
    ts::next_tx(&mut scenario, ADMIN);
    {
        let admin_cap = ts::take_from_sender<AdminCap>(&scenario);
        let mut registry = ts::take_shared<SettlementRegistry>(&scenario);
        let mut vault = ts::take_shared<Vault>(&scenario);

        let recipients = vector[PLAYER1, PLAYER2];
        let amounts = vector[45, 30];

        settlement::settle_match_batch(
            &admin_cap,
            &mut registry,
            &mut vault,
            1001,
            recipients,
            amounts,
            ts::ctx(&mut scenario),
        );

        assert!(settlement::is_match_settled(&registry, 1001), 1);
        assert!(settlement::total_matches_settled(&registry) == 1, 2);
        assert!(settlement::total_rrc_settled(&registry) == 75, 3);
        assert!(settlement::match_payout(&registry, 1001) == 75, 4);

        ts::return_to_sender(&scenario, admin_cap);
        ts::return_shared(registry);
        ts::return_shared(vault);
    };

    // 3. Verify Player 1 received 45 RRC
    ts::next_tx(&mut scenario, PLAYER1);
    {
        let coin = ts::take_from_sender<coin::Coin<CREDITS>>(&scenario);
        assert!(coin::value(&coin) == 45, 10);
        ts::return_to_sender(&scenario, coin);
    };

    // 4. Verify Player 2 received 30 RRC
    ts::next_tx(&mut scenario, PLAYER2);
    {
        let coin = ts::take_from_sender<coin::Coin<CREDITS>>(&scenario);
        assert!(coin::value(&coin) == 30, 20);
        ts::return_to_sender(&scenario, coin);
    };

    ts::end(scenario);
}

#[test]
#[expected_failure(abort_code = settlement::E_MATCH_ALREADY_SETTLED)]
fun test_duplicate_settlement_fails() {
    let mut scenario = ts::begin(ADMIN);
    {
        gear::init_for_testing(ts::ctx(&mut scenario));
        credits::init_for_testing(ts::ctx(&mut scenario));
        settlement::create_registry_for_testing(ts::ctx(&mut scenario));
    };

    // 1st Settlement of Match 2002
    ts::next_tx(&mut scenario, ADMIN);
    {
        let admin_cap = ts::take_from_sender<AdminCap>(&scenario);
        let mut registry = ts::take_shared<SettlementRegistry>(&scenario);
        let mut vault = ts::take_shared<Vault>(&scenario);

        settlement::settle_match_batch(
            &admin_cap,
            &mut registry,
            &mut vault,
            2002,
            vector[PLAYER1],
            vector[50],
            ts::ctx(&mut scenario),
        );

        ts::return_to_sender(&scenario, admin_cap);
        ts::return_shared(registry);
        ts::return_shared(vault);
    };

    // 2nd Duplicate Settlement of Match 2002 -> MUST ABORT!
    ts::next_tx(&mut scenario, ADMIN);
    {
        let admin_cap = ts::take_from_sender<AdminCap>(&scenario);
        let mut registry = ts::take_shared<SettlementRegistry>(&scenario);
        let mut vault = ts::take_shared<Vault>(&scenario);

        settlement::settle_match_batch(
            &admin_cap,
            &mut registry,
            &mut vault,
            2002,
            vector[PLAYER1],
            vector[50],
            ts::ctx(&mut scenario),
        );

        ts::return_to_sender(&scenario, admin_cap);
        ts::return_shared(registry);
        ts::return_shared(vault);
    };

    ts::end(scenario);
}

#[test]
#[expected_failure(abort_code = settlement::E_EXCEEDS_MATCH_BUDGET)]
fun test_exceeds_budget_fails() {
    let mut scenario = ts::begin(ADMIN);
    {
        gear::init_for_testing(ts::ctx(&mut scenario));
        credits::init_for_testing(ts::ctx(&mut scenario));
        settlement::create_registry_for_testing(ts::ctx(&mut scenario));
    };

    // Attempt payout of 1500 RRC (> 1000 MAX_MATCH_REWARD_BUDGET) -> MUST ABORT!
    ts::next_tx(&mut scenario, ADMIN);
    {
        let admin_cap = ts::take_from_sender<AdminCap>(&scenario);
        let mut registry = ts::take_shared<SettlementRegistry>(&scenario);
        let mut vault = ts::take_shared<Vault>(&scenario);

        settlement::settle_match_batch(
            &admin_cap,
            &mut registry,
            &mut vault,
            3003,
            vector[PLAYER1],
            vector[1500],
            ts::ctx(&mut scenario),
        );

        ts::return_to_sender(&scenario, admin_cap);
        ts::return_shared(registry);
        ts::return_shared(vault);
    };

    ts::end(scenario);
}

#[test]
#[expected_failure(abort_code = settlement::E_MISMATCHED_BATCH_LENGTHS)]
fun test_mismatched_batch_lengths_fails() {
    let mut scenario = ts::begin(ADMIN);
    {
        gear::init_for_testing(ts::ctx(&mut scenario));
        credits::init_for_testing(ts::ctx(&mut scenario));
        settlement::create_registry_for_testing(ts::ctx(&mut scenario));
    };

    // 2 recipients, 1 amount -> MUST ABORT!
    ts::next_tx(&mut scenario, ADMIN);
    {
        let admin_cap = ts::take_from_sender<AdminCap>(&scenario);
        let mut registry = ts::take_shared<SettlementRegistry>(&scenario);
        let mut vault = ts::take_shared<Vault>(&scenario);

        settlement::settle_match_batch(
            &admin_cap,
            &mut registry,
            &mut vault,
            4004,
            vector[PLAYER1, PLAYER2],
            vector[50], // Mismatched length!
            ts::ctx(&mut scenario),
        );

        ts::return_to_sender(&scenario, admin_cap);
        ts::return_shared(registry);
        ts::return_shared(vault);
    };

    ts::end(scenario);
}
