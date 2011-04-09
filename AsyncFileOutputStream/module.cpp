/************************************************************************
 * To the extent possible under law, Nils Maier has waived all copyright
 * and related or neighboring rights to this work.
 ************************************************************************/

#include "AsyncFileOutputStream.h"
#include "dtaIAsyncFileOutputStream.h"

#include "mozilla/ModuleUtils.h"
#include "nsIClassInfoImpl.h"


#define CONTRACTID "@downthemall.net/async-file-output-stream;1"

// Generic factory
NS_GENERIC_FACTORY_CONSTRUCTOR(AsyncFileOutputStream)
NS_DEFINE_NAMED_CID(DTAIASYNCFILEOUTPUTSTREAM_IID);

static const mozilla::Module::CIDEntry kIIDs[] = {
    { &kDTAIASYNCFILEOUTPUTSTREAM_IID, true, 0, AsyncFileOutputStreamConstructor },
    { 0 }
};
static const mozilla::Module::ContractIDEntry kContracts[] = {
    { CONTRACTID, &kDTAIASYNCFILEOUTPUTSTREAM_IID },
    { 0 }
};
static const mozilla::Module::CategoryEntry kCategories[] = {
    { 0 }
};
static const mozilla::Module kModule = {
    mozilla::Module::kVersion,
    kIIDs,
    kContracts,
    kCategories
};
NSMODULE_DEFN(module) = &kModule;
