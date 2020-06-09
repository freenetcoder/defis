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

#include <QObject>
#include <QSettings>
#include <QDir>
#include <mutex>

#include "model/wallet_model.h"

class WalletSettings : public QObject
{
    Q_OBJECT
public:
    WalletSettings(const QDir& appDataDir);

    QString getNodeAddress() const;
    void setNodeAddress(const QString& value);

    int getLockTimeout() const;
    void setLockTimeout(int value);

    bool isPasswordReqiredToSpendMoney() const;
    void setPasswordReqiredToSpendMoney(bool value);

    bool isAllowedgrimmLinks() const;
    void setAllowedgrimmLinks(bool value);

    void initModel(WalletModel::Ptr model);
    std::string getWalletStorage() const;
    std::string getAppDataPath() const;
    void reportProblem();
    uint getLocalNodeMiningThreads() const;
    void setLocalNodeMiningThreads(uint n);

    QString getLocalNodecac_symbol() const;
    void setLocalNodecac_symbol(const QString& cacsymbol);

    unsigned long long getcac_blockreward() const;
    void setcac_blockreward(long long blockreward);

    unsigned long long getcac_drop0() const;
    void setcac_drop0(long long drop0);

    unsigned long long getcac_drop1() const;
    void setcac_drop1(long long drop1);

    unsigned int getcac_coinbasematurity() const;
    void setcac_coinbasematurity(int coinbasematurity);

    unsigned int getcac_standartmaturity() const;
    void setcac_standartmaturity(int standartmaturity);

    unsigned int getcac_blocktime() const;
    void setcac_blocktime(int blocktime);

    unsigned int getcac_diff() const;
    void setcac_diff(int diff);

    unsigned long long getcac_premine() const;
    void setcac_premine(long long premine);

    bool getRunLocalNode() const;
    void setRunLocalNode(bool value);

    uint getLocalNodePort() const;
    void setLocalNodePort(uint port);
    std::string getLocalNodeStorage() const;
    std::string getTempDir() const;
    std::string getCACNodeStorage() const;

    QStringList getLocalNodePeers() const;
    void setLocalNodePeers(const QStringList& qPeers);
    #ifdef XGM_USE_GPU
   bool getUseGpu() const;
   void setUseGpu(bool value);
   std::vector<int32_t> getMiningDevices() const;
   void setMiningDevices(const std::vector<int32_t>& value);
   #endif



    QString getLocale() const;
    QString getLanguageName() const;
    void setLocaleByLanguageName(const QString& language);
    static QStringList getSupportedLanguages();
    static void openFolder(const QString& path);

public:
    static const char* WalletCfg;
    static const char* LogsFolder;
    static const char* SettingsFile;
    static const char* WalletDBFile;
    static const char* NodeDBFile;

    void applyChanges();

signals:
    void nodeAddressChanged();
    void lockTimeoutChanged();
    void localNodeRunChanged();
    void localNodePortChanged();
    void localNodePeersChanged();
    void localNodeSynchronizedChanged();
    void localeChanged();
    void localNodeMiningThreadsChanged();

    void cac_premineChanged();

    void cac_blockrewardChanged();
    void cac_drop0Changed();
    void cac_drop1Changed();
    void cac_coinbasematurityChanged();
    void cac_standartmaturityChanged();
    void cac_blocktimeChanged();
    void cac_diffChanged();

    void localNodecac_symbolChanged();

    #ifdef XGM_USE_GPU
   void localNodeUseGpuChanged();
   void localNodeMiningDevicesChanged();
   #endif


private:
    QSettings m_data;
    QDir m_appDataDir;
    mutable std::mutex m_mutex;
    using Lock = std::unique_lock<std::mutex>;
};
