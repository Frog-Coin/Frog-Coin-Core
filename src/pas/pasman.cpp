#include "pasman.h"
#include "pas.h"
#include "pasengine.h"
#include "chain.h"
#include "util.h"
#include "addrman.h"
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>


/** Pubkeyaliasservice manager */
CPubkeyaliasserviceMan paserviceman;
CCriticalSection cs_process_messagePAS;

//
// CPubkeyaliasserviceDB
//

CPubkeyaliasserviceDB::CPubkeyaliasserviceDB()
{
    pathPAS = GetDataDir() / "pascache.dat";
    strMagicMessage = "PubkeyaliasserviceCache";
}

bool CPubkeyaliasserviceDB::Write(const CPubkeyaliasserviceMan& paservicemanToSave)
{
    int64_t nStart = GetTimeMillis();

    // serialize addresses, checksum data up to that point, then append csum
    CDataStream ssPubkeyaliasservices(SER_DISK, CLIENT_VERSION);
    ssPubkeyaliasservices << strMagicMessage; // pubkeyaliasservice cache file specific magic message
    ssPubkeyaliasservices << FLATDATA(Params().MessageStart()); // network specific magic number
    ssPubkeyaliasservices << paservicemanToSave;
    uint256 hash = Hash(ssPubkeyaliasservices.begin(), ssPubkeyaliasservices.end());
    ssPubkeyaliasservices << hash;

    // open output file, and associate with CAutoFile
    FILE *file = fopen(pathPAS.string().c_str(), "wb");
    CAutoFile fileout = CAutoFile(file, SER_DISK, CLIENT_VERSION);
    if (!fileout)
        return error("%s : Failed to open file %s", __func__, pathPAS.string());

    // Write and commit header, data
    try {
        fileout << ssPubkeyaliasservices;
    }
    catch (std::exception &e) {
        return error("%s : Serialize or I/O error - %s", __func__, e.what());
    }
    FileCommit(fileout);
    fileout.fclose();

    LogPrintf("Written info to pascache.dat  %dms\n", GetTimeMillis() - nStart);
    LogPrintf("  %s\n", paservicemanToSave.ToString());

    return true;
}

CPubkeyaliasserviceDB::ReadResult CPubkeyaliasserviceDB::Read(CPubkeyaliasserviceMan& paservicemanToLoad)
{
    int64_t nStart = GetTimeMillis();
    // open input file, and associate with CAutoFile
    FILE *file = fopen(pathPAS.string().c_str(), "rb");
    CAutoFile filein = CAutoFile(file, SER_DISK, CLIENT_VERSION);
    if (!filein)
    {
        error("%s : Failed to open file %s", __func__, pathPAS.string());
        return FileError;
    }

    // use file size to size memory buffer
    int fileSize = boost::filesystem::file_size(pathPAS);
    int dataSize = fileSize - sizeof(uint256);
    // Don't try to resize to a negative number if file is small
    if (dataSize < 0)
        dataSize = 0;
    vector<unsigned char> vchData;
    vchData.resize(dataSize);
    uint256 hashIn;

    // read data and checksum from file
    try {
        filein.read((char *)&vchData[0], dataSize);
        filein >> hashIn;
    }
    catch (std::exception &e) {
        error("%s : Deserialize or I/O error - %s", __func__, e.what());
        return HashReadError;
    }
    filein.fclose();

    CDataStream ssPubkeyaliasservices(vchData, SER_DISK, CLIENT_VERSION);

    // verify stored checksum matches input data
    uint256 hashTmp = Hash(ssPubkeyaliasservices.begin(), ssPubkeyaliasservices.end());
    if (hashIn != hashTmp)
    {
        error("%s : Checksum mismatch, data corrupted", __func__);
        return IncorrectHash;
    }

    unsigned char pchMsgTmp[4];
    std::string strMagicMessageTmp;
    try {
        // de-serialize file header (pubkeyaliasservice cache file specific magic message) and ..

        ssPubkeyaliasservices >> strMagicMessageTmp;

        // ... verify the message matches predefined one
        if (strMagicMessage != strMagicMessageTmp)
        {
            error("%s : Invalid pubkeyaliasservice cache magic message", __func__);
            return IncorrectMagicMessage;
        }

        // de-serialize file header (network specific magic number) and ..
        ssPubkeyaliasservices >> FLATDATA(pchMsgTmp);

        // ... verify the network matches ours
        if (memcmp(pchMsgTmp, Params().MessageStart(), sizeof(pchMsgTmp)))
        {
            error("%s : Invalid network magic number", __func__);
            return IncorrectMagicNumber;
        }

        // de-serialize address data into one CPasList object
        ssPubkeyaliasservices >> paservicemanToLoad;
    }
    catch (std::exception &e) {
        paservicemanToLoad.ClearPAS();
        error("%s : Deserialize or I/O error - %s", __func__, e.what());
        return IncorrectFormat;
    }

    paservicemanToLoad.CheckAndRemovePAS(); // clean out expired
    LogPrintf("Loaded info from pascache.dat  %dms\n", GetTimeMillis() - nStart);
    LogPrintf("  %s\n", paservicemanToLoad.ToString());

    return Ok;
}

void DumpPubkeyaliasservices()
{
    int64_t nStart = GetTimeMillis();

    CPubkeyaliasserviceDB pasdb;
    CPubkeyaliasserviceMan tempPAserviceman;

    LogPrintf("Verifying pascache.dat format...\n");
    CPubkeyaliasserviceDB::ReadResult readResult = pasdb.Read(tempPAserviceman);
    // there was an error and it was not an error on file openning => do not proceed
    if (readResult == CPubkeyaliasserviceDB::FileError)
        LogPrintf("Missing pubkeyaliasservice list file - pascache.dat, will try to recreate\n");
    else if (readResult != CPubkeyaliasserviceDB::Ok)
    {
        LogPrintf("Error reading pascache.dat: ");
        if(readResult == CPubkeyaliasserviceDB::IncorrectFormat)
            LogPrintf("magic is ok but data has invalid format, will try to recreate\n");
        else
        {
            LogPrintf("file format is unknown or invalid, please fix it manually\n");
            return;
        }
    }
    LogPrintf("Writting info to pascache.dat...\n");
    pasdb.Write(paserviceman);

    LogPrintf("Pubkeyaliasservice dump finished  %dms\n", GetTimeMillis() - nStart);
}

CPubkeyaliasserviceMan::CPubkeyaliasserviceMan() {
    // Do nothing
}

bool CPubkeyaliasserviceMan::AddPAS(CPubkeyaliasservice &pas)
{
    LOCK(cs);

    if (!pas.IsEnabled())
        return false;

    CPubkeyaliasservice *ppas = Find(pas.vin);

    if (ppas == NULL)
    {
        LogPrint("pubkeyaliasservice", "CPubkeyaliasserviceMan: Adding new pubkeyaliasservice %s - %i now\n", pas.addr.ToString().c_str(), size() + 1);
        vPubkeyaliasservices.push_back(pas);
        return true;
    }

    return false;
}

void CPubkeyaliasserviceMan::AskForPAS(CNode* pnode, CTxIn &vin)
{
    std::map<COutPoint, int64_t>::iterator i = mWeAskedForPubkeyaliasserviceListEntry.find(vin.prevout);
    if (i != mWeAskedForPubkeyaliasserviceListEntry.end())
    {
        int64_t t = (*i).second;
        if (GetTime() < t) return; // we've asked recently
    }

    // ask for the pasb info once from the node that sent pasp

    LogPrintf("CPubkeyaliasserviceMan::AskForPAS - Asking node for missing entry, vin: %s\n", vin.ToString());
    pnode->PushMessage("pasg", vin);
    int64_t askAgain = GetTime() + PUBKEYALIASSERVICES_QUEUE_SECONDS;
    mWeAskedForPubkeyaliasserviceListEntry[vin.prevout] = askAgain;
}

void CPubkeyaliasserviceMan::CheckPAS()
{
    LOCK(cs);

    BOOST_FOREACH(CPubkeyaliasservice& pas, vPubkeyaliasservices)
        pas.Check();
}

void CPubkeyaliasserviceMan::CheckAndRemovePAS()
{
    LOCK(cs);

    CheckPAS();

    //remove inactive
    vector<CPubkeyaliasservice>::iterator it = vPubkeyaliasservices.begin();
    while(it != vPubkeyaliasservices.end()){
        if((*it).activeStatePAS == CPubkeyaliasservice::PUBKEYALIASSERVICE_REMOVE || (*it).activeStatePAS == CPubkeyaliasservice::PUBKEYALIASSERVICE_VIN_ERROR || (*it).protocolVersion < nPubkeyaliasserviceMinProtocol){
            LogPrint("pubkeyaliasservice", "CPubkeyaliasserviceMan: Removing inactive pubkeyaliasservice %s - %i now\n", (*it).addr.ToString().c_str(), size() - 1);
            it = vPubkeyaliasservices.erase(it);
        } else {
            ++it;
        }
    }

    // check who's asked for the pubkeyaliasservice list
    map<CNetAddr, int64_t>::iterator it1 = mAskedUsForPubkeyaliasserviceList.begin();
    while(it1 != mAskedUsForPubkeyaliasserviceList.end()){
        if((*it1).second < GetTime()) {
            mAskedUsForPubkeyaliasserviceList.erase(it1++);
        } else {
            ++it1;
        }
    }

    // check who we asked for the pubkeyaliasservice list
    it1 = mWeAskedForPubkeyaliasserviceList.begin();
    while(it1 != mWeAskedForPubkeyaliasserviceList.end()){
        if((*it1).second < GetTime()){
            mWeAskedForPubkeyaliasserviceList.erase(it1++);
        } else {
            ++it1;
        }
    }

    // check which pubkeyaliasservices we've asked for
    map<COutPoint, int64_t>::iterator it2 = mWeAskedForPubkeyaliasserviceListEntry.begin();
    while(it2 != mWeAskedForPubkeyaliasserviceListEntry.end()){
        if((*it2).second < GetTime()){
            mWeAskedForPubkeyaliasserviceListEntry.erase(it2++);
        } else {
            ++it2;
        }
    }

}

void CPubkeyaliasserviceMan::ClearPAS()
{
    LOCK(cs);
    vPubkeyaliasservices.clear();
    mAskedUsForPubkeyaliasserviceList.clear();
    mWeAskedForPubkeyaliasserviceList.clear();
    mWeAskedForPubkeyaliasserviceListEntry.clear();
}

int CPubkeyaliasserviceMan::CountEnabledPAS()
{
    int i = 0;

    BOOST_FOREACH(CPubkeyaliasservice& pas, vPubkeyaliasservices) {
        pas.Check();
        if((pas.regTime + 31600000) < GetTime()) continue;
        i++;
    }

    return i;
}

int CPubkeyaliasserviceMan::CountPubkeyaliasservicesAboveProtocolPAS(int minprotocolVersion)
{
    int i = 0;

    BOOST_FOREACH(CPubkeyaliasservice& pas, vPubkeyaliasservices) {
        pas.Check();
        if(pas.protocolVersion < minprotocolVersion || !pas.IsEnabled()) continue;
        i++;
    }

    return i;
}

CPubkeyaliasservice *CPubkeyaliasserviceMan::Find(const CTxIn &vin)
{
    LOCK(cs);

    BOOST_FOREACH(CPubkeyaliasservice& pas, vPubkeyaliasservices)
    {
        if(pas.vin.prevout == vin.prevout)
            return &pas;
    }
    return NULL;
}

CPubkeyaliasservice* CPubkeyaliasserviceMan::FindOldestNotInVec(const std::vector<CTxIn> &vVins, int nMinimumAge)
{
    LOCK(cs);

    CPubkeyaliasservice *pOldestPubkeyaliasservice = NULL;

    BOOST_FOREACH(CPubkeyaliasservice &pas, vPubkeyaliasservices)
    {   
        pas.Check();
        if(!pas.IsEnabled()) continue;

        if(pas.GetPubkeyaliasserviceInputAge() < nMinimumAge) continue;

        bool found = false;
        BOOST_FOREACH(const CTxIn& vin, vVins)
            if(pas.vin.prevout == vin.prevout)
            {   
                found = true;
                break;
            }
        if(found) continue;

        if(pOldestPubkeyaliasservice == NULL || pOldestPubkeyaliasservice->regTime < pas.regTime)
        {
            pOldestPubkeyaliasservice = &pas;
        }
    }

    return pOldestPubkeyaliasservice;
}

void CPubkeyaliasserviceMan::RemovePAS(CTxIn vin)
{
    LOCK(cs);

    vector<CPubkeyaliasservice>::iterator it = vPubkeyaliasservices.begin();
    while(it != vPubkeyaliasservices.end()){
        if((*it).vin == vin){
            LogPrint("pubkeyaliasservice", "CPubkeyaliasserviceMan: Removing Pubkeyaliasservice %s - %i now\n", (*it).addr.ToString().c_str(), size() - 1);
            vPubkeyaliasservices.erase(it);
            break;
        }
    }
}

std::string CPubkeyaliasserviceMan::ToString() const
{
    std::ostringstream info;

    info << "pubkeyaliasservices: " << (int)vPubkeyaliasservices.size() <<
            ", peers who asked us for pubkeyaliasservice list: " << (int)mAskedUsForPubkeyaliasserviceList.size() <<
            ", peers we asked for pubkeyaliasservice list: " << (int)mWeAskedForPubkeyaliasserviceList.size() <<
            ", entries in Pubkeyaliasservice list we asked for: " << (int)mWeAskedForPubkeyaliasserviceListEntry.size();

    return info.str();
}
