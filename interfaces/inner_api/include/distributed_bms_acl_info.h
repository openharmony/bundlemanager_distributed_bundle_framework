/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_DBMS_INCLUDE_DISTRIBUTED_BMS_ACL_INFO_H
#define FOUNDATION_APPEXECFWK_SERVICES_DBMS_INCLUDE_DISTRIBUTED_BMS_ACL_INFO_H

#include <string>

#include "parcel.h"

namespace OHOS {
namespace AppExecFwk {
struct DistributedBmsAclInfo : public Parcelable {
    std::string networkId;
    int32_t userId = 0;
    std::string accountId;
    uint64_t tokenId = 0;
    std::string pkgName;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static DistributedBmsAclInfo *Unmarshalling(Parcel &parcel);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif // FOUNDATION_APPEXECFWK_SERVICES_DBMS_INCLUDE_DISTRIBUTED_BMS_ACL_INFO_H