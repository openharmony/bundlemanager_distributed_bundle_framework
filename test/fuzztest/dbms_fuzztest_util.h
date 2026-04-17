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

#ifndef DBMS_FUZZTEST_UTIL_H
#define DBMS_FUZZTEST_UTIL_H

#include <fuzzer/FuzzedDataProvider.h>
#include <map>
#include <string>
#include <vector>

#include "bundle_info.h"

namespace OHOS {
namespace AppExecFwk {
namespace DMSFuzzTestUtil {
constexpr size_t STRING_MAX_LENGTH = 128;
constexpr size_t ARRAY_MAX_LENGTH = 128;
constexpr uint32_t CODE_MIN_ONE = 1;
constexpr uint32_t CODE_MAX_THREE = 3;
const std::vector<int32_t> USERS {
    Constants::ANY_USERID,
    Constants::ALL_USERID,
    Constants::UNSPECIFIED_USERID,
    Constants::INVALID_USERID,
    Constants::U1,
    Constants::DEFAULT_USERID,
    Constants::START_USERID
};

std::vector<std::string> GenerateStringArray(FuzzedDataProvider& fdp, size_t arraySizeMax = ARRAY_MAX_LENGTH,
    size_t stringSize = STRING_MAX_LENGTH)
{
    std::vector<std::string> result;
    size_t arraySize = fdp.ConsumeIntegralInRange<size_t>(0, arraySizeMax);
    result.reserve(arraySize);

    for (size_t i = 0; i < arraySize; ++i) {
        std::string str = fdp.ConsumeRandomLengthString(stringSize);
        result.emplace_back(str);
    }

    return result;
}

int32_t GenerateRandomUser(FuzzedDataProvider& fdp)
{
    uint8_t usersSize = USERS.size();
    if (usersSize == 0) {
        return Constants::START_USERID; // No users available, return 100
    }
    uint8_t index = fdp.ConsumeIntegralInRange<uint8_t>(0, usersSize) % usersSize; // 0 to size-1
    return USERS[index];
}

std::string GenerateASCIIString(FuzzedDataProvider& fdp)
{
    std::string result = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    result.erase(std::remove_if(result.begin(), result.end(),
        [](char c) { return static_cast<unsigned char>(c) >= 0x80; }), result.end());
    return result.empty() ? "test" : result;
}

void GenerateSignatureInfo(FuzzedDataProvider& fdp, SignatureInfo& signatureInfo)
{
    signatureInfo.appId = GenerateASCIIString(fdp);
    signatureInfo.fingerprint = GenerateASCIIString(fdp);
    signatureInfo.appIdentifier = GenerateASCIIString(fdp);
    signatureInfo.certificate = GenerateASCIIString(fdp);
}

void GenerateBundleInfo(FuzzedDataProvider& fdp, BundleInfo& bundleInfo)
{
    bundleInfo.isNewVersion = fdp.ConsumeBool();
    bundleInfo.isKeepAlive = fdp.ConsumeBool();
    bundleInfo.singleton = fdp.ConsumeBool();
    bundleInfo.isPreInstallApp = fdp.ConsumeBool();
    bundleInfo.isNativeApp = fdp.ConsumeBool();
    bundleInfo.entryInstallationFree = fdp.ConsumeBool();
    bundleInfo.isDifferentName = fdp.ConsumeBool();
    bundleInfo.versionCode = fdp.ConsumeIntegral<uint32_t>();
    bundleInfo.minCompatibleVersionCode = fdp.ConsumeIntegral<uint32_t>();
    bundleInfo.compatibleVersion = fdp.ConsumeIntegral<uint32_t>();
    bundleInfo.targetVersion = fdp.ConsumeIntegral<uint32_t>();
    bundleInfo.appIndex = fdp.ConsumeIntegral<int32_t>();
    bundleInfo.minSdkVersion = fdp.ConsumeIntegral<int32_t>();
    bundleInfo.maxSdkVersion = fdp.ConsumeIntegral<int32_t>();
    bundleInfo.overlayType = fdp.ConsumeIntegralInRange<int32_t>(CODE_MIN_ONE, CODE_MAX_THREE);
    bundleInfo.uid = fdp.ConsumeIntegral<int>();
    bundleInfo.gid = fdp.ConsumeIntegral<int>();
    bundleInfo.installTime = fdp.ConsumeIntegral<int64_t>();
    bundleInfo.updateTime = fdp.ConsumeIntegral<int64_t>();
    bundleInfo.firstInstallTime = fdp.ConsumeIntegral<int64_t>();
    bundleInfo.name = GenerateASCIIString(fdp);
    bundleInfo.versionName = GenerateASCIIString(fdp);
    bundleInfo.vendor = GenerateASCIIString(fdp);
    bundleInfo.releaseType = GenerateASCIIString(fdp);
    bundleInfo.mainEntry = GenerateASCIIString(fdp);
    bundleInfo.entryModuleName = GenerateASCIIString(fdp);
    bundleInfo.appId = GenerateASCIIString(fdp);
    bundleInfo.cpuAbi = GenerateASCIIString(fdp);
    bundleInfo.seInfo = GenerateASCIIString(fdp);
    bundleInfo.label = GenerateASCIIString(fdp);
    bundleInfo.description = GenerateASCIIString(fdp);
    bundleInfo.jointUserId = GenerateASCIIString(fdp);

    GenerateSignatureInfo(fdp, bundleInfo.signatureInfo);

    bundleInfo.oldAppIds = GenerateStringArray(fdp);
    bundleInfo.hapModuleNames = GenerateStringArray(fdp);
    bundleInfo.moduleNames = GenerateStringArray(fdp);
    bundleInfo.modulePublicDirs = GenerateStringArray(fdp);
    bundleInfo.moduleDirs = GenerateStringArray(fdp);
    bundleInfo.moduleResPaths = GenerateStringArray(fdp);

    bundleInfo.reqPermissions = GenerateStringArray(fdp);
    bundleInfo.defPermissions = GenerateStringArray(fdp);
}
}  // namespace DBMSFuzzTestUtil
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // DBMS_FUZZTEST_UTIL_H