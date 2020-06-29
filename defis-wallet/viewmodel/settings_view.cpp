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

#include "settings_view.h"
#include "version.h"
#include <QtQuick>
#include <QApplication>
#include <QClipboard>
#include "model/app_model.h"
#include <thread>
#include "wallet/secstring.h"

#include <algorithm>
#ifdef XGM_USE_GPU
#include "utility/gpu/gpu_tools.h"
#endif



using namespace grimm;
using namespace ECC;
using namespace std;

DeviceItem::DeviceItem(const QString& name, int32_t index, bool enabled)
    : m_name(name)
    , m_index(index)
    , m_enabled(enabled)
{

}

DeviceItem::~DeviceItem()
{

}

QString DeviceItem::getName() const
{
    return m_name;
}

bool DeviceItem::getEnabled() const
{
    return m_enabled;
}

void DeviceItem::setEnabled(bool value)
{
    if (m_enabled != value)
    {
        m_enabled = value;
        emit enableChanged();
    }
}

int32_t DeviceItem::getIndex() const
{
    return m_index;
}



SettingsViewModel::SettingsViewModel()
    : m_settings{AppModel::getInstance()->getSettings()}
    , m_isValidNodeAddress{true}
    , m_isNeedToCheckAddress(false)
    , m_isNeedToApplyChanges(false)
    , m_supportedLanguages(WalletSettings::getSupportedLanguages())
{
    undoChanges();
    connect(&AppModel::getInstance()->getNode(), SIGNAL(startedNode()), SLOT(onNodeStarted()));
    connect(&AppModel::getInstance()->getNode(), SIGNAL(stoppedNode()), SLOT(onNodeStopped()));

    connect(AppModel::getInstance()->getWallet().get(), SIGNAL(addressChecked(const QString&, bool)),
        SLOT(onAddressChecked(const QString&, bool)));

    m_timerId = startTimer(CHECK_INTERVAL);
}

void SettingsViewModel::onNodeStarted()
{
    emit localNodeRunningChanged();
}

void SettingsViewModel::onNodeStopped()
{
    emit localNodeRunningChanged();
}

void SettingsViewModel::onAddressChecked(const QString& addr, bool isValid)
{
    if (m_nodeAddress == addr && m_isValidNodeAddress != isValid)
    {
        m_isValidNodeAddress = isValid;
        emit validNodeAddressChanged();

        if (m_isNeedToApplyChanges)
        {
            if (m_isValidNodeAddress)
                applyChanges();

            m_isNeedToApplyChanges = false;
        }
    }
}

bool SettingsViewModel::isLocalNodeRunning() const
{
    return AppModel::getInstance()->getNode().isNodeRunning();
}

bool SettingsViewModel::isValidNodeAddress() const
{
    return m_isValidNodeAddress;
}

QString SettingsViewModel::getNodeAddress() const
{
    return m_nodeAddress;
}

void SettingsViewModel::setNodeAddress(const QString& value)
{
    if (value != m_nodeAddress)
    {
        m_nodeAddress = value;

        if (!m_isNeedToCheckAddress)
        {
            m_isNeedToCheckAddress = true;
            m_timerId = startTimer(CHECK_INTERVAL);
        }

        emit nodeAddressChanged();
        emit propertiesChanged();
    }
}

QString SettingsViewModel::getVersion() const
{
    return QString::fromStdString(PROJECT_VERSION);
}

bool SettingsViewModel::getLocalNodeRun() const
{
    return m_localNodeRun;
}

void SettingsViewModel::setLocalNodeRun(bool value)
{
    if (value != m_localNodeRun)
    {
        m_localNodeRun = value;

        if (!m_localNodeRun && !m_isNeedToCheckAddress)
        {
            m_isNeedToCheckAddress = true;
            m_timerId = startTimer(CHECK_INTERVAL);
        }

        emit localNodeRunChanged();
        emit propertiesChanged();
    }
}

uint SettingsViewModel::getLocalNodePort() const
{
    return m_localNodePort;
}

void SettingsViewModel::setLocalNodePort(uint value)
{
    if (value != m_localNodePort)
    {
        m_localNodePort = value;
        emit localNodePortChanged();
        emit propertiesChanged();
    }
}

uint SettingsViewModel::getLocalNodeMiningThreads() const
{
    return m_localNodeMiningThreads;
}

void SettingsViewModel::setLocalNodeMiningThreads(uint value)
{
    if (value != m_localNodeMiningThreads)
    {
        m_localNodeMiningThreads = value;
        emit localNodeMiningThreadsChanged();
        emit propertiesChanged();
    }
}

unsigned long long SettingsViewModel::getcac_premine() const
{
    return m_cac_premine;
}

void SettingsViewModel::setcac_premine(unsigned long long value)
{
    if (value != m_cac_premine)
    {
        m_cac_premine = value;
        emit cac_premineChanged();
        emit propertiesChanged();
    }
}


unsigned long long SettingsViewModel::getcac_blockreward() const
{
    return m_cac_blockreward;
}

void SettingsViewModel::setcac_blockreward(unsigned long long value)
{
    if (value != m_cac_blockreward)
    {
        m_cac_blockreward = value;
        emit cac_blockrewardChanged();
        emit propertiesChanged();
    }
}


unsigned long long SettingsViewModel::getcac_drop0() const
{
    return m_cac_drop0;
}

void SettingsViewModel::setcac_drop0(unsigned long long value)
{
    if (value != m_cac_drop0)
    {
        m_cac_drop0 = value;
        emit cac_drop0Changed();
        emit propertiesChanged();
    }
}


unsigned long long SettingsViewModel::getcac_drop1() const
{
    return m_cac_drop1;
}

void SettingsViewModel::setcac_drop1(unsigned long long value)
{
    if (value != m_cac_drop1)
    {
        m_cac_drop1 = value;
        emit cac_drop1Changed();
        emit propertiesChanged();
    }
}

unsigned int SettingsViewModel::getcac_coinbasematurity() const
{
    return m_cac_coinbasematurity;
}

void SettingsViewModel::setcac_coinbasematurity(unsigned int value)
{
    if (value != m_cac_coinbasematurity)
    {
        m_cac_coinbasematurity = value;
        emit cac_coinbasematurityChanged();
        emit propertiesChanged();
    }
}


unsigned int SettingsViewModel::getcac_standartmaturity() const
{
    return m_cac_standartmaturity;
}

void SettingsViewModel::setcac_standartmaturity(unsigned int value)
{
    if (value != m_cac_standartmaturity)
    {
        m_cac_standartmaturity = value;
        emit cac_standartmaturityChanged();
        emit propertiesChanged();
    }
}



unsigned int SettingsViewModel::getcac_blocktime() const
{
    return m_cac_blocktime;
}

void SettingsViewModel::setcac_blocktime(unsigned int value)
{
    if (value != m_cac_blocktime)
    {
        m_cac_blocktime = value;
        emit cac_blocktimeChanged();
        emit propertiesChanged();
    }
}


unsigned int SettingsViewModel::getcac_diff() const
{
    return m_cac_diff;
}

void SettingsViewModel::setcac_diff(unsigned int value)
{
    if (value != m_cac_diff)
    {
        m_cac_diff = value;
        emit cac_diffChanged();
        emit propertiesChanged();
    }
}


QString SettingsViewModel::getLocalNodecac_symbol() const
{
    return m_localNodecac_symbol;
}

void SettingsViewModel::setLocalNodecac_symbol(const QString& value)
{
    if (value != m_localNodecac_symbol)
    {
        m_localNodecac_symbol = value;
        emit localNodecac_symbolChanged();
        emit propertiesChanged();
    }
}

QString SettingsViewModel::getLocalNodecac_id1() const
{
    return m_localNodecac_id1;
}

void SettingsViewModel::setLocalNodecac_id1(const QString& value)
{
    if (value != m_localNodecac_id1)
    {
        m_localNodecac_id1 = value;
        emit localNodecac_id1Changed();
        emit propertiesChanged();
    }
}


QString SettingsViewModel::getLocalNodecac_id2() const
{
    return m_localNodecac_id2;
}

void SettingsViewModel::setLocalNodecac_id2(const QString& value)
{
    if (value != m_localNodecac_id2)
    {
        m_localNodecac_id2 = value;
        emit localNodecac_id2Changed();
        emit propertiesChanged();
    }
}

int SettingsViewModel::getLockTimeout() const
{
    return m_lockTimeout;
}

void SettingsViewModel::setLockTimeout(int value)
{
    if (value != m_lockTimeout)
    {
        m_lockTimeout = value;
        m_settings.setLockTimeout(m_lockTimeout);
        emit lockTimeoutChanged();
    }
}

bool SettingsViewModel::isPasswordReqiredToSpendMoney() const
{
    return m_isPasswordReqiredToSpendMoney;
}

void SettingsViewModel::setPasswordReqiredToSpendMoney(bool value)
{
    if (value != m_isPasswordReqiredToSpendMoney)
    {
        m_isPasswordReqiredToSpendMoney = value;
        m_settings.setPasswordReqiredToSpendMoney(m_isPasswordReqiredToSpendMoney);
        emit passwordReqiredToSpendMoneyChanged();
    }
}

bool SettingsViewModel::isAllowedgrimmLinks() const
{
    return m_isAllowedgrimmLinks;
}

void SettingsViewModel::allowgrimmLinks(bool value)
{
    if (value != m_isAllowedgrimmLinks)
    {
        m_isAllowedgrimmLinks = value;
        m_settings.setAllowedgrimmLinks(m_isAllowedgrimmLinks);
        emit grimmLinksAllowed();
    }
}

QStringList SettingsViewModel::getSupportedLanguages() const
{
    return m_supportedLanguages;
}

int SettingsViewModel::getCurrentLanguageIndex() const
{
    return m_currentLanguageIndex;
}

void SettingsViewModel::setCurrentLanguageIndex(int value)
{
    m_currentLanguageIndex = value;
    m_settings.setLocaleByLanguageName(
            m_supportedLanguages[m_currentLanguageIndex]);
    emit currentLanguageIndexChanged();
}

QString SettingsViewModel::getCurrentLanguage() const
{
    return m_supportedLanguages[m_currentLanguageIndex];
}

void SettingsViewModel::setCurrentLanguage(QString value)
{
    auto index = m_supportedLanguages.indexOf(value);
    if (index != -1 )
    {
        setCurrentLanguageIndex(index);
    }
}

uint SettingsViewModel::coreAmount() const
{
    return std::thread::hardware_concurrency();
}

void SettingsViewModel::addLocalNodePeer(const QString& localNodePeer)
{
    m_localNodePeers.push_back(localNodePeer);
    emit localNodePeersChanged();
    emit propertiesChanged();
}

void SettingsViewModel::deleteLocalNodePeer(int index)
{
    m_localNodePeers.removeAt(index);
    emit localNodePeersChanged();
    emit propertiesChanged();
}

void SettingsViewModel::openUrl(const QString& url)
{
    QDesktopServices::openUrl(QUrl(url));
}

void SettingsViewModel::copyToClipboard(const QString& text)
{
    QApplication::clipboard()->setText(text);
}

QQmlListProperty<DeviceItem> SettingsViewModel::getSupportedDevices()
{
    return QQmlListProperty<DeviceItem>(this, m_supportedDevices);
}

bool SettingsViewModel::hasSupportedGpu()
{
#ifdef XGM_USE_GPU
    if (!m_hasSupportedGpu.is_initialized())
    {
        m_hasSupportedGpu = HasSupportedCard();
    }
    if (*m_hasSupportedGpu == false)
    {
        setUseGpu(false);
        return false;
    }

    if (m_supportedDevices.empty())
    {
        auto selectedDevices = m_settings.getMiningDevices();
        auto cards = GetSupportedCards();
        for (const auto& card : cards)
        {
            bool enabled = find(selectedDevices.begin(), selectedDevices.end(), card.index) != selectedDevices.end();
            m_supportedDevices.push_back(new DeviceItem(QString::fromStdString(card.name), (int32_t)card.index, enabled));
        }
    }

    return true;
#else
    return false;
#endif
}

void SettingsViewModel::refreshWallet()
{
    AppModel::getInstance()->getWallet()->getAsync()->refresh();
}

#ifdef XGM_USE_GPU

vector<int32_t> SettingsViewModel::getSelectedDevice() const
{
    vector<int32_t> v;
    for (const auto& d : m_supportedDevices)
    {
        DeviceItem* device = (DeviceItem*)d;
        if (device->getEnabled())
        {
            v.push_back(device->getIndex());
        }
    }
    return v;
}

#endif



void SettingsViewModel::openFolder(const QString& path)
{
    WalletSettings::openFolder(path);
}

bool SettingsViewModel::isChanged() const
{
    return m_nodeAddress != m_settings.getNodeAddress()
        || m_localNodeRun != m_settings.getRunLocalNode()
        || m_localNodePort != m_settings.getLocalNodePort()
        || m_localNodeMiningThreads != m_settings.getLocalNodeMiningThreads()
        || m_cac_premine != m_settings.getcac_premine()
        || m_cac_blockreward != m_settings.getcac_blockreward()
        || m_cac_drop0 != m_settings.getcac_drop0()
        || m_cac_drop1 != m_settings.getcac_drop1()
        || m_cac_coinbasematurity != m_settings.getcac_coinbasematurity()
        || m_cac_standartmaturity != m_settings.getcac_standartmaturity()
        || m_cac_blocktime != m_settings.getcac_blocktime()
        || m_cac_diff != m_settings.getcac_diff()
        || m_localNodecac_symbol != m_settings.getLocalNodecac_symbol()
        || m_localNodecac_id1 != m_settings.getLocalNodecac_id1()
        || m_localNodecac_id2 != m_settings.getLocalNodecac_id2()
        #ifdef XGM_USE_GPU
                || m_useGpu != m_settings.getUseGpu()
                || (!m_supportedDevices.empty() && m_settings.getMiningDevices() != getSelectedDevice())
                #endif
        || m_localNodePeers != m_settings.getLocalNodePeers();

}

void SettingsViewModel::applyChanges()
{
    if (!m_localNodeRun && m_isNeedToCheckAddress)
    {
        m_isNeedToApplyChanges = true;
        return;
    }

    m_settings.setNodeAddress(m_nodeAddress);
    m_settings.setRunLocalNode(m_localNodeRun);
    m_settings.setLocalNodePort(m_localNodePort);
    m_settings.setLocalNodeMiningThreads(m_localNodeMiningThreads);
    m_settings.setcac_premine(m_cac_premine);
    m_settings.setcac_blockreward(m_cac_blockreward);
    m_settings.setcac_drop0(m_cac_drop0);
    m_settings.setcac_drop1(m_cac_drop1);
    m_settings.setcac_coinbasematurity(m_cac_coinbasematurity);
    m_settings.setcac_standartmaturity(m_cac_standartmaturity);
    m_settings.setcac_blocktime(m_cac_blocktime);
    m_settings.setcac_diff(m_cac_diff);



    m_settings.setLocalNodecac_symbol(m_localNodecac_symbol);
    m_settings.setLocalNodecac_id1(m_localNodecac_id1);
    m_settings.setLocalNodecac_id2(m_localNodecac_id2);
    m_settings.setLocalNodePeers(m_localNodePeers);
    #ifdef XGM_USE_GPU
    m_settings.setUseGpu(m_useGpu);
    m_settings.setMiningDevices(getSelectedDevice());
    #endif

    m_settings.applyChanges();
    emit propertiesChanged();
}

QStringList SettingsViewModel::getLocalNodePeers() const
{
    return m_localNodePeers;
}

void SettingsViewModel::setLocalNodePeers(const QStringList& localNodePeers)
{
    m_localNodePeers = localNodePeers;
    emit localNodePeersChanged();
    emit propertiesChanged();
}

QString SettingsViewModel::getWalletLocation() const
{
    return QString::fromStdString(m_settings.getAppDataPath());
}

void SettingsViewModel::setUseGpu(bool value)
{
#ifdef XGM_USE_GPU
    m_useGpu = value;
    emit localNodeUseGpuChanged();
    emit propertiesChanged();
#endif
}

bool SettingsViewModel::getUseGpu() const
{
    return m_useGpu;
}

void SettingsViewModel::undoChanges()
{
    setNodeAddress(m_settings.getNodeAddress());
    setLocalNodeRun(m_settings.getRunLocalNode());
    setLocalNodePort(m_settings.getLocalNodePort());
    setLocalNodeMiningThreads(m_settings.getLocalNodeMiningThreads());
    setcac_blockreward(m_settings.getcac_blockreward());
    setcac_premine(m_settings.getcac_premine());
    setcac_drop0(m_settings.getcac_drop0());
    setcac_drop1(m_settings.getcac_drop1());
    setcac_coinbasematurity(m_settings.getcac_coinbasematurity());
    setcac_blocktime(m_settings.getcac_blocktime());
    setcac_diff(m_settings.getcac_diff());
    setcac_standartmaturity(m_settings.getcac_standartmaturity());



    setLocalNodecac_symbol(m_settings.getLocalNodecac_symbol());
    setLocalNodecac_id1(m_settings.getLocalNodecac_id1());
    setLocalNodecac_id2(m_settings.getLocalNodecac_id2());
    setLockTimeout(m_settings.getLockTimeout());
    setLocalNodePeers(m_settings.getLocalNodePeers());
    #ifdef XGM_USE_GPU
    setUseGpu(m_settings.getUseGpu());
    #endif

    setPasswordReqiredToSpendMoney(m_settings.isPasswordReqiredToSpendMoney());
    allowgrimmLinks(m_settings.isAllowedgrimmLinks());
    setCurrentLanguageIndex(
            m_supportedLanguages.indexOf(m_settings.getLanguageName()));
}

void SettingsViewModel::reportProblem()
{
    m_settings.reportProblem();
}

bool SettingsViewModel::checkWalletPassword(const QString& oldPass) const
{
    SecString secretPass = oldPass.toStdString();
    return AppModel::getInstance()->checkWalletPassword(secretPass);
}

void SettingsViewModel::changeWalletPassword(const QString& pass)
{
    AppModel::getInstance()->changeWalletPassword(pass.toStdString());
}

void SettingsViewModel::timerEvent(QTimerEvent *event)
{
    if (m_isNeedToCheckAddress && !m_localNodeRun)
    {
        m_isNeedToCheckAddress = false;

        AppModel::getInstance()->getWallet()->getAsync()->checkAddress(m_nodeAddress.toStdString());

        killTimer(m_timerId);
    }
}
