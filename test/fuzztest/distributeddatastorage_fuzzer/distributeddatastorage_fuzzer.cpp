/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "distributeddatastorage_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "dbms_fuzztest_util.h"
#define private public
#include "distributed_data_storage.h"
#include "securec.h"

using namespace OHOS::AppExecFwk;
using namespace OHOS::AppExecFwk::DMSFuzzTestUtil;
namespace OHOS {
bool DoSomethingInterestingWithMyAPI(FuzzedDataProvider& fdp)
{
    auto distributedDataStorage = DistributedDataStorage::GetInstance();
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    int32_t userId = GenerateRandomUser(fdp);
    distributedDataStorage->SaveStorageDistributeInfo(bundleName, userId);
    BundleInfo bundleInfo;
    GenerateBundleInfo(fdp, bundleInfo);
    DistributedBundleInfo distributedBundleInfo = distributedDataStorage->ConvertToDistributedBundleInfo(bundleInfo);
    distributedDataStorage->InnerSaveStorageDistributeInfo(distributedBundleInfo);
    distributedDataStorage->DeleteStorageDistributeInfo(bundleName, userId);
    std::string networkId = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    DistributedBundleInfo info = distributedBundleInfo;
    distributedDataStorage->GetStorageDistributeInfo(networkId, bundleName, info);
    std::string stringResult = bundleName;
    uint32_t accessTokenId = fdp.ConsumeIntegral<uint32_t>();
    distributedDataStorage->GetDistributedBundleName(networkId, accessTokenId, stringResult);
    std::string udid = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    distributedDataStorage->AnonymizeUdid(udid);
    distributedDataStorage->DeviceAndNameToKey(udid, bundleName);
    distributedDataStorage->SyncAndCompleted(udid, networkId);
    stringResult = udid;
    distributedDataStorage->GetLocalUdid(stringResult);
    distributedDataStorage->GetAllOldDistributionBundleInfo(GenerateStringArray(fdp));
    return true;
}
}

// Fuzzer entry point.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::DoSomethingInterestingWithMyAPI(fdp);
    return 0;
}