module robot_rumble::marketplace;

use sui::coin::{Self, Coin};
use sui::event;
use robot_rumble::credits::CREDITS;
use robot_rumble::gear::Weapon;

// === Errors ===
const E_NOT_SELLER: u64 = 1;
const E_INSUFFICIENT_PAYMENT: u64 = 2;

// === Structs ===
public struct Listing has key, store {
    id: UID,
    seller: address,
    price: u64,
    weapon: Weapon,
}

// === Events ===
public struct GearListed has copy, drop {
    listing_id: ID,
    seller: address,
    weapon_id: ID,
    price: u64,
}

public struct GearPurchased has copy, drop {
    listing_id: ID,
    buyer: address,
    seller: address,
    weapon_id: ID,
    price: u64,
}

public struct ListingCancelled has copy, drop {
    listing_id: ID,
    seller: address,
    weapon_id: ID,
}

// === Functions ===
public fun list_weapon(weapon: Weapon, price: u64, ctx: &mut TxContext) {
    let seller = ctx.sender();
    let weapon_id = object::id(&weapon);
    let listing = Listing {
        id: object::new(ctx),
        seller,
        price,
        weapon,
    };
    let listing_id = object::id(&listing);
    transfer::share_object(listing);

    event::emit(GearListed {
        listing_id,
        seller,
        weapon_id,
        price,
    });
}

#[allow(lint(self_transfer))]
public fun buy_weapon(
    listing: Listing,
    payment: Coin<CREDITS>,
    ctx: &mut TxContext,
) {
    let Listing { id, seller, price, weapon } = listing;
    let listing_id = object::uid_to_inner(&id);
    let weapon_id = object::id(&weapon);
    let buyer = ctx.sender();

    assert!(coin::value(&payment) >= price, E_INSUFFICIENT_PAYMENT);

    // Transfer payment to seller
    transfer::public_transfer(payment, seller);
    // Transfer weapon to buyer
    transfer::public_transfer(weapon, buyer);

    object::delete(id);

    event::emit(GearPurchased {
        listing_id,
        buyer,
        seller,
        weapon_id,
        price,
    });
}

#[allow(lint(self_transfer))]
public fun cancel_listing(listing: Listing, ctx: &mut TxContext) {
    let Listing { id, seller, price: _, weapon } = listing;
    assert!(ctx.sender() == seller, E_NOT_SELLER);

    let listing_id = object::uid_to_inner(&id);
    let weapon_id = object::id(&weapon);

    transfer::public_transfer(weapon, seller);
    object::delete(id);

    event::emit(ListingCancelled {
        listing_id,
        seller,
        weapon_id,
    });
}
