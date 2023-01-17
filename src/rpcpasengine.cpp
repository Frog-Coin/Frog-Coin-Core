// Copyright (c) 2022-2023 The CryptoCoderz Team / Espers project
// Copyright (c) 2022-2023 The FrogCoin Project
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "main.h"
#include "chain.h"
#include "db.h"
#include "init.h"
//
//TODO: add pas registration handling...
// (Originally activemasternode.cpp)
//#include "pas/pasreg.h"
//
#include "pas/pasman.h"
//
//TODO: add pas config handling...
//#include "pas/pasconfig.h"
//
#include "rpcserver.h"
#include <boost/lexical_cast.hpp>
#include "util.h"

#include <fstream>
using namespace json_spirit;
using namespace std;


Value pubkeyaliasservice(const Array& params, bool fHelp)
{
    string strCommand;
    if (params.size() >= 1)
        strCommand = params[0].get_str();

    if (fHelp  ||
        (strCommand != "count" && strCommand != "current" && strCommand != "debug" && strCommand != "genkey" && strCommand != "enforce" && strCommand != "list" && strCommand != "list-conf"
        	&& strCommand != "start" && strCommand != "start-alias" && strCommand != "start-many" && strCommand != "status" && strCommand != "stop" && strCommand != "stop-alias"
                && strCommand != "stop-many" && strCommand != "winners" && strCommand != "connect" && strCommand != "outputs" && strCommand != "vote-many" && strCommand != "vote"))
        throw runtime_error(
                "pubkeyaliasservice \"command\"... ( \"passphrase\" )\n"
                "Set of commands to execute pubkeyaliasservice related actions\n"
                "\nArguments:\n"
                "1. \"command\"        (string or set of strings, required) The command to execute\n"
                "2. \"passphrase\"     (string, optional) The wallet passphrase\n"
                "\nAvailable commands:\n"
                "  count        - Print number of all known pubkeyaliasservices (optional: 'enabled', 'both')\n"
                "  debug        - Print pubkeyaliasservice status\n"
                "  genkey       - Generate new pubkeyaliasservice privkey\n"
                "  enforce      - Enforce pubkeyaliasservice versioning\n"
                "  list         - Print list of all known pubkeyaliasservices (see pubkeyaliasservicelist for more info)\n"
                "  list-conf    - Print pubkeyaliasservice.conf in JSON format\n"
                "  outputs      - Print pubkeyaliasservice compatible outputs\n"
                "  start        - Start pubkeyaliasservice configured in FrogCoin.conf\n"
                "  start-alias  - Start single pubkeyaliasservice by assigned alias configured in pubkeyaliasservice.conf\n"
                "  start-many   - Start all pubkeyaliasservices configured in pubkeyaliasservice.conf\n"
                "  status       - Print pubkeyaliasservice status information\n"
                "  stop         - Stop pubkeyaliasservice configured in FrogCoin.conf\n"
                "  stop-alias   - Stop single pubkeyaliasservice by assigned alias configured in pubkeyaliasservice.conf\n"
                "  stop-many    - Stop all pubkeyaliasservices configured in pubkeyaliasservice.conf\n"
                );

    if (strCommand == "stop")
    {
        if(!fMasterNode) return "you must set pubkeyaliasservice=1 in the configuration";

        if(pwalletMain->IsLocked()) {
            SecureString strWalletPass;
            strWalletPass.reserve(100);

            if (params.size() == 2){
                strWalletPass = params[1].get_str().c_str();
            } else {
                throw runtime_error(
                    "Your wallet is locked, passphrase is required\n");
            }

            if(!pwalletMain->Unlock(strWalletPass)){
                return "incorrect passphrase";
            }
        }

        std::string errorMessage;
        //if(!activeMasternode.StopMasterNode(errorMessage)) {
        //	return "stop failed: " + errorMessage;
        //}
        pwalletMain->Lock();

        //if(activeMasternode.status == MASTERNODE_STOPPED) return "successfully stopped pubkeyaliasservice";
        //if(activeMasternode.status == MASTERNODE_NOT_CAPABLE) return "not capable pubkeyaliasservice";

        return "unknown";
    }

    if (strCommand == "stop-alias")
    {
	    if (params.size() < 2){
			throw runtime_error(
			"command needs at least 2 parameters\n");
	    }

	    std::string alias = params[1].get_str().c_str();

    	if(pwalletMain->IsLocked()) {
    		SecureString strWalletPass;
    	    strWalletPass.reserve(100);

			if (params.size() == 3){
				strWalletPass = params[2].get_str().c_str();
			} else {
				throw runtime_error(
				"Your wallet is locked, passphrase is required\n");
			}

			if(!pwalletMain->Unlock(strWalletPass)){
				return "incorrect passphrase";
			}
        }

    	bool found = false;

		Object statusObj;
		statusObj.push_back(Pair("alias", alias));
        /*
    	BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, pubkeyaliasserviceConfig.getEntries()) {
    		if(mne.getAlias() == alias) {
    			found = true;
    			std::string errorMessage;
    			bool result = activeMasternode.StopMasterNode(mne.getIp(), mne.getPrivKey(), errorMessage);

				statusObj.push_back(Pair("result", result ? "successful" : "failed"));
    			if(!result) {
   					statusObj.push_back(Pair("errorMessage", errorMessage));
   				}
    			break;
    		}
    	}
        */
    	if(!found) {
    		statusObj.push_back(Pair("result", "failed"));
    		statusObj.push_back(Pair("errorMessage", "could not find alias in config. Verify with list-conf."));
    	}

    	pwalletMain->Lock();
    	return statusObj;
    }
    /*
    if (strCommand == "stop-many")
    {
    	if(pwalletMain->IsLocked()) {
			SecureString strWalletPass;
			strWalletPass.reserve(100);

			if (params.size() == 2){
				strWalletPass = params[1].get_str().c_str();
			} else {
				throw runtime_error(
				"Your wallet is locked, passphrase is required\n");
			}

			if(!pwalletMain->Unlock(strWalletPass)){
				return "incorrect passphrase";
			}
		}

		int total = 0;
		int successful = 0;
		int fail = 0;


		Object resultsObj;

		BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, pubkeyaliasserviceConfig.getEntries()) {
			total++;

			std::string errorMessage;
			bool result = activeMasternode.StopMasterNode(mne.getIp(), mne.getPrivKey(), errorMessage);

			Object statusObj;
			statusObj.push_back(Pair("alias", mne.getAlias()));
			statusObj.push_back(Pair("result", result ? "successful" : "failed"));

			if(result) {
				successful++;
			} else {
				fail++;
				statusObj.push_back(Pair("errorMessage", errorMessage));
			}

			resultsObj.push_back(Pair("status", statusObj));
		}
		pwalletMain->Lock();

		Object returnObj;
		returnObj.push_back(Pair("overall", "Successfully stopped " + boost::lexical_cast<std::string>(successful) + " pubkeyaliasservices, failed to stop " +
				boost::lexical_cast<std::string>(fail) + ", total " + boost::lexical_cast<std::string>(total)));
		returnObj.push_back(Pair("detail", resultsObj));

		return returnObj;

    }
    */
    if (strCommand == "list")
    {
        Array newParams(params.size() - 1);
        std::copy(params.begin() + 1, params.end(), newParams.begin());
        return pubkeyaliasservicelist(newParams, fHelp);
    }

    if (strCommand == "count")
    {
        if (params.size() > 2){
            throw runtime_error(
                "too many parameters\n");
        }

        if (params.size() == 2)
        {
            if(params[1] == "enabled") return paserviceman.CountEnabledPAS();
            if(params[1] == "both") return boost::lexical_cast<std::string>(paserviceman.CountEnabledPAS()) + " / " + boost::lexical_cast<std::string>(paserviceman.size());
        }
        return paserviceman.size();
    }
    /*
    if (strCommand == "start")
    {
        if(!fMasterNode) return "you must set pubkeyaliasservice=1 in the configuration";

        if(pwalletMain->IsLocked()) {
            SecureString strWalletPass;
            strWalletPass.reserve(100);

            if (params.size() == 2){
                strWalletPass = params[1].get_str().c_str();
            } else {
                throw runtime_error(
                    "Your wallet is locked, passphrase is required\n");
            }

            if(!pwalletMain->Unlock(strWalletPass)){
                return "incorrect passphrase";
            }
        }

        if(activeMasternode.status != MASTERNODE_REMOTELY_ENABLED && activeMasternode.status != MASTERNODE_IS_CAPABLE){
            activeMasternode.status = MASTERNODE_NOT_PROCESSED; // TODO: consider better way
            std::string errorMessage;
            activeMasternode.ManageStatus();
            pwalletMain->Lock();
        }

        if(activeMasternode.status == MASTERNODE_REMOTELY_ENABLED) return "pubkeyaliasservice started remotely";
        if(activeMasternode.status == MASTERNODE_INPUT_TOO_NEW) return "pubkeyaliasservice input must have at least 15 confirmations";
        if(activeMasternode.status == MASTERNODE_STOPPED) return "pubkeyaliasservice is stopped";
        if(activeMasternode.status == MASTERNODE_IS_CAPABLE) return "successfully started pubkeyaliasservice";
        if(activeMasternode.status == MASTERNODE_NOT_CAPABLE) return "not capable pubkeyaliasservice: " + activeMasternode.notCapableReason;
        if(activeMasternode.status == MASTERNODE_SYNC_IN_PROCESS) return "sync in process. Must wait until client is synced to start.";

        return "unknown";
    }
    */
    /*
    if (strCommand == "start-alias")
    {
	    if (params.size() < 2){
			throw runtime_error(
			"command needs at least 2 parameters\n");
	    }

	    std::string alias = params[1].get_str().c_str();

    	if(pwalletMain->IsLocked()) {
    		SecureString strWalletPass;
    	    strWalletPass.reserve(100);

			if (params.size() == 3){
				strWalletPass = params[2].get_str().c_str();
			} else {
				throw runtime_error(
				"Your wallet is locked, passphrase is required\n");
			}

			if(!pwalletMain->Unlock(strWalletPass)){
				return "incorrect passphrase";
			}
        }

    	bool found = false;

		Object statusObj;
		statusObj.push_back(Pair("alias", alias));

    	BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, pubkeyaliasserviceConfig.getEntries()) {
    		if(mne.getAlias() == alias) {
    			found = true;
    			std::string errorMessage;
                std::string strDonateAddress = "";
                std::string strDonationPercentage = "";

                bool result = activeMasternode.Register(mne.getIp(), mne.getPrivKey(), mne.getTxHash(), mne.getOutputIndex(), strDonateAddress, strDonationPercentage, errorMessage);
  
                statusObj.push_back(Pair("result", result ? "successful" : "failed"));
    			if(!result) {
					statusObj.push_back(Pair("errorMessage", errorMessage));
				}
    			break;
    		}
    	}

    	if(!found) {
    		statusObj.push_back(Pair("result", "failed"));
    		statusObj.push_back(Pair("errorMessage", "could not find alias in config. Verify with list-conf."));
    	}

    	pwalletMain->Lock();
    	return statusObj;

    }
    */
    /*
    if (strCommand == "start-many")
    {
    	if(pwalletMain->IsLocked()) {
			SecureString strWalletPass;
			strWalletPass.reserve(100);

			if (params.size() == 2){
				strWalletPass = params[1].get_str().c_str();
			} else {
				throw runtime_error(
				"Your wallet is locked, passphrase is required\n");
			}

			if(!pwalletMain->Unlock(strWalletPass)){
				return "incorrect passphrase";
			}
		}

		std::vector<CMasternodeConfig::CMasternodeEntry> mnEntries;
		mnEntries = pubkeyaliasserviceConfig.getEntries();

		int total = 0;
		int successful = 0;
		int fail = 0;

		Object resultsObj;

		BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, pubkeyaliasserviceConfig.getEntries()) {
			total++;

			std::string errorMessage;
            std::string strDonateAddress = "";
            std::string strDonationPercentage = "";

            bool result = activeMasternode.Register(mne.getIp(), mne.getPrivKey(), mne.getTxHash(), mne.getOutputIndex(), strDonateAddress, strDonationPercentage, errorMessage);

			Object statusObj;
			statusObj.push_back(Pair("alias", mne.getAlias()));
			statusObj.push_back(Pair("result", result ? "succesful" : "failed"));

			if(result) {
				successful++;
			} else {
				fail++;
				statusObj.push_back(Pair("errorMessage", errorMessage));
			}

			resultsObj.push_back(Pair("status", statusObj));
		}
		pwalletMain->Lock();

		Object returnObj;
		returnObj.push_back(Pair("overall", "Successfully started " + boost::lexical_cast<std::string>(successful) + " pubkeyaliasservices, failed to start " +
				boost::lexical_cast<std::string>(fail) + ", total " + boost::lexical_cast<std::string>(total)));
		returnObj.push_back(Pair("detail", resultsObj));

		return returnObj;
    }
    */
    if (strCommand == "debug")
    {
        //if(activeMasternode.status == MASTERNODE_REMOTELY_ENABLED) return "pubkeyaliasservice started remotely";
        //if(activeMasternode.status == MASTERNODE_INPUT_TOO_NEW) return "pubkeyaliasservice input must have at least 15 confirmations";
        //if(activeMasternode.status == MASTERNODE_IS_CAPABLE) return "successfully started pubkeyaliasservice";
        //if(activeMasternode.status == MASTERNODE_STOPPED) return "pubkeyaliasservice is stopped";
        //if(activeMasternode.status == MASTERNODE_NOT_CAPABLE) return "not capable pubkeyaliasservice: " + activeMasternode.notCapableReason;
        //if(activeMasternode.status == MASTERNODE_SYNC_IN_PROCESS) return "sync in process. Must wait until client is synced to start.";

        CTxIn vin = CTxIn();
        //CPubKey pubkey = CScript();
        CKey key;
        //TODO: engage active checks once pas registration is done...
        bool found = false;//activeMasternode.GetMasterNodeVin(vin, pubkey, key);
        if(!found){
            return "Missing pubkeyaliasservice input, please look at the documentation for instructions on pubkeyaliasservice creation";
        } else {
            return "No problems were found";
        }
    }

    if (strCommand == "create")
    {

        return "Not implemented yet, please look at the documentation for instructions on pubkeyaliasservice creation";
    }

    if (strCommand == "genkey")
    {
        CKey secret;
        secret.MakeNewKey(false);

        return CFrogCoinSecret(secret).ToString();
    }

    if(strCommand == "enforce")
    {
        return "Not implemented yet, please look at the documentation for instructions on pubkeyaliasservice version enforcement";
    }

    /*
    if(strCommand == "list-conf")
    {
    	std::vector<CMasternodeConfig::CMasternodeEntry> mnEntries;
    	mnEntries = pubkeyaliasserviceConfig.getEntries();

        Object resultObj;

        BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, pubkeyaliasserviceConfig.getEntries()) {
    		Object mnObj;
    		mnObj.push_back(Pair("alias", mne.getAlias()));
    		mnObj.push_back(Pair("address", mne.getIp()));
    		mnObj.push_back(Pair("privateKey", mne.getPrivKey()));
    		mnObj.push_back(Pair("txHash", mne.getTxHash()));
    		mnObj.push_back(Pair("outputIndex", mne.getOutputIndex()));
    		resultObj.push_back(Pair("pubkeyaliasservice", mnObj));
    	}

    	return resultObj;
    }

    if (strCommand == "outputs"){
        // Find possible candidates
        vector<COutput> possibleCoins = activeMasternode.SelectCoinsMasternode();

        Object obj;
        BOOST_FOREACH(COutput& out, possibleCoins) {
            obj.push_back(Pair(out.tx->GetHash().ToString().c_str(), boost::lexical_cast<std::string>(out.i)));
        }

        return obj;

    }

    if(strCommand == "status")
    {
        std::vector<CMasternodeConfig::CMasternodeEntry> mnEntries;
        mnEntries = pubkeyaliasserviceConfig.getEntries();

        CScript pubkey;
        pubkey = GetScriptForDestination(activeMasternode.pubKeyMasternode.GetID());
        CTxDestination address1;
        ExtractDestination(pubkey, address1);
        CFrogCoinAddress address2(address1);

        Object mnObj;
        CMasternode *pmn = mnodeman.Find(activeMasternode.vin);

        mnObj.push_back(Pair("vin", activeMasternode.vin.ToString().c_str()));
        mnObj.push_back(Pair("service", activeMasternode.service.ToString().c_str()));
        mnObj.push_back(Pair("status", activeMasternode.status));
        //mnObj.push_back(Pair("pubKeyMasternode", address2.ToString().c_str()));
        if (pmn) mnObj.push_back(Pair("pubkey", CFrogCoinAddress(pmn->pubkey.GetID()).ToString()));
        mnObj.push_back(Pair("notCapableReason", activeMasternode.notCapableReason.c_str()));

        return mnObj;
    }
    */
    return Value::null;
}

Value pubkeyaliasservicelist(const Array& params, bool fHelp)
{
    std::string strMode = "status";
    std::string strFilter = "";

    if (params.size() >= 1) strMode = params[0].get_str();
    if (params.size() == 2) strFilter = params[1].get_str();

    if (fHelp ||
            (strMode != "full" && strMode != "protocol"
                && strMode != "pubkey" && strMode != "status"))
    {
        throw runtime_error(
                "pubkeyaliasservicelist ( \"mode\" \"filter\" )\n"
                "Get a list of pubkeyaliasservices in different modes\n"
                "\nArguments:\n"
                "1. \"mode\"      (string, optional/required to use filter, defaults = status) The mode to run list in\n"
                "2. \"filter\"    (string, optional) Filter results. Partial match by IP by default in all modes, additional matches in some modes\n"
                "\nAvailable modes:\n"
                "  full           - Print info in format 'status protocol pubkey vin lastseen activeseconds' (can be additionally filtered, partial match)\n"
                "  protocol       - Print protocol of a pubkeyaliasservice (can be additionally filtered, exact match)\n"
                "  pubkey         - Print public key associated with a pubkeyaliasservice (can be additionally filtered, partial match)\n"
                "  status         - Print pubkeyaliasservice status: ENABLED / EXPIRED / VIN_SPENT / REMOVE / POS_ERROR (can be additionally filtered, partial match)\n"
                );
    }

    Object obj;
//
        std::vector<CPubkeyaliasservice> vPubkeyaliasservices = paserviceman.GetFullPubkeyaliasserviceVector();
        BOOST_FOREACH(CPubkeyaliasservice& pas, vPubkeyaliasservices) {
            std::string strVin = pas.vin.prevout.ToStringShort();
            if (strMode == "full") {
                CScript pubkey;
                pubkey.SetDestination(pas.pubkey.GetID());
                CTxDestination address1;
                ExtractDestination(pubkey, address1);
                CFrogCoinAddress address2(address1);

                std::ostringstream addrStream;
                addrStream << setw(21) << strVin;

                std::ostringstream stringStream;
                stringStream << setw(10) <<
                               pas.Status() << " " <<
                               pas.protocolVersion << " " <<
                               address2.ToString();
                std::string output = stringStream.str();
                stringStream << " " << strVin;
                if(strFilter !="" && stringStream.str().find(strFilter) == string::npos &&
                        strVin.find(strFilter) == string::npos) continue;
                obj.push_back(Pair(addrStream.str(), output));
            } else if (strMode == "protocol") {
                if(strFilter !="" && strFilter != boost::lexical_cast<std::string>(pas.protocolVersion) &&
                    strVin.find(strFilter) == string::npos) continue;
                obj.push_back(Pair(strVin,       (int64_t)pas.protocolVersion));
            } else if (strMode == "pubkey") {
                CScript pubkey;
                pubkey.SetDestination(pas.pubkey.GetID());
                CTxDestination address1;
                ExtractDestination(pubkey, address1);
                CFrogCoinAddress address2(address1);

                if(strFilter !="" && address2.ToString().find(strFilter) == string::npos &&
                    strVin.find(strFilter) == string::npos) continue;
                obj.push_back(Pair(strVin,       address2.ToString().c_str()));
            } else if(strMode == "status") {
                std::string strStatus = pas.Status();
                if(strFilter !="" && strVin.find(strFilter) == string::npos && strStatus.find(strFilter) == string::npos) continue;
                obj.push_back(Pair(strVin,       strStatus.c_str()));
            }
        }
//
    return obj;

}
