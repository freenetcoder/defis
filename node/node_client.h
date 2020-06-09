// Copyright 2018 The Beam Team / Copyright 2019 The Grimm Team
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <thread>
#include <memory>
#include <atomic>
#include <condition_variable>
#include "node/node.h"
#include "core/block_crypt.h"
#include "utility/io/errorhandling.h"
#include "utility/io/reactor.h"

namespace grimm
{
class INodeClientObserver
{
public:
    virtual void onSyncProgressUpdated(int done, int total) = 0;
    virtual void onStartedNode() = 0;
    virtual void onStoppedNode() = 0;
    virtual void onFailedToStartNode(io::ErrorCode errorCode) = 0;
    virtual void onSyncError(Node::IObserver::Error error) = 0;
    virtual unsigned int getLocalNodeMiningThreads() = 0;
    virtual std::string getLocalNodecac_symbol() = 0;

    virtual unsigned long long getcac_premine() = 0;
    virtual unsigned long long getcac_blockreward() = 0;
    virtual unsigned long long getcac_drop0() = 0;
    virtual unsigned long long getcac_drop1() = 0;
    virtual unsigned int getcac_coinbasematurity() = 0;
    virtual unsigned int getcac_standartmaturity() = 0;
    virtual unsigned int getcac_blocktime() = 0;
    virtual unsigned int getcac_diff() = 0;

    virtual uint16_t getLocalNodePort() = 0;
    virtual std::string getLocalNodeStorage() = 0;
    virtual std::string getTempDir() = 0;
    virtual std::string getCACNodeStorage() = 0;
    virtual std::vector<std::string> getLocalNodePeers() = 0;
    #ifdef XGM_USE_GPU
        virtual std::unique_ptr<grimm::IExternalPOW> getStratumServer() = 0;
        #endif //  XGM_USE_GPU

    virtual void onNodeThreadFinished() = 0;
};

class NodeClient
{
public:
    NodeClient(INodeClientObserver* observer);
    ~NodeClient();

    void setKdf(grimm::Key::IKdf::Ptr);
    void setOwnerKey(grimm::Key::IPKdf::Ptr);
    void startNode();
    void stopNode();

    void start();

    bool isNodeRunning() const;

private:
    void runLocalNode();

private:
    INodeClientObserver* m_observer;
    std::shared_ptr<std::thread> m_thread;
    std::weak_ptr<grimm::io::Reactor> m_reactor;
    std::atomic<bool> m_shouldStartNode;
    std::atomic<bool> m_shouldTerminateModel;
    std::atomic<bool> m_isRunning;
    std::condition_variable m_waiting;
    grimm::Key::IKdf::Ptr m_pKdf;
    grimm::Key::IPKdf::Ptr m_ownerKey;
};
}
