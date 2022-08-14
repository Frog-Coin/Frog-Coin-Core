// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "assert.h"

#include "chainparams.h"
#include "main.h"
#include "util.h"

#include <boost/assign/list_of.hpp>

using namespace boost::assign;

struct SeedSpec6 {
    uint8_t addr[16];
    uint16_t port;
};

#include "chainparamsseeds.h"

// Convert the pnSeeds6 array into usable address objects.
static void convertSeed6(std::vector<CAddress> &vSeedsOut, const SeedSpec6 *data, unsigned int count)
{
    // It'll only connect to one or two seed nodes because once it connects,
    // it'll get a pile of addresses with newer timestamps.
    // Seed nodes are given a random 'last seen time' of between one and two
    // weeks ago.
    const int64_t nOneWeek = 7*24*60*60;
    for (unsigned int i = 0; i < count; i++)
    {
        struct in6_addr ip;
        memcpy(&ip, data[i].addr, sizeof(ip));
        CAddress addr(CService(ip, data[i].port));
        addr.nTime = GetTime() - GetRand(nOneWeek) - nOneWeek;
        vSeedsOut.push_back(addr);
    }
}

//
// Main network
//
class CMainParams : public CChainParams {
public:
    CMainParams() {
        // The message start string is designed to be unlikely to occur in normal data.
        // The characters are rarely used upper ASCII, not valid as UTF-8, and produce
        // a large 4-byte int at any alignment.
        pchMessageStart[0] = 0xa7;
        pchMessageStart[1] = 0xf7;
        pchMessageStart[2] = 0x7c;
        pchMessageStart[3] = 0xaa;
        vAlertPubKey = ParseHex("44baae9dc249f123756b659c91a56897eba2efaacb6a192acdbef7894465f81f85d131aadfef3be6145678454852a2d08c6314bba5ca3cbe5616262da3b1a6fffa");
        nDefaultPort = 20995; // peer
        nRPCPort = 20925; // rpc
        bnProofOfWorkLimit = CBigNum(~uint256(0) >> 18);
        bnProofOfStakeLimit = CBigNum(~uint256(0) >> 18);

        const char* pszTimestamp = "Coinbase Faces SEC Probe on Crypto Listings; Shares Tumble | Allyson Versprille and Lydia Beyoud - Bloomberg | July 25th, 2022";
        std::vector<CTxIn> vin;
        vin.resize(1);
        vin[0].scriptSig = CScript() << 0 << CBigNum(42) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
        std::vector<CTxOut> vout;
        vout.resize(1);
        vout[0].nValue = 1 * COIN;
        vout[0].SetEmpty();
        CTransaction txNew(1, 1658711582, vin, vout, 0);
        genesis.vtx.push_back(txNew);
        genesis.hashPrevBlock = 0;
        genesis.hashMerkleRoot = genesis.BuildMerkleTree();
        genesis.nVersion = 1;
        genesis.nTime    = 1658711582;
        genesis.nBits    = bnProofOfWorkLimit.GetCompact();
        genesis.nNonce   = 800655;

        /** Genesis Block MainNet */
        /*
            Hashed MainNet Genesis Block Output
            block.hashMerkleRoot == e3d70353f235529d78322d496c82cf32487688f94ff4502d361452c171f1e181
            block.nTime = 1658711582
            block.nNonce = 800655
            block.GetHash = 000000a8ed8a69e60232b3de9890acd29c61c363845d01f030a8923d53d4f56d
        */

        hashGenesisBlock = genesis.GetHash();
        assert(hashGenesisBlock == uint256("0x000000a8ed8a69e60232b3de9890acd29c61c363845d01f030a8923d53d4f56d"));
        assert(genesis.hashMerkleRoot == uint256("0xe3d70353f235529d78322d496c82cf32487688f94ff4502d361452c171f1e181"));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,35);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,38);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,48);
        base58Prefixes[STEALTH_ADDRESS] = std::vector<unsigned char>(1,51);
        base58Prefixes[EXT_PUBLIC_KEY] = list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();

        // vSeeds.push_back(CDNSSeedData("node0",  "000.000.000.000"));


        convertSeed6(vFixedSeeds, pnSeed6_main, ARRAYLEN(pnSeed6_main));

        nPoolMaxTransactions = 9;
        strMNenginePoolDummyAddress = "FLKnRQZ5DYzRPF4S9kDAoapdrS3XTVj6qb";
        strDevOpsAddress = "F7CCbdxwD1VZNo8S3uCxUQZyhvNRx3nSmH";
        strPASfeeAddress = "F7CCbdxwD1VZNo8S3uCxUQZyhvNRx3nSmH";
        nEndPoWBlock = 0x7fffffff;
        nStartPoSBlock = 1;
    }

    virtual const CBlock& GenesisBlock() const { return genesis; }
    virtual Network NetworkID() const { return CChainParams::MAIN; }

    virtual const vector<CAddress>& FixedSeeds() const {
        return vFixedSeeds;
    }
protected:
    CBlock genesis;
    vector<CAddress> vFixedSeeds;
};
static CMainParams mainParams;


//
// Testnet
//
class CTestNetParams : public CMainParams {
public:
    CTestNetParams() {
        // The message start string is designed to be unlikely to occur in normal data.
        // The characters are rarely used upper ASCII, not valid as UTF-8, and produce
        // a large 4-byte int at any alignment.
        pchMessageStart[0] = 0xa9;
        pchMessageStart[1] = 0xf9;
        pchMessageStart[2] = 0x9c;
        pchMessageStart[3] = 0xdd;
        bnProofOfWorkLimit = CBigNum(~uint256(0) >> 15);
        bnProofOfStakeLimit = CBigNum(~uint256(0) >> 15);
        vAlertPubKey = ParseHex("44baae9dc489f996be6b659c91a518567ba2efaacb6acdefcdbef7894452f81f85d131aadfef3be6145678454852a2d08c6314bba5ca3cbe5616262da3b1a6aaab");
        nDefaultPort = 20099; // 
        nRPCPort = 20198; //
        strDataDir = "testnet";

        // Modify the testnet genesis block so the timestamp is valid for a later start.
        genesis.nTime  = 1658711592;
        genesis.nBits  = bnProofOfWorkLimit.GetCompact();
        genesis.nNonce = 1832;

        /** Genesis Block TestNet */
        /*
            Hashed TestNet Genesis Block Output
            block.hashMerkleRoot == e3d70353f235529d78322d496c82cf32487688f94ff4502d361452c171f1e181
            block.nTime = 1658711592
            block.nNonce = 1832
            block.GetHash = 0001fbb51564f958c2782b645d10f94f4cb72f0605a8feb247d9d3b17a30e9b6
        */

        hashGenesisBlock = genesis.GetHash();
        assert(hashGenesisBlock == uint256("0x0001fbb51564f958c2782b645d10f94f4cb72f0605a8feb247d9d3b17a30e9b6"));

        vFixedSeeds.clear();
        vSeeds.clear();

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,37); // F or G
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,42); //
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,39); //
        base58Prefixes[STEALTH_ADDRESS] = std::vector<unsigned char>(1,52); //
        base58Prefixes[EXT_PUBLIC_KEY] = list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();

        nEndPoWBlock = 0x7fffffff;
    }
    virtual Network NetworkID() const { return CChainParams::TESTNET; }
};
static CTestNetParams testNetParams;

//
// Regression test
//
class CRegTestParams : public CTestNetParams {
public:
    CRegTestParams() {
        pchMessageStart[0] = 0xa8;
        pchMessageStart[1] = 0xf8;
        pchMessageStart[2] = 0x8c;
        pchMessageStart[3] = 0xff;
        bnProofOfWorkLimit = CBigNum(~uint256(0) >> 1);
        genesis.nTime = 1658711682;
        genesis.nBits  = bnProofOfWorkLimit.GetCompact();
        genesis.nNonce = 8;
        hashGenesisBlock = genesis.GetHash();
        nDefaultPort = 20009; //
        strDataDir = "regtest";

        /** Genesis Block RegNet */
        /*
            Hashed RegNet Genesis Block Output
            block.hashMerkleRoot == e3d70353f235529d78322d496c82cf32487688f94ff4502d361452c171f1e181
            block.nTime = 1658711682
            block.nNonce = 8
            block.GetHash = 96946553aeea0b4fb64a5bbf0a5c7a41b4d81beec97e725666e5dd70bb9f300b
        */

        assert(hashGenesisBlock == uint256("0x96946553aeea0b4fb64a5bbf0a5c7a41b4d81beec97e725666e5dd70bb9f300b"));

        vSeeds.clear();  // Regtest mode doesn't have any DNS seeds.
    }

    virtual bool RequireRPCPassword() const { return false; }
    virtual Network NetworkID() const { return CChainParams::REGTEST; }
};
static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = &mainParams;

const CChainParams &Params() {
    return *pCurrentParams;
}

void SelectParams(CChainParams::Network network) {
    switch (network) {
        case CChainParams::MAIN:
            pCurrentParams = &mainParams;
            break;
        case CChainParams::TESTNET:
            pCurrentParams = &testNetParams;
            break;
        case CChainParams::REGTEST:
            pCurrentParams = &regTestParams;
            break;
        default:
            assert(false && "Unimplemented network");
            return;
    }
}

bool SelectParamsFromCommandLine() {
    bool fRegTest = GetBoolArg("-regtest", false);
    bool fTestNet = GetBoolArg("-testnet", false);

    if (fTestNet && fRegTest) {
        return false;
    }

    if (fRegTest) {
        SelectParams(CChainParams::REGTEST);
    } else if (fTestNet) {
        SelectParams(CChainParams::TESTNET);
    } else {
        SelectParams(CChainParams::MAIN);
    }
    return true;
}
