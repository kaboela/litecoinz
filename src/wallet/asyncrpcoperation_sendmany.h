// Copyright (c) 2016 The Zcash developers
// Copyright (c) 2017-2020 The LitecoinZ Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#ifndef ASYNCRPCOPERATION_SENDMANY_H
#define ASYNCRPCOPERATION_SENDMANY_H

#include <amount.h>
#include <asyncrpcoperation.h>
#include <primitives/transaction.h>
#include <rpc/request.h>
#include <transaction_builder.h>
#include <wallet/paymentdisclosure.h>
#include <wallet/wallet.h>
#include <zcash/Address.hpp>
#include <zcash/JoinSplit.hpp>

#include <array>
#include <tuple>
#include <unordered_map>

#include <univalue.h>

// Default transaction fee if caller does not specify one.
#define ASYNC_RPC_OPERATION_DEFAULT_MINERS_FEE   10000

class TxValues;

class SendManyRecipient {
public:
    std::string address;
    CAmount amount;
    std::string memo;

    SendManyRecipient(std::string address_, CAmount amount_, std::string memo_ = "") :
        address(address_), amount(amount_), memo(memo_) {}
};

class SendManyInputJSOP {
public:
    SproutOutPoint outpoint;
    libzcash::SproutNote note;
    CAmount amount;

    SendManyInputJSOP(SproutOutPoint outpoint_, libzcash::SproutNote note_, CAmount amount_) :
        outpoint(outpoint_), note(note_), amount(amount_) {}
};

// Package of info which is passed to perform_joinsplit methods.
struct AsyncJoinSplitInfo
{
    std::vector<libzcash::JSInput> vjsin;
    std::vector<libzcash::JSOutput> vjsout;
    std::vector<libzcash::SproutNote> notes;
    CAmount vpub_old = 0;
    CAmount vpub_new = 0;
};

// A struct to help us track the witness and anchor for a given SproutOutPoint
struct WitnessAnchorData {
	Optional<SproutWitness> witness;
	uint256 anchor;
};

class AsyncRPCOperation_sendmany : public AsyncRPCOperation {
public:
    AsyncRPCOperation_sendmany(
        CWallet* const pwallet,
        Optional<TransactionBuilder> builder,
        CMutableTransaction contextualTx,
        std::string fromAddress,
        std::vector<SendManyRecipient> tOutputs,
        std::vector<SendManyRecipient> zOutputs,
        int minDepth,
        CAmount fee = ASYNC_RPC_OPERATION_DEFAULT_MINERS_FEE,
        UniValue contextInfo = NullUniValue);
    virtual ~AsyncRPCOperation_sendmany();

    // We don't want to be copied or moved around
    AsyncRPCOperation_sendmany(AsyncRPCOperation_sendmany const&) = delete;             // Copy construct
    AsyncRPCOperation_sendmany(AsyncRPCOperation_sendmany&&) = delete;                  // Move construct
    AsyncRPCOperation_sendmany& operator=(AsyncRPCOperation_sendmany const&) = delete;  // Copy assign
    AsyncRPCOperation_sendmany& operator=(AsyncRPCOperation_sendmany &&) = delete;      // Move assign

    virtual void main();

    virtual UniValue getStatus() const;

    bool testmode = false;  // Set to true to disable sending txs and generating proofs

    bool paymentDisclosureMode = false; // Set to true to save esk for encrypted notes in payment disclosure database.

private:
    CWallet* pwallet_;
    CTransactionRef tx_;
    std::string fromaddress_;
    std::vector<SendManyRecipient> t_outputs_;
    std::vector<SendManyRecipient> z_outputs_;
    int mindepth_;
    CAmount fee_;
    UniValue contextinfo_;     // optional data to include in return value from getStatus()

    bool isUsingBuilder_; // Indicates that no Sprout addresses are involved
    uint32_t consensusBranchId_;
    bool isfromtaddr_;
    bool isfromzaddr_;
    CTxDestination fromtaddr_;
    libzcash::PaymentAddress frompaymentaddress_;
    libzcash::SpendingKey spendingkey_;

    uint256 joinSplitPubKey_;
    unsigned char joinSplitPrivKey_[crypto_sign_SECRETKEYBYTES];

    // The key is the result string from calling SproutOutPoint::ToString()
    std::unordered_map<std::string, WitnessAnchorData> jsopWitnessAnchorMap;

    std::vector<COutput> t_inputs_;
    std::vector<SendManyInputJSOP> z_sprout_inputs_;
    std::vector<SaplingNoteEntry> z_sapling_inputs_;

    TransactionBuilder builder_;

    void add_taddr_change_output_to_tx(ReserveDestination& changedest, CAmount amount);
    void add_taddr_outputs_to_tx();
    bool find_unspent_notes();
    bool find_utxos(TxValues& txValues);
    // Load transparent inputs into the transaction or the transactionBuilder (in case of have it)
    bool load_inputs(TxValues& txValues);
    std::array<unsigned char, ZC_MEMO_SIZE> get_memo_from_hex_string(std::string s);
    bool main_impl();

    // JoinSplit without any input notes to spend
    UniValue perform_joinsplit(AsyncJoinSplitInfo &);

    // JoinSplit with input notes to spend (SproutOutPoints))
    UniValue perform_joinsplit(AsyncJoinSplitInfo &, std::vector<SproutOutPoint> & );

    // JoinSplit where you have the witnesses and anchor
    UniValue perform_joinsplit(
        AsyncJoinSplitInfo & info,
        std::vector<Optional < SproutWitness>> witnesses,
        uint256 anchor);

    // payment disclosure!
    std::vector<PaymentDisclosureKeyInfo> paymentDisclosureData_;
};

#endif /* ASYNCRPCOPERATION_SENDMANY_H */
