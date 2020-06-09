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
#include <QQmlListProperty>

#include "model/settings.h"

class DeviceItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name       READ getName     CONSTANT )
    Q_PROPERTY(bool enabled       READ getEnabled WRITE setEnabled     NOTIFY enableChanged)

public:

    DeviceItem() = default;
    DeviceItem(const QString& name, int32_t index, bool enabled);
    virtual ~DeviceItem();

    QString getName() const;
    bool getEnabled() const;
    void setEnabled(bool value);
    int32_t getIndex() const;


signals:
    void enableChanged();

private:
    QString m_name;
    int32_t m_index;
    bool m_enabled;
};

class SettingsViewModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString nodeAddress READ getNodeAddress WRITE setNodeAddress NOTIFY nodeAddressChanged)
    Q_PROPERTY(QString version READ getVersion CONSTANT)
    Q_PROPERTY(bool localNodeRun READ getLocalNodeRun WRITE setLocalNodeRun NOTIFY localNodeRunChanged)
    Q_PROPERTY(uint localNodePort READ getLocalNodePort WRITE setLocalNodePort NOTIFY localNodePortChanged)
    Q_PROPERTY(uint localNodeMiningThreads READ getLocalNodeMiningThreads WRITE setLocalNodeMiningThreads NOTIFY localNodeMiningThreadsChanged)

    Q_PROPERTY(qlonglong cac_premineid READ getcac_premine WRITE setcac_premine NOTIFY cac_premineChanged)
    Q_PROPERTY(qlonglong cac_blockrewardid READ getcac_blockreward WRITE setcac_blockreward NOTIFY cac_blockrewardChanged)
    Q_PROPERTY(qlonglong cac_drop0id READ getcac_drop0 WRITE setcac_drop0 NOTIFY cac_drop0Changed)
    Q_PROPERTY(qlonglong cac_drop1id READ getcac_drop1 WRITE setcac_drop1 NOTIFY cac_drop1Changed)
    Q_PROPERTY(uint cac_coinbasematurityid READ getcac_coinbasematurity WRITE setcac_coinbasematurity NOTIFY cac_coinbasematurityChanged)
    Q_PROPERTY(uint cac_standartmaturityid READ getcac_standartmaturity WRITE setcac_standartmaturity NOTIFY cac_standartmaturityChanged)
    Q_PROPERTY(uint cac_blocktimeid READ getcac_blocktime WRITE setcac_blocktime NOTIFY cac_blocktimeChanged)
    Q_PROPERTY(uint cac_diffid READ getcac_diff WRITE setcac_diff NOTIFY cac_diffChanged)

    Q_PROPERTY(QString localNodecac_symbol READ getLocalNodecac_symbol WRITE setLocalNodecac_symbol NOTIFY localNodecac_symbolChanged)
    Q_PROPERTY(bool isChanged READ isChanged NOTIFY propertiesChanged)
    Q_PROPERTY(QStringList localNodePeers READ getLocalNodePeers NOTIFY localNodePeersChanged)
    Q_PROPERTY(int lockTimeout READ getLockTimeout WRITE setLockTimeout NOTIFY lockTimeoutChanged)
    Q_PROPERTY(QString walletLocation READ getWalletLocation CONSTANT)
    Q_PROPERTY(bool isLocalNodeRunning READ isLocalNodeRunning NOTIFY localNodeRunningChanged)
    Q_PROPERTY(bool useGpu READ getUseGpu WRITE setUseGpu NOTIFY localNodeUseGpuChanged)
   Q_PROPERTY(QQmlListProperty<DeviceItem> supportedDevices READ getSupportedDevices NOTIFY localNodeUseGpuChanged)

    Q_PROPERTY(bool isPasswordReqiredToSpendMoney READ isPasswordReqiredToSpendMoney WRITE setPasswordReqiredToSpendMoney NOTIFY passwordReqiredToSpendMoneyChanged)
    Q_PROPERTY(bool isAllowedgrimmLinks READ isAllowedgrimmLinks WRITE allowgrimmLinks NOTIFY grimmLinksAllowed)
    Q_PROPERTY(QStringList supportedLanguages READ getSupportedLanguages NOTIFY currentLanguageIndexChanged)
    Q_PROPERTY(int currentLanguageIndex READ getCurrentLanguageIndex NOTIFY currentLanguageIndexChanged)
    Q_PROPERTY(QString currentLanguage READ getCurrentLanguage WRITE setCurrentLanguage)
    Q_PROPERTY(bool isValidNodeAddress READ isValidNodeAddress NOTIFY validNodeAddressChanged)
public:

    SettingsViewModel();

    QString getNodeAddress() const;
    void setNodeAddress(const QString& value);
    QString getVersion() const;
    bool getLocalNodeRun() const;
    void setLocalNodeRun(bool value);
    uint getLocalNodePort() const;
    void setLocalNodePort(uint value);
    uint getLocalNodeMiningThreads() const;
    void setLocalNodeMiningThreads(uint value);

    unsigned long long getcac_premine() const;
    void setcac_premine(unsigned long long value);

    unsigned long long getcac_blockreward() const;
    void setcac_blockreward(unsigned long long value);

    unsigned long long getcac_drop0() const;
    void setcac_drop0(unsigned long long value);

    unsigned long long getcac_drop1() const;
    void setcac_drop1(unsigned long long value);

    unsigned int getcac_coinbasematurity() const;
    void setcac_coinbasematurity(unsigned int value);

    unsigned int getcac_standartmaturity() const;
    void setcac_standartmaturity(unsigned int value);

    unsigned int getcac_blocktime() const;
    void setcac_blocktime(unsigned int value);

    unsigned int getcac_diff() const;
    void setcac_diff(unsigned int value);




    QString getLocalNodecac_symbol() const;
    void setLocalNodecac_symbol(const QString& value);
    int getLockTimeout() const;
    void setLockTimeout(int value);
    bool isPasswordReqiredToSpendMoney() const;
    void setPasswordReqiredToSpendMoney(bool value);
    bool isAllowedgrimmLinks() const;
    void allowgrimmLinks(bool value);
    QStringList getSupportedLanguages() const;
    int getCurrentLanguageIndex() const;
    void setCurrentLanguageIndex(int value);
    QString getCurrentLanguage() const;
    void setCurrentLanguage(QString value);

    QStringList getLocalNodePeers() const;
    void setLocalNodePeers(const QStringList& localNodePeers);
    QString getWalletLocation() const;
    void setUseGpu(bool value);
   bool getUseGpu() const;

    bool isLocalNodeRunning() const;
    bool isValidNodeAddress() const;
     QQmlListProperty<DeviceItem> getSupportedDevices();



    bool isChanged() const;

    Q_INVOKABLE uint coreAmount() const;
    Q_INVOKABLE void addLocalNodePeer(const QString& localNodePeer);
    Q_INVOKABLE void deleteLocalNodePeer(int index);
    Q_INVOKABLE void openUrl(const QString& url);
    Q_INVOKABLE void copyToClipboard(const QString& text);
    Q_INVOKABLE bool hasSupportedGpu();

    Q_INVOKABLE void refreshWallet();
    Q_INVOKABLE void openFolder(const QString& path);
  private:

    std::vector<int32_t> getSelectedDevice() const;


public slots:
    void applyChanges();
    void undoChanges();
	void reportProblem();
    bool checkWalletPassword(const QString& oldPass) const;
    void changeWalletPassword(const QString& pass);
    void onNodeStarted();
    void onNodeStopped();
    void onAddressChecked(const QString& addr, bool isValid);

signals:
    void nodeAddressChanged();
    void localNodeRunChanged();
    void localNodePortChanged();
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
    void localNodePeersChanged();
    void propertiesChanged();
    void lockTimeoutChanged();
     void localNodeUseGpuChanged();

    void localNodeRunningChanged();
    void passwordReqiredToSpendMoneyChanged();
    void validNodeAddressChanged();
    void currentLanguageIndexChanged();
    void grimmLinksAllowed();

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    WalletSettings& m_settings;

    QString m_nodeAddress;
    bool m_localNodeRun;
    uint m_localNodePort;
    uint m_localNodeMiningThreads;

    unsigned long long m_cac_premine;

    unsigned long long m_cac_blockreward;
    unsigned long long m_cac_drop0;
    unsigned long long m_cac_drop1;
    unsigned int m_cac_coinbasematurity;
    unsigned int m_cac_standartmaturity;
    unsigned int m_cac_blocktime;
    unsigned int m_cac_diff;


    QString m_localNodecac_symbol;

    QStringList m_localNodePeers;
    QList<DeviceItem*> m_supportedDevices;

    int m_lockTimeout;
    bool m_useGpu;
   boost::optional<bool> m_hasSupportedGpu;

    bool m_isPasswordReqiredToSpendMoney;
    bool m_isAllowedgrimmLinks;
    bool m_isValidNodeAddress;
    bool m_isNeedToCheckAddress;
    bool m_isNeedToApplyChanges;
    QStringList m_supportedLanguages;
    int m_currentLanguageIndex;
    int m_timerId;

    const int CHECK_INTERVAL = 1000;
};
