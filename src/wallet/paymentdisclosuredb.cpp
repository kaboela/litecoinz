// Copyright (c) 2017 The Zcash developers
// Copyright (c) 2017-2020 The LitecoinZ Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#include <fs.h>
#include <wallet/paymentdisclosuredb.h>

#include <util/system.h>
#include <dbwrapper.h>

static fs::path emptyPath;

/**
 * Static method to return the shared/default payment disclosure database.
 */
std::shared_ptr<PaymentDisclosureDB> PaymentDisclosureDB::sharedInstance() {
    // Thread-safe in C++11 and gcc 4.3
    static std::shared_ptr<PaymentDisclosureDB> ptr = std::make_shared<PaymentDisclosureDB>();
    return ptr;
}

// C++11 delegated constructor
PaymentDisclosureDB::PaymentDisclosureDB() : PaymentDisclosureDB(emptyPath) {
}

PaymentDisclosureDB::PaymentDisclosureDB(const fs::path& dbPath) {
    fs::path path(dbPath);
    if (path.empty()) {
        path = GetDataDir() / "paymentdisclosure";
        LogPrintf("%s: using default path for database: %s\n", __func__, path.string());
    } else {
        LogPrintf("%s: using custom path for database: %s\n", __func__, path.string());
    }

    TryCreateDirectories(path);
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, path.string(), &db);
    dbwrapper_private::HandleError(status); // throws exception
    LogPrintf("%s: Opened LevelDB successfully\n", __func__);
}

PaymentDisclosureDB::~PaymentDisclosureDB() {
    if (db != nullptr) {
        delete db;
    }
}

bool PaymentDisclosureDB::Put(const PaymentDisclosureKey& key, const PaymentDisclosureInfo& info)
{
    if (db == nullptr) {
        return false;
    }

    std::lock_guard<std::mutex> guard(lock_);

    CDataStream ssValue(SER_DISK, CLIENT_VERSION);
    ssValue.reserve(GetSerializeSize(info, ssValue.GetVersion()));
    ssValue << info;
    leveldb::Slice slice(&ssValue[0], ssValue.size());

    leveldb::Status status = db->Put(writeOptions, key.ToString(), slice);
    dbwrapper_private::HandleError(status);
    return true;
}

bool PaymentDisclosureDB::Get(const PaymentDisclosureKey& key, PaymentDisclosureInfo& info)
{
    if (db == nullptr) {
        return false;
    }

    std::lock_guard<std::mutex> guard(lock_);

    std::string strValue;
    leveldb::Status status = db->Get(readOptions, key.ToString(), &strValue);
    if (!status.ok()) {
        if (status.IsNotFound())
            return false;
        LogPrintf("%s: LevelDB read failure: %s\n", __func__, status.ToString());
        dbwrapper_private::HandleError(status);
    }

    try {
        CDataStream ssValue(strValue.data(), strValue.data() + strValue.size(), SER_DISK, CLIENT_VERSION);
        ssValue >> info;
    } catch (const std::exception&) {
        return false;
    }
    return true;
}
