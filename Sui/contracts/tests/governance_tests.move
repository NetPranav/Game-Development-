#[test_only]
module robot_rumble::governance_tests;

use std::string;
use sui::clock;
use sui::test_scenario as ts;
use robot_rumble::governance::{
    Self,
    GovernedConfig,
    Proposal,
    GovernanceAdminCap,
    VotingPowerVoucher
};

const ADMIN: address = @0xA11CE;
const VOTER1: address = @0xB0B;
const VOTER2: address = @0xCAFE;
const VOTER3: address = @0xD00D;

const DAY_MS: u64 = 24 * 60 * 60 * 1000;
const SEVEN_DAYS_MS: u64 = 7 * DAY_MS;
const FORTY_EIGHT_HOURS_MS: u64 = 48 * 60 * 60 * 1000;

#[test]
fun test_governance_full_lifecycle_success() {
    let mut scenario = ts::begin(ADMIN);
    
    // 1. Initialize governance & clock
    {
        governance::init_for_testing(ts::ctx(&mut scenario));
    };

    // Create test clock at t = 1,000,000 ms
    let mut clk = clock::create_for_testing(ts::ctx(&mut scenario));
    clock::set_for_testing(&mut clk, 1_000_000);

    // 2. Admin issues snapshot voting vouchers
    // Total eligible power denominator = 2,000
    // Quorum needed: 2,000 * 20% = 400
    ts::next_tx(&mut scenario, ADMIN);
    {
        let admin_cap = ts::take_from_sender<GovernanceAdminCap>(&scenario);
        governance::issue_voting_voucher(&admin_cap, VOTER1, 600, ts::ctx(&mut scenario));
        governance::issue_voting_voucher(&admin_cap, VOTER2, 200, ts::ctx(&mut scenario));
        governance::issue_voting_voucher(&admin_cap, VOTER3, 150, ts::ctx(&mut scenario));
        ts::return_to_sender(&scenario, admin_cap);
    };

    // 3. Voter1 creates proposal: Change PARAM_WIN_REWARD (ID 1) from 50 -> 80
    ts::next_tx(&mut scenario, VOTER1);
    {
        let mut config = ts::take_shared<GovernedConfig>(&scenario);
        governance::create_proposal(
            &mut config,
            string::utf8(b"Increase Arena Win Reward"),
            string::utf8(b"Boost reward from 50 to 80 RRC credits per victory"),
            1,  // PARAM_WIN_REWARD
            80, // Target value
            2000, // Total eligible voting power denominator
            &clk,
            ts::ctx(&mut scenario),
        );
        ts::return_shared(config);
    };

    // 4. Voters cast votes
    // Voter1: For (600 votes)
    ts::next_tx(&mut scenario, VOTER1);
    {
        let mut proposal = ts::take_shared<Proposal>(&scenario);
        let voucher = ts::take_from_sender<VotingPowerVoucher>(&scenario);

        governance::cast_vote(&mut proposal, &voucher, 1, &clk, ts::ctx(&mut scenario)); // 1 = For

        assert!(governance::for_votes(&proposal) == 600, 10);
        ts::return_shared(proposal);
        ts::return_to_sender(&scenario, voucher);
    };

    // Voter2: Against (200 votes)
    ts::next_tx(&mut scenario, VOTER2);
    {
        let mut proposal = ts::take_shared<Proposal>(&scenario);
        let voucher = ts::take_from_sender<VotingPowerVoucher>(&scenario);

        governance::cast_vote(&mut proposal, &voucher, 0, &clk, ts::ctx(&mut scenario)); // 0 = Against

        assert!(governance::against_votes(&proposal) == 200, 11);
        ts::return_shared(proposal);
        ts::return_to_sender(&scenario, voucher);
    };

    // Voter3: Abstain (150 votes)
    ts::next_tx(&mut scenario, VOTER3);
    {
        let mut proposal = ts::take_shared<Proposal>(&scenario);
        let voucher = ts::take_from_sender<VotingPowerVoucher>(&scenario);

        governance::cast_vote(&mut proposal, &voucher, 2, &clk, ts::ctx(&mut scenario)); // 2 = Abstain

        assert!(governance::abstain_votes(&proposal) == 150, 12);
        ts::return_shared(proposal);
        ts::return_to_sender(&scenario, voucher);
    };

    // Total participating: 600 + 200 + 150 = 950 >= 400 (Quorum 47.5% met!)
    // Non-abstaining: 600 + 200 = 800
    // Approval: 600 / 800 = 75% > 51% (Threshold met!)

    // 5. Advance clock past 7-day voting period
    clock::set_for_testing(&mut clk, 1_000_000 + SEVEN_DAYS_MS + 1000);

    // Evaluate proposal -> should enter STATE_PASSED_TIMELOCKED (state = 1)
    ts::next_tx(&mut scenario, ADMIN);
    {
        let mut proposal = ts::take_shared<Proposal>(&scenario);
        governance::evaluate_proposal(&mut proposal, &clk);
        assert!(governance::proposal_state(&proposal) == 1, 20); // 1 = STATE_PASSED_TIMELOCKED
        ts::return_shared(proposal);
    };

    // 6. Advance clock to after 48h timelock expiration
    clock::set_for_testing(&mut clk, 1_000_000 + SEVEN_DAYS_MS + FORTY_EIGHT_HOURS_MS + 2000);

    // 7. Execute proposal (permissionless)
    ts::next_tx(&mut scenario, VOTER1);
    {
        let mut proposal = ts::take_shared<Proposal>(&scenario);
        let mut config = ts::take_shared<GovernedConfig>(&scenario);

        governance::execute_proposal(&mut proposal, &mut config, &clk);

        assert!(governance::proposal_state(&proposal) == 3, 30); // 3 = STATE_EXECUTED
        assert!(governance::win_reward_credits(&config) == 80, 31); // Updated from 50 -> 80!

        ts::return_shared(proposal);
        ts::return_shared(config);
    };

    clock::destroy_for_testing(clk);
    ts::end(scenario);
}

#[test]
fun test_governance_quorum_failure() {
    let mut scenario = ts::begin(ADMIN);
    { governance::init_for_testing(ts::ctx(&mut scenario)); };
    let mut clk = clock::create_for_testing(ts::ctx(&mut scenario));
    clock::set_for_testing(&mut clk, 1_000_000);

    // Issue voucher with only 100 votes (out of 2000 total eligible => 5% < 20% Quorum)
    ts::next_tx(&mut scenario, ADMIN);
    {
        let admin_cap = ts::take_from_sender<GovernanceAdminCap>(&scenario);
        governance::issue_voting_voucher(&admin_cap, VOTER1, 100, ts::ctx(&mut scenario));
        ts::return_to_sender(&scenario, admin_cap);
    };

    ts::next_tx(&mut scenario, VOTER1);
    {
        let mut config = ts::take_shared<GovernedConfig>(&scenario);
        governance::create_proposal(
            &mut config,
            string::utf8(b"Raise Fees"),
            string::utf8(b"Proposal that fails quorum"),
            4,   // PARAM_MARKETPLACE_FEE_BPS
            300, // 300 bps
            2000,
            &clk,
            ts::ctx(&mut scenario),
        );
        ts::return_shared(config);
    };

    ts::next_tx(&mut scenario, VOTER1);
    {
        let mut proposal = ts::take_shared<Proposal>(&scenario);
        let voucher = ts::take_from_sender<VotingPowerVoucher>(&scenario);
        governance::cast_vote(&mut proposal, &voucher, 1, &clk, ts::ctx(&mut scenario)); // For
        ts::return_shared(proposal);
        ts::return_to_sender(&scenario, voucher);
    };

    // Advance clock past 7 days
    clock::set_for_testing(&mut clk, 1_000_000 + SEVEN_DAYS_MS + 1000);

    ts::next_tx(&mut scenario, ADMIN);
    {
        let mut proposal = ts::take_shared<Proposal>(&scenario);
        governance::evaluate_proposal(&mut proposal, &clk);
        // Quorum not met -> STATE_DEFEATED (state = 2)
        assert!(governance::proposal_state(&proposal) == 2, 40);
        ts::return_shared(proposal);
    };

    clock::destroy_for_testing(clk);
    ts::end(scenario);
}

#[test]
fun test_governance_approval_failure() {
    let mut scenario = ts::begin(ADMIN);
    { governance::init_for_testing(ts::ctx(&mut scenario)); };
    let mut clk = clock::create_for_testing(ts::ctx(&mut scenario));
    clock::set_for_testing(&mut clk, 1_000_000);

    // Denominator = 1000. Quorum = 200.
    // Voter1: 300 For. Voter2: 400 Against. Total = 700 >= 200 (Quorum met).
    // Approval: 300 / 700 = 42.8% <= 51% (Fails threshold!).
    ts::next_tx(&mut scenario, ADMIN);
    {
        let admin_cap = ts::take_from_sender<GovernanceAdminCap>(&scenario);
        governance::issue_voting_voucher(&admin_cap, VOTER1, 300, ts::ctx(&mut scenario));
        governance::issue_voting_voucher(&admin_cap, VOTER2, 400, ts::ctx(&mut scenario));
        ts::return_to_sender(&scenario, admin_cap);
    };

    ts::next_tx(&mut scenario, VOTER1);
    {
        let mut config = ts::take_shared<GovernedConfig>(&scenario);
        governance::create_proposal(
            &mut config,
            string::utf8(b"Lower Upgrade Cost"),
            string::utf8(b"Cut cost to 5 credits"),
            2, // PARAM_UPGRADE_COST_BASE
            5,
            1000,
            &clk,
            ts::ctx(&mut scenario),
        );
        ts::return_shared(config);
    };

    ts::next_tx(&mut scenario, VOTER1);
    {
        let mut proposal = ts::take_shared<Proposal>(&scenario);
        let voucher = ts::take_from_sender<VotingPowerVoucher>(&scenario);
        governance::cast_vote(&mut proposal, &voucher, 1, &clk, ts::ctx(&mut scenario)); // For
        ts::return_shared(proposal);
        ts::return_to_sender(&scenario, voucher);
    };

    ts::next_tx(&mut scenario, VOTER2);
    {
        let mut proposal = ts::take_shared<Proposal>(&scenario);
        let voucher = ts::take_from_sender<VotingPowerVoucher>(&scenario);
        governance::cast_vote(&mut proposal, &voucher, 0, &clk, ts::ctx(&mut scenario)); // Against
        ts::return_shared(proposal);
        ts::return_to_sender(&scenario, voucher);
    };

    // Advance clock past 7 days
    clock::set_for_testing(&mut clk, 1_000_000 + SEVEN_DAYS_MS + 1000);

    ts::next_tx(&mut scenario, ADMIN);
    {
        let mut proposal = ts::take_shared<Proposal>(&scenario);
        governance::evaluate_proposal(&mut proposal, &clk);
        // Approval threshold not met -> STATE_DEFEATED (state = 2)
        assert!(governance::proposal_state(&proposal) == 2, 50);
        ts::return_shared(proposal);
    };

    clock::destroy_for_testing(clk);
    ts::end(scenario);
}

#[test]
#[expected_failure(abort_code = governance::E_ALREADY_VOTED)]
fun test_governance_double_voting_aborts() {
    let mut scenario = ts::begin(ADMIN);
    { governance::init_for_testing(ts::ctx(&mut scenario)); };
    let mut clk = clock::create_for_testing(ts::ctx(&mut scenario));
    clock::set_for_testing(&mut clk, 1_000_000);

    ts::next_tx(&mut scenario, ADMIN);
    {
        let admin_cap = ts::take_from_sender<GovernanceAdminCap>(&scenario);
        governance::issue_voting_voucher(&admin_cap, VOTER1, 500, ts::ctx(&mut scenario));
        ts::return_to_sender(&scenario, admin_cap);
    };

    ts::next_tx(&mut scenario, VOTER1);
    {
        let mut config = ts::take_shared<GovernedConfig>(&scenario);
        governance::create_proposal(
            &mut config,
            string::utf8(b"Test Proposal"),
            string::utf8(b"Testing double vote"),
            1,
            60,
            1000,
            &clk,
            ts::ctx(&mut scenario),
        );
        ts::return_shared(config);
    };

    // Vote 1st time
    ts::next_tx(&mut scenario, VOTER1);
    {
        let mut proposal = ts::take_shared<Proposal>(&scenario);
        let voucher = ts::take_from_sender<VotingPowerVoucher>(&scenario);
        governance::cast_vote(&mut proposal, &voucher, 1, &clk, ts::ctx(&mut scenario));
        ts::return_shared(proposal);
        ts::return_to_sender(&scenario, voucher);
    };

    // Vote 2nd time -> ABORTS with E_ALREADY_VOTED
    ts::next_tx(&mut scenario, VOTER1);
    {
        let mut proposal = ts::take_shared<Proposal>(&scenario);
        let voucher = ts::take_from_sender<VotingPowerVoucher>(&scenario);
        governance::cast_vote(&mut proposal, &voucher, 1, &clk, ts::ctx(&mut scenario));
        ts::return_shared(proposal);
        ts::return_to_sender(&scenario, voucher);
    };

    clock::destroy_for_testing(clk);
    ts::end(scenario);
}

#[test]
#[expected_failure(abort_code = governance::E_TIMELOCK_ACTIVE)]
fun test_governance_premature_execution_aborts() {
    let mut scenario = ts::begin(ADMIN);
    { governance::init_for_testing(ts::ctx(&mut scenario)); };
    let mut clk = clock::create_for_testing(ts::ctx(&mut scenario));
    clock::set_for_testing(&mut clk, 1_000_000);

    ts::next_tx(&mut scenario, ADMIN);
    {
        let admin_cap = ts::take_from_sender<GovernanceAdminCap>(&scenario);
        governance::issue_voting_voucher(&admin_cap, VOTER1, 500, ts::ctx(&mut scenario));
        ts::return_to_sender(&scenario, admin_cap);
    };

    ts::next_tx(&mut scenario, VOTER1);
    {
        let mut config = ts::take_shared<GovernedConfig>(&scenario);
        governance::create_proposal(
            &mut config,
            string::utf8(b"Premature Execution Test"),
            string::utf8(b"Testing timelock bypass rejection"),
            1,
            70,
            1000,
            &clk,
            ts::ctx(&mut scenario),
        );
        ts::return_shared(config);
    };

    ts::next_tx(&mut scenario, VOTER1);
    {
        let mut proposal = ts::take_shared<Proposal>(&scenario);
        let voucher = ts::take_from_sender<VotingPowerVoucher>(&scenario);
        governance::cast_vote(&mut proposal, &voucher, 1, &clk, ts::ctx(&mut scenario));
        ts::return_shared(proposal);
        ts::return_to_sender(&scenario, voucher);
    };

    // Advance clock past 7 days to evaluate
    clock::set_for_testing(&mut clk, 1_000_000 + SEVEN_DAYS_MS + 1000);

    ts::next_tx(&mut scenario, ADMIN);
    {
        let mut proposal = ts::take_shared<Proposal>(&scenario);
        governance::evaluate_proposal(&mut proposal, &clk);
        assert!(governance::proposal_state(&proposal) == 1, 60); // In timelock
        ts::return_shared(proposal);
    };

    // Attempt execution before 48h has elapsed (only 1 hour into timelock) -> MUST ABORT!
    clock::set_for_testing(&mut clk, 1_000_000 + SEVEN_DAYS_MS + (1 * 60 * 60 * 1000));

    ts::next_tx(&mut scenario, VOTER1);
    {
        let mut proposal = ts::take_shared<Proposal>(&scenario);
        let mut config = ts::take_shared<GovernedConfig>(&scenario);
        governance::execute_proposal(&mut proposal, &mut config, &clk);
        ts::return_shared(proposal);
        ts::return_shared(config);
    };

    clock::destroy_for_testing(clk);
    ts::end(scenario);
}

#[test]
#[expected_failure(abort_code = governance::E_UNAPPROVED_PARAMETER)]
fun test_governance_unapproved_parameter_aborts() {
    let mut scenario = ts::begin(ADMIN);
    { governance::init_for_testing(ts::ctx(&mut scenario)); };
    let clk = clock::create_for_testing(ts::ctx(&mut scenario));

    ts::next_tx(&mut scenario, VOTER1);
    {
        let mut config = ts::take_shared<GovernedConfig>(&scenario);
        // Attempt parameter ID 99 (arbitrary / unapproved) -> MUST ABORT!
        governance::create_proposal(
            &mut config,
            string::utf8(b"Malicious Proposal"),
            string::utf8(b"Attempting to modify unapproved parameter"),
            99, // Unapproved param
            100,
            1000,
            &clk,
            ts::ctx(&mut scenario),
        );
        ts::return_shared(config);
    };

    clock::destroy_for_testing(clk);
    ts::end(scenario);
}
