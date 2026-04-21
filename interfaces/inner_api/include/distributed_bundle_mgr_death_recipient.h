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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_DBMS_INCLUDE_DISTRIBUTED_BUNDLE_MGR_DEATH_RECIPIENT_H
#define FOUNDATION_APPEXECFWK_SERVICES_DBMS_INCLUDE_DISTRIBUTED_BUNDLE_MGR_DEATH_RECIPIENT_H

#include <string>

#include "distributed_bundle_mgr_client.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
class DistributedBundleMgrDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    explicit DistributedBundleMgrDeathRecipient(DistributedBundleMgrClient* client) : client_(client) {}
    /**
     * @brief Called when distributed bundle manager process died.
     * @return
     */
    virtual void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
private:
    DistributedBundleMgrClient* client_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_SERVICES_DBMS_INCLUDE_DISTRIBUTED_BUNDLE_MGR_DEATH_RECIPIENT_H