module robot_rumble::settlement;

use sui::event;
use sui::table::{Self, Table};
use robot_rumble::credits::{Self, Vault};
use robot_rumble::gear::AdminCap;

// === Errors ===
const E_MATCH_ALREADY_SETTLED: u64 = 1;
const E_MISMATCHED_BATCH_LENGTHS: u64 = 2;
const E_EMPTY_BATCH: u64 = 3;
const E_EXCEEDS_MATCH_BUDGET: u64 = 4;

/// Maximum allowed RRC rewards in a single match settlement (anti-drain safety cap)
const MAX_MATCH_REWARD_BUDGET: u64 = 1000;

// === Shared Settlement Registry ===
public struct SettlementRegistry has key {
    id: UID,
    settled_matches: Table<u64, u64>, // match_id -> total_payout
    total_matches_settled: u64,
    total_rrc_settled: u64,
}

// === Events ===
public struct MatchSettled has copy, drop {
    match_id: u64,
    recipients_count: u64,
    total_payout: u64,
    settler: address,
}

public struct PlayerSettlement has copy, drop {
    match_id: u64,
    recipient: address,
    reward_amount: u64,
}

// === Registry Creation (Callable by Admin) ===
public fun create_registry(_cap: &AdminCap, ctx: &mut TxContext) {
    let registry = SettlementRegistry {
        id: object::new(ctx),
        settled_matches: table::new(ctx),
        total_matches_settled: 0,
        total_rrc_settled: 0,
    };
    transfer::share_object(registry);
}

// === Batched Settlement Entry Function ===
public fun settle_match_batch(
    _cap: &AdminCap,
    registry: &mut SettlementRegistry,
    vault: &mut Vault,
    match_id: u64,
    recipients: vector<address>,
    amounts: vector<u64>,
    ctx: &mut TxContext,
) {
    // 1. Enforce idempotency: match cannot be settled twice
    assert!(!table::contains(&registry.settled_matches, match_id), E_MATCH_ALREADY_SETTLED);

    // 2. Validate batch inputs
    let len = vector::length(&recipients);
    assert!(len > 0, E_EMPTY_BATCH);
    assert!(len == vector::length(&amounts), E_MISMATCHED_BATCH_LENGTHS);

    // 3. Process payouts in a single atomic transaction loop
    let mut i = 0;
    let mut total_payout = 0;
    while (i < len) {
        let recipient = *vector::borrow(&recipients, i);
        let amount = *vector::borrow(&amounts, i);
        if (amount > 0) {
            total_payout = total_payout + amount;
            let coin = credits::mint(vault, amount, ctx);
            transfer::public_transfer(coin, recipient);
            event::emit(PlayerSettlement {
                match_id,
                recipient,
                reward_amount: amount,
            });
        };
        i = i + 1;
    };

    // 4. Enforce match budget limit
    assert!(total_payout <= MAX_MATCH_REWARD_BUDGET, E_EXCEEDS_MATCH_BUDGET);

    // 5. Record settlement
    table::add(&mut registry.settled_matches, match_id, total_payout);
    registry.total_matches_settled = registry.total_matches_settled + 1;
    registry.total_rrc_settled = registry.total_rrc_settled + total_payout;

    event::emit(MatchSettled {
        match_id,
        recipients_count: len,
        total_payout,
        settler: ctx.sender(),
    });
}

// === Public Getters ===
public fun is_match_settled(registry: &SettlementRegistry, match_id: u64): bool {
    table::contains(&registry.settled_matches, match_id)
}

public fun total_matches_settled(registry: &SettlementRegistry): u64 {
    registry.total_matches_settled
}

public fun total_rrc_settled(registry: &SettlementRegistry): u64 {
    registry.total_rrc_settled
}

public fun match_payout(registry: &SettlementRegistry, match_id: u64): u64 {
    *table::borrow(&registry.settled_matches, match_id)
}

#[test_only]
public fun create_registry_for_testing(ctx: &mut TxContext) {
    let registry = SettlementRegistry {
        id: object::new(ctx),
        settled_matches: table::new(ctx),
        total_matches_settled: 0,
        total_rrc_settled: 0,
    };
    transfer::share_object(registry);
}
