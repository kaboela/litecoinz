// Copyright (c) 2018 The Bitcoin Core developers
// Copyright (c) 2017-2020 The LitecoinZ Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_INTERFACES_WALLET_H
#define BITCOIN_INTERFACES_WALLET_H

#include <amount.h>                    // For CAmount
#include <pubkey.h>                    // For CKeyID and CScriptID (definitions needed in CTxDestination instantiation)
#include <script/standard.h>           // For CTxDestination
#include <support/allocators/secure.h> // For SecureString
#include <ui_interface.h>              // For ChangeType

#include <functional>
#include <map>
#include <memory>
#include <stdint.h>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

class CCoinControl;
class CInputControl;
class CFeeRate;
class CKey;
class CWallet;
class SproutNoteData;
class SaplingNoteData;
enum isminetype : unsigned int;
enum class FeeReason;
typedef uint8_t isminefilter;

enum class OutputType;
struct CRecipient;

namespace interfaces {

class Handler;
struct WalletAddress;
struct WalletShieldedAddress;
struct WalletBalances;
struct WalletTx;
struct WalletTxOut;
struct WalletSproutNote;
struct WalletSaplingNote;
struct WalletTxStatus;

using WalletOrderForm = std::vector<std::pair<std::string, std::string>>;
using WalletValueMap = std::map<std::string, std::string>;

//! Interface for accessing a wallet.
class Wallet
{
public:
    virtual ~Wallet() {}

    //! Encrypt wallet.
    virtual bool encryptWallet(const SecureString& wallet_passphrase) = 0;

    //! Return whether wallet is encrypted.
    virtual bool isCrypted() = 0;

    //! Lock wallet.
    virtual bool lock() = 0;

    //! Unlock wallet.
    virtual bool unlock(const SecureString& wallet_passphrase) = 0;

    //! Return whether wallet is locked.
    virtual bool isLocked() = 0;

    //! Change wallet passphrase.
    virtual bool changeWalletPassphrase(const SecureString& old_wallet_passphrase,
        const SecureString& new_wallet_passphrase) = 0;

    //! Abort a rescan.
    virtual void abortRescan() = 0;

    //! Back up wallet.
    virtual bool backupWallet(const std::string& filename) = 0;

    //! Get wallet name.
    virtual std::string getWalletName() = 0;

    // Get a new address.
    virtual bool getNewDestination(const OutputType type, const std::string label, CTxDestination& dest) = 0;

    // Get a new sprout address.
    virtual bool getNewSproutDestination(const std::string label, libzcash::PaymentAddress& dest) = 0;

    // Get a new sapling address.
    virtual bool getNewSaplingDestination(const std::string label, libzcash::PaymentAddress& dest) = 0;

    //! Get public key.
    virtual bool getPubKey(const CKeyID& address, CPubKey& pub_key) = 0;

    //! Get private key.
    virtual bool getPrivKey(const CKeyID& address, CKey& key) = 0;

    //! Return whether wallet has private key.
    virtual bool isSpendable(const CTxDestination& dest) = 0;

    //! Return whether wallet has watch only keys.
    virtual bool haveWatchOnly() = 0;

    //! Add or update address.
    virtual bool setAddressBook(const CTxDestination& dest, const std::string& name, const std::string& purpose) = 0;
    virtual bool setSproutAddressBook(const libzcash::PaymentAddress& dest, const std::string& name, const std::string& purpose) = 0;
    virtual bool setSaplingAddressBook(const libzcash::PaymentAddress& dest, const std::string& name, const std::string& purpose) = 0;

    // Remove address.
    virtual bool delAddressBook(const CTxDestination& dest) = 0;
    virtual bool delSproutAddressBook(const libzcash::PaymentAddress& dest) = 0;
    virtual bool delSaplingAddressBook(const libzcash::PaymentAddress& dest) = 0;

    //! Look up address in wallet, return whether exists.
    virtual bool getAddress(const CTxDestination& dest,
        std::string* name,
        isminetype* is_mine,
        std::string* purpose) = 0;
    virtual bool getSproutAddress(const libzcash::PaymentAddress& dest,
        std::string* name,
        isminetype* is_mine,
        std::string* purpose) = 0;
    virtual bool getSaplingAddress(const libzcash::PaymentAddress& dest,
        std::string* name,
        isminetype* is_mine,
        std::string* purpose) = 0;

    //! Get wallet transparent address list.
    virtual std::vector<WalletAddress> getAddresses() = 0;

    //! Get wallet sprout address list.
    virtual std::vector<WalletShieldedAddress> getSproutAddresses() = 0;

    //! Get wallet sapling address list.
    virtual std::vector<WalletShieldedAddress> getSaplingAddresses() = 0;

    //! Add scripts to key store so old so software versions opening the wallet
    //! database can detect payments to newer address types.
    virtual void learnRelatedScripts(const CPubKey& key, OutputType type) = 0;

    //! Add dest data.
    virtual bool addDestData(const CTxDestination& dest, const std::string& key, const std::string& value) = 0;

    //! Erase dest data.
    virtual bool eraseDestData(const CTxDestination& dest, const std::string& key) = 0;

    //! Get dest values with prefix.
    virtual std::vector<std::string> getDestValues(const std::string& prefix) = 0;

    //! Lock coin.
    virtual void lockCoin(const COutPoint& output) = 0;

    //! Unlock coin.
    virtual void unlockCoin(const COutPoint& output) = 0;

    //! Return whether coin is locked.
    virtual bool isLockedCoin(const COutPoint& output) = 0;

    //! List locked coins.
    virtual void listLockedCoins(std::vector<COutPoint>& outputs) = 0;

    //! Create transaction.
    virtual CTransactionRef createTransaction(const std::vector<CRecipient>& recipients,
        const CCoinControl& coin_control,
        bool sign,
        int& change_pos,
        CAmount& fee,
        std::string& fail_reason) = 0;

    //! Commit transaction.
    virtual void commitTransaction(CTransactionRef tx,
        WalletValueMap value_map,
        WalletOrderForm order_form) = 0;

    //! Return whether transaction can be abandoned.
    virtual bool transactionCanBeAbandoned(const uint256& txid) = 0;

    //! Abandon transaction.
    virtual bool abandonTransaction(const uint256& txid) = 0;

    //! Return whether transaction can be bumped.
    virtual bool transactionCanBeBumped(const uint256& txid) = 0;

    //! Create bump transaction.
    virtual bool createBumpTransaction(const uint256& txid,
        const CCoinControl& coin_control,
        CAmount total_fee,
        std::vector<std::string>& errors,
        CAmount& old_fee,
        CAmount& new_fee,
        CMutableTransaction& mtx) = 0;

    //! Sign bump transaction.
    virtual bool signBumpTransaction(CMutableTransaction& mtx) = 0;

    //! Commit bump transaction.
    virtual bool commitBumpTransaction(const uint256& txid,
        CMutableTransaction&& mtx,
        std::vector<std::string>& errors,
        uint256& bumped_txid) = 0;

    //! Get a transaction.
    virtual CTransactionRef getTx(const uint256& txid) = 0;

    //! Get transaction information.
    virtual WalletTx getWalletTx(const uint256& txid) = 0;

    //! Get list of all wallet transactions.
    virtual std::vector<WalletTx> getWalletTxs() = 0;

    //! Try to get updated status for a particular transaction, if possible without blocking.
    virtual bool tryGetTxStatus(const uint256& txid,
        WalletTxStatus& tx_status,
        int& num_blocks,
        int64_t& block_time) = 0;

    //! Get transaction details.
    virtual WalletTx getWalletTxDetails(const uint256& txid,
        WalletTxStatus& tx_status,
        WalletOrderForm& order_form,
        bool& in_mempool,
        int& num_blocks) = 0;

    //! Get balances.
    virtual WalletBalances getBalances() = 0;

    //! Get shielded balance.
    virtual CAmount getShieldedBalance() = 0;

    //! Get balances if possible without waiting for chain and wallet locks.
    virtual bool tryGetBalances(WalletBalances& balances,
        int& num_blocks,
        bool force,
        int cached_num_blocks) = 0;

    //! Get balance.
    virtual CAmount getBalance() = 0;

    //! Get transparent balance.
    virtual CAmount getBalanceTaddr(std::string address) = 0;

    //! Get shielded balance.
    virtual CAmount getBalanceZaddr(std::string address) = 0;

    //! Get available balance.
    virtual CAmount getAvailableBalance(const CCoinControl& coin_control) = 0;

    //! Return whether transaction input belongs to wallet.
    virtual isminetype txinIsMine(const CTxIn& txin) = 0;

    //! Return whether transaction output belongs to wallet.
    virtual isminetype txoutIsMine(const CTxOut& txout) = 0;

    //! Return debit amount if transaction input belongs to wallet.
    virtual CAmount getDebit(const CTxIn& txin, isminefilter filter) = 0;

    //! Return credit amount if transaction input belongs to wallet.
    virtual CAmount getCredit(const CTxOut& txout, isminefilter filter) = 0;

    //! Return AvailableCoins + LockedCoins grouped by wallet address.
    //! (put change in one group with wallet address)
    using CoinsList = std::map<CTxDestination, std::vector<std::tuple<COutPoint, WalletTxOut>>>;
    virtual CoinsList listCoins(bool fOnlyCoinbase, bool fIncludeCoinbase) = 0;

    //! Return Sprout GetFilteredNotes grouped by wallet address.
    //! (put change in one group with wallet address)
    using SproutNotesList = std::map<libzcash::SproutPaymentAddress, std::vector<std::tuple<SproutOutPoint, WalletSproutNote>>>;
    virtual SproutNotesList listSproutNotes() = 0;

    //! Return Sapling GetFilteredNotes grouped by wallet address.
    //! (put change in one group with wallet address)
    using SaplingNotesList = std::map<libzcash::SaplingPaymentAddress, std::vector<std::tuple<SaplingOutPoint, WalletSaplingNote>>>;
    virtual SaplingNotesList listSaplingNotes() = 0;

    //! Return wallet transaction output information.
    virtual std::vector<WalletTxOut> getCoins(const std::vector<COutPoint>& outputs) = 0;

    //! Get required fee.
    virtual CAmount getRequiredFee(unsigned int tx_bytes) = 0;

    //! Get minimum fee.
    virtual CAmount getMinimumFee(unsigned int tx_bytes,
        const CCoinControl& coin_control,
        int* returned_target,
        FeeReason* reason) = 0;

    //! Get custom fee.
    virtual CAmount getCustomFee(const CInputControl& input_control) = 0;

    //! Get tx confirm target.
    virtual unsigned int getConfirmTarget() = 0;

    // Return whether HD enabled.
    virtual bool hdEnabled() = 0;

    // Return whether the wallet is blank.
    virtual bool canGetAddresses() = 0;

    // check if a certain wallet flag is set.
    virtual bool IsWalletFlagSet(uint64_t flag) = 0;

    // Get default address type.
    virtual OutputType getDefaultAddressType() = 0;

    // Get default change type.
    virtual OutputType getDefaultChangeType() = 0;

    //! Get max tx fee.
    virtual CAmount getDefaultMaxTxFee() = 0;

    // Remove wallet.
    virtual void remove() = 0;

    //! Register handler for unload message.
    using UnloadFn = std::function<void()>;
    virtual std::unique_ptr<Handler> handleUnload(UnloadFn fn) = 0;

    //! Register handler for show progress messages.
    using ShowProgressFn = std::function<void(const std::string& title, int progress)>;
    virtual std::unique_ptr<Handler> handleShowProgress(ShowProgressFn fn) = 0;

    //! Register handler for status changed messages.
    using StatusChangedFn = std::function<void()>;
    virtual std::unique_ptr<Handler> handleStatusChanged(StatusChangedFn fn) = 0;

    //! Register handler for transparent address book changed messages.
    using AddressBookChangedFn = std::function<void(const CTxDestination& address,
        const std::string& label,
        bool is_mine,
        const std::string& purpose,
        ChangeType status)>;
    virtual std::unique_ptr<Handler> handleAddressBookChanged(AddressBookChangedFn fn) = 0;

    //! Register handler for sprout address book changed messages.
    using SproutAddressBookChangedFn = std::function<void(const libzcash::PaymentAddress& address,
        const std::string& label,
        bool is_mine,
        const std::string& purpose,
        ChangeType status)>;
    virtual std::unique_ptr<Handler> handleSproutAddressBookChanged(SproutAddressBookChangedFn fn) = 0;

    //! Register handler for sapling address book changed messages.
    using SaplingAddressBookChangedFn = std::function<void(const libzcash::PaymentAddress& address,
        const std::string& label,
        bool is_mine,
        const std::string& purpose,
        ChangeType status)>;
    virtual std::unique_ptr<Handler> handleSaplingAddressBookChanged(SaplingAddressBookChangedFn fn) = 0;

    //! Register handler for transaction changed messages.
    using TransactionChangedFn = std::function<void(const uint256& txid, ChangeType status)>;
    virtual std::unique_ptr<Handler> handleTransactionChanged(TransactionChangedFn fn) = 0;

    //! Register handler for watchonly changed messages.
    using WatchOnlyChangedFn = std::function<void(bool have_watch_only)>;
    virtual std::unique_ptr<Handler> handleWatchOnlyChanged(WatchOnlyChangedFn fn) = 0;

    //! Register handler for keypool changed messages.
    using CanGetAddressesChangedFn = std::function<void()>;
    virtual std::unique_ptr<Handler> handleCanGetAddressesChanged(CanGetAddressesChangedFn fn) = 0;
};

//! Information about one wallet address.
struct WalletAddress
{
    CTxDestination dest;
    isminetype is_mine;
    std::string name;
    std::string purpose;

    WalletAddress(CTxDestination dest, isminetype is_mine, std::string name, std::string purpose)
        : dest(std::move(dest)), is_mine(is_mine), name(std::move(name)), purpose(std::move(purpose))
    {
    }
};

//! Information about one wallet shielded address.
struct WalletShieldedAddress
{
    libzcash::PaymentAddress dest;
    isminetype is_mine;
    std::string name;
    std::string purpose;

    WalletShieldedAddress(libzcash::PaymentAddress dest, isminetype is_mine, std::string name, std::string purpose)
        : dest(std::move(dest)), is_mine(is_mine), name(std::move(name)), purpose(std::move(purpose))
    {
    }
};

//! Collection of wallet balances.
struct WalletBalances
{
    CAmount balance = 0;
    CAmount coinbase_balance = 0;
    CAmount shielded_balance = 0;
    CAmount unconfirmed_balance = 0;
    CAmount unconfirmed_coinbase_balance = 0;
    CAmount unconfirmed_shielded_balance = 0;
    CAmount immature_balance = 0;
    CAmount immature_shielded_balance = 0;
    bool have_watch_only = false;
    CAmount watch_only_balance = 0;
    CAmount watch_only_coinbase_balance = 0;
    CAmount watch_only_shielded_balance = 0;
    CAmount unconfirmed_watch_only_balance = 0;
    CAmount unconfirmed_watch_only_coinbase_balance = 0;
    CAmount unconfirmed_watch_only_shielded_balance = 0;
    CAmount immature_watch_only_balance = 0;
    CAmount immature_watch_only_shielded_balance = 0;

    bool balanceChanged(const WalletBalances& prev) const
    {
        return balance != prev.balance ||
               coinbase_balance != prev.coinbase_balance ||
               shielded_balance != prev.shielded_balance ||
               unconfirmed_balance != prev.unconfirmed_balance ||
               unconfirmed_coinbase_balance != prev.unconfirmed_coinbase_balance ||
               unconfirmed_shielded_balance != prev.unconfirmed_shielded_balance ||
               immature_balance != prev.immature_balance ||
               immature_shielded_balance != prev.immature_shielded_balance ||
               watch_only_balance != prev.watch_only_balance ||
               watch_only_coinbase_balance != prev.watch_only_coinbase_balance ||
               watch_only_shielded_balance != prev.watch_only_shielded_balance ||
               unconfirmed_watch_only_balance != prev.unconfirmed_watch_only_balance ||
               unconfirmed_watch_only_coinbase_balance != prev.unconfirmed_watch_only_coinbase_balance ||
               unconfirmed_watch_only_shielded_balance != prev.unconfirmed_watch_only_shielded_balance ||
               immature_watch_only_balance != prev.immature_watch_only_balance ||
               immature_watch_only_shielded_balance != prev.immature_watch_only_shielded_balance;
    }
};

// Wallet transaction information.
struct WalletTx
{
    CTransactionRef tx;
    std::vector<isminetype> txin_is_mine;
    std::vector<isminetype> txout_is_mine;
    std::vector<bool> txout_is_change;
    std::vector<CTxDestination> txout_address;
    std::vector<isminetype> txout_address_is_mine;
    CAmount credit;
    CAmount debit;
    CAmount change;
    int64_t time;
    std::map<std::string, std::string> value_map;
    bool is_coinbase;
};

//! Updated transaction status.
struct WalletTxStatus
{
    int block_height;
    int blocks_to_maturity;
    int depth_in_main_chain;
    unsigned int time_received;
    uint32_t lock_time;
    bool is_final;
    bool is_trusted;
    bool is_abandoned;
    bool is_coinbase;
    bool is_in_main_chain;
};

//! Wallet transaction output.
struct WalletTxOut
{
    CTxOut txout;
    int64_t time;
    int depth_in_main_chain = -1;
    bool is_spent = false;
};

//! Wallet transaction sprout note.
struct WalletSproutNote
{
    libzcash::SproutPaymentAddress address;
    libzcash::SproutNote note;
    SproutOutPoint jsop;
    SproutNoteData nd;
    std::array<unsigned char, ZC_MEMO_SIZE> memo;
    int64_t time;
    int depth_in_main_chain = -1;
    bool is_spent = false;
};

//! Wallet transaction sapling note.
struct WalletSaplingNote
{
    libzcash::SaplingPaymentAddress address;
    libzcash::SaplingNote note;
    SaplingOutPoint op;
    SaplingNoteData nd;
    std::array<unsigned char, ZC_MEMO_SIZE> memo;
    int64_t time;
    int depth_in_main_chain = -1;
    bool is_spent = false;
};

//! Return implementation of Wallet interface. This function is defined in
//! dummywallet.cpp and throws if the wallet component is not compiled.
std::unique_ptr<Wallet> MakeWallet(const std::shared_ptr<CWallet>& wallet);

} // namespace interfaces

#endif // BITCOIN_INTERFACES_WALLET_H
