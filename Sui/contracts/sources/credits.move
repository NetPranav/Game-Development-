/// CREDITS reward currency for Robot Rumble.
module robot_rumble::credits;

use sui::coin::{Self, Coin, TreasuryCap};

public struct CREDITS has drop {}

/// Shared: owns the treasury; only this package can mint/burn through it.
public struct Vault has key {
    id: UID,
    treasury: TreasuryCap<CREDITS>,
}

#[allow(deprecated_usage)]
fun init(otw: CREDITS, ctx: &mut TxContext) {
    let (treasury, metadata) = coin::create_currency(
        otw,
        0,
        b"RRC",
        b"Robot Credits",
        b"Reward currency for Robot Rumble",
        option::none(),
        ctx,
    );
    transfer::public_freeze_object(metadata);
    transfer::share_object(Vault { id: object::new(ctx), treasury });
}

public(package) fun mint(vault: &mut Vault, amount: u64, ctx: &mut TxContext): Coin<CREDITS> {
    coin::mint(&mut vault.treasury, amount, ctx)
}

public(package) fun burn(vault: &mut Vault, c: Coin<CREDITS>): u64 {
    coin::burn(&mut vault.treasury, c)
}

#[test_only]
public fun init_for_testing(ctx: &mut TxContext) {
    init(CREDITS {}, ctx);
}
