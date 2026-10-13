module robot_rumble::governance;

use std::string::String;
use sui::clock::{Self, Clock};
use sui::event;
use sui::vec_set::{Self, VecSet};

// === Errors ===
const E_NOT_AUTHORIZED: u64 = 1;
const E_VOTING_NOT_OPEN: u64 = 2;
const E_VOTING_ENDED: u64 = 3;
const E_ALREADY_VOTED: u64 = 4;
const E_INVALID_VOTE_TYPE: u64 = 5;
const E_APPROVAL_THRESHOLD_NOT_MET: u64 = 7;
const E_TIMELOCK_ACTIVE: u64 = 8;
const E_ALREADY_EXECUTED: u64 = 9;
const E_UNAPPROVED_PARAMETER: u64 = 10;
const E_VALUE_OUT_OF_BOUNDS: u64 = 11;
const E_ZERO_VOTING_POWER: u64 = 12;
const E_ZERO_TOTAL_POWER: u64 = 13;

// === Time Constants (in milliseconds) ===
/// 7 days voting period
const VOTING_PERIOD_MS: u64 = 7 * 24 * 60 * 60 * 1000; // 604,800,000 ms
/// 48 hours execution timelock
const TIMELOCK_DELAY_MS: u64 = 48 * 60 * 60 * 1000;    // 172,800,000 ms

// === Governance Thresholds ===
/// Quorum: 20% of snapshot eligible voting power
const QUORUM_PERCENT: u64 = 20;
/// Approval: Strictly > 51% of participating (For + Against) non-abstaining votes
const APPROVAL_PERCENT: u64 = 51;

// === Vote Types ===
const VOTE_AGAINST: u8 = 0;
const VOTE_FOR: u8 = 1;
const VOTE_ABSTAIN: u8 = 2;

// === Proposal States ===
const STATE_ACTIVE: u8 = 0;
const STATE_PASSED_TIMELOCKED: u8 = 1;
const STATE_DEFEATED: u8 = 2;
const STATE_EXECUTED: u8 = 3;

// === Allowed Governed Parameter IDs ===
const PARAM_WIN_REWARD: u8 = 1;           // Reward credits per victory (default 50)
const PARAM_UPGRADE_COST_BASE: u8 = 2;     // Base upgrade cost (default 10)
const PARAM_MAX_UPGRADE_LEVEL: u8 = 3;     // Max gear upgrade level (default 20)
const PARAM_MARKETPLACE_FEE_BPS: u8 = 4;   // Marketplace fee in basis points (default 250 = 2.5%)

// === Governed Registry (Shared Object) ===
public struct GovernedConfig has key {
    id: UID,
    win_reward_credits: u64,
    upgrade_cost_base: u64,
    max_upgrade_level: u64,
    marketplace_fee_bps: u64,
    proposal_count: u64,
}

// === Proposal (Shared Object) ===
public struct Proposal has key {
    id: UID,
    proposal_index: u64,
    proposer: address,
    title: String,
    description: String,
    param_id: u8,
    target_value: u64,
    start_time_ms: u64,
    end_time_ms: u64,
    execution_time_ms: u64,
    total_eligible_voting_power: u64,
    for_votes: u64,
    against_votes: u64,
    abstain_votes: u64,
    voters: VecSet<address>,
    state: u8,
}

// === Governance Admin Capability ===
public struct GovernanceAdminCap has key, store {
    id: UID,
}

// === Voting Proof / Eligibility Voucher ===
/// Explicit capability representing snapshot-verified voting weight.
/// Separates the governance voting logic from the currently undecided voter eligibility policy.
public struct VotingPowerVoucher has key, store {
    id: UID,
    voter: address,
    voting_power: u64,
}

// === Events ===
public struct ProposalCreated has copy, drop {
    proposal_id: ID,
    proposal_index: u64,
    proposer: address,
    title: String,
    param_id: u8,
    target_value: u64,
    start_time_ms: u64,
    end_time_ms: u64,
    total_eligible_voting_power: u64,
}

public struct VoteCast has copy, drop {
    proposal_id: ID,
    voter: address,
    vote_type: u8,
    voting_power: u64,
}

public struct ProposalTimelocked has copy, drop {
    proposal_id: ID,
    execution_allowed_after_ms: u64,
    for_votes: u64,
    against_votes: u64,
    abstain_votes: u64,
}

public struct ProposalExecuted has copy, drop {
    proposal_id: ID,
    param_id: u8,
    old_value: u64,
    new_value: u64,
}

public struct ProposalDefeated has copy, drop {
    proposal_id: ID,
    reason: u8, // 1: Quorum not met, 2: Approval threshold not met
}

// === Init ===
fun init(ctx: &mut TxContext) {
    let config = GovernedConfig {
        id: object::new(ctx),
        win_reward_credits: 50,
        upgrade_cost_base: 10,
        max_upgrade_level: 20,
        marketplace_fee_bps: 250,
        proposal_count: 0,
    };
    transfer::share_object(config);
    transfer::transfer(GovernanceAdminCap { id: object::new(ctx) }, ctx.sender());
}

// === Proposal Creation ===
public fun create_proposal(
    config: &mut GovernedConfig,
    title: String,
    description: String,
    param_id: u8,
    target_value: u64,
    total_eligible_voting_power: u64,
    clock: &Clock,
    ctx: &mut TxContext,
) {
    assert!(total_eligible_voting_power > 0, E_ZERO_TOTAL_POWER);
    validate_parameter(param_id, target_value);

    config.proposal_count = config.proposal_count + 1;
    let now = clock::timestamp_ms(clock);
    let end_time = now + VOTING_PERIOD_MS;

    let proposal = Proposal {
        id: object::new(ctx),
        proposal_index: config.proposal_count,
        proposer: ctx.sender(),
        title,
        description,
        param_id,
        target_value,
        start_time_ms: now,
        end_time_ms: end_time,
        execution_time_ms: end_time + TIMELOCK_DELAY_MS,
        total_eligible_voting_power,
        for_votes: 0,
        against_votes: 0,
        abstain_votes: 0,
        voters: vec_set::empty(),
        state: STATE_ACTIVE,
    };

    let proposal_id = object::id(&proposal);
    event::emit(ProposalCreated {
        proposal_id,
        proposal_index: proposal.proposal_index,
        proposer: proposal.proposer,
        title: proposal.title,
        param_id,
        target_value,
        start_time_ms: now,
        end_time_ms: end_time,
        total_eligible_voting_power,
    });

    transfer::share_object(proposal);
}

// === Issue Snapshot Voucher (Admin / Policy Authority) ===
public fun issue_voting_voucher(
    _admin: &GovernanceAdminCap,
    voter: address,
    voting_power: u64,
    ctx: &mut TxContext,
) {
    assert!(voting_power > 0, E_ZERO_VOTING_POWER);
    let voucher = VotingPowerVoucher {
        id: object::new(ctx),
        voter,
        voting_power,
    };
    transfer::transfer(voucher, voter);
}

// === Vote on Proposal ===
public fun cast_vote(
    proposal: &mut Proposal,
    voucher: &VotingPowerVoucher,
    vote_type: u8,
    clock: &Clock,
    ctx: &TxContext,
) {
    let now = clock::timestamp_ms(clock);
    let sender = ctx.sender();

    // Verify voucher owner
    assert!(voucher.voter == sender, E_NOT_AUTHORIZED);
    assert!(voucher.voting_power > 0, E_ZERO_VOTING_POWER);

    // Verify proposal timing & state
    assert!(proposal.state == STATE_ACTIVE, E_VOTING_ENDED);
    assert!(now >= proposal.start_time_ms, E_VOTING_NOT_OPEN);
    assert!(now <= proposal.end_time_ms, E_VOTING_ENDED);

    // Verify no double voting
    assert!(!vec_set::contains(&proposal.voters, &sender), E_ALREADY_VOTED);
    assert!(vote_type == VOTE_AGAINST || vote_type == VOTE_FOR || vote_type == VOTE_ABSTAIN, E_INVALID_VOTE_TYPE);

    vec_set::insert(&mut proposal.voters, sender);

    if (vote_type == VOTE_FOR) {
        proposal.for_votes = proposal.for_votes + voucher.voting_power;
    } else if (vote_type == VOTE_AGAINST) {
        proposal.against_votes = proposal.against_votes + voucher.voting_power;
    } else {
        proposal.abstain_votes = proposal.abstain_votes + voucher.voting_power;
    };

    event::emit(VoteCast {
        proposal_id: object::id(proposal),
        voter: sender,
        vote_type,
        voting_power: voucher.voting_power,
    });
}

// === Conclude Voting & Enter Timelock ===
public fun evaluate_proposal(
    proposal: &mut Proposal,
    clock: &Clock,
) {
    let now = clock::timestamp_ms(clock);
    assert!(proposal.state == STATE_ACTIVE, E_ALREADY_EXECUTED);
    assert!(now > proposal.end_time_ms, E_VOTING_NOT_OPEN);

    let total_participating = proposal.for_votes + proposal.against_votes + proposal.abstain_votes;
    let required_quorum = (proposal.total_eligible_voting_power * QUORUM_PERCENT) / 100;

    // Check Quorum (Abstentions count toward quorum)
    if (total_participating < required_quorum) {
        proposal.state = STATE_DEFEATED;
        event::emit(ProposalDefeated {
            proposal_id: object::id(proposal),
            reason: 1, // Quorum not met
        });
        return
    };

    // Check Approval Threshold: strictly > 51% of (for_votes + against_votes)
    let non_abstaining = proposal.for_votes + proposal.against_votes;
    if (non_abstaining == 0) {
        proposal.state = STATE_DEFEATED;
        event::emit(ProposalDefeated {
            proposal_id: object::id(proposal),
            reason: 2, // No yes/no votes cast
        });
        return
    };

    // for_votes * 100 > non_abstaining * 51
    if ((proposal.for_votes * 100) > (non_abstaining * APPROVAL_PERCENT)) {
        proposal.state = STATE_PASSED_TIMELOCKED;
        event::emit(ProposalTimelocked {
            proposal_id: object::id(proposal),
            execution_allowed_after_ms: proposal.execution_time_ms,
            for_votes: proposal.for_votes,
            against_votes: proposal.against_votes,
            abstain_votes: proposal.abstain_votes,
        });
    } else {
        proposal.state = STATE_DEFEATED;
        event::emit(ProposalDefeated {
            proposal_id: object::id(proposal),
            reason: 2, // Approval threshold not met
        });
    };
}

// === Execute Passed Proposal (Permissionless after 48h Timelock) ===
public fun execute_proposal(
    proposal: &mut Proposal,
    config: &mut GovernedConfig,
    clock: &Clock,
) {
    assert!(proposal.state == STATE_PASSED_TIMELOCKED, E_APPROVAL_THRESHOLD_NOT_MET);
    let now = clock::timestamp_ms(clock);
    assert!(now >= proposal.execution_time_ms, E_TIMELOCK_ACTIVE);

    let old_val = apply_parameter(config, proposal.param_id, proposal.target_value);
    proposal.state = STATE_EXECUTED;

    event::emit(ProposalExecuted {
        proposal_id: object::id(proposal),
        param_id: proposal.param_id,
        old_value: old_val,
        new_value: proposal.target_value,
    });
}

// === Internal Validation of Parameters ===
fun validate_parameter(param_id: u8, val: u64) {
    if (param_id == PARAM_WIN_REWARD) {
        assert!(val >= 10 && val <= 500, E_VALUE_OUT_OF_BOUNDS);
    } else if (param_id == PARAM_UPGRADE_COST_BASE) {
        assert!(val >= 1 && val <= 100, E_VALUE_OUT_OF_BOUNDS);
    } else if (param_id == PARAM_MAX_UPGRADE_LEVEL) {
        assert!(val >= 5 && val <= 50, E_VALUE_OUT_OF_BOUNDS);
    } else if (param_id == PARAM_MARKETPLACE_FEE_BPS) {
        assert!(val <= 1000, E_VALUE_OUT_OF_BOUNDS); // Max 10%
    } else {
        abort E_UNAPPROVED_PARAMETER
    };
}

fun apply_parameter(config: &mut GovernedConfig, param_id: u8, val: u64): u64 {
    validate_parameter(param_id, val);
    if (param_id == PARAM_WIN_REWARD) {
        let old = config.win_reward_credits;
        config.win_reward_credits = val;
        old
    } else if (param_id == PARAM_UPGRADE_COST_BASE) {
        let old = config.upgrade_cost_base;
        config.upgrade_cost_base = val;
        old
    } else if (param_id == PARAM_MAX_UPGRADE_LEVEL) {
        let old = config.max_upgrade_level;
        config.max_upgrade_level = val;
        old
    } else if (param_id == PARAM_MARKETPLACE_FEE_BPS) {
        let old = config.marketplace_fee_bps;
        config.marketplace_fee_bps = val;
        old
    } else {
        abort E_UNAPPROVED_PARAMETER
    }
}

// === Public Getters ===
public fun win_reward_credits(config: &GovernedConfig): u64 { config.win_reward_credits }
public fun upgrade_cost_base(config: &GovernedConfig): u64 { config.upgrade_cost_base }
public fun max_upgrade_level(config: &GovernedConfig): u64 { config.max_upgrade_level }
public fun marketplace_fee_bps(config: &GovernedConfig): u64 { config.marketplace_fee_bps }
public fun proposal_state(proposal: &Proposal): u8 { proposal.state }
public fun for_votes(proposal: &Proposal): u64 { proposal.for_votes }
public fun against_votes(proposal: &Proposal): u64 { proposal.against_votes }
public fun abstain_votes(proposal: &Proposal): u64 { proposal.abstain_votes }
public fun total_eligible_voting_power(proposal: &Proposal): u64 { proposal.total_eligible_voting_power }
public fun execution_time_ms(proposal: &Proposal): u64 { proposal.execution_time_ms }
public fun param_id(proposal: &Proposal): u8 { proposal.param_id }
public fun target_value(proposal: &Proposal): u64 { proposal.target_value }

#[test_only]
public fun init_for_testing(ctx: &mut TxContext) {
    init(ctx);
}
