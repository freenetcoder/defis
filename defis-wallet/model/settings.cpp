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

#include "settings.h"

#include <algorithm>
#include <map>

#include <QFileDialog>
#include <QtQuick>

#include "defis-wallet/model/app_model.h"
#include "version.h"

#include "quazip/quazip.h"
#include "quazip/quazipfile.h"

using namespace std;

namespace
{
    const char* kNodeAddressName = "node/address";
    const char* kLocaleName = "locale";
    const char* kLockTimeoutName = "lock_timeout";
    const char* kRequirePasswordToSpendMoney = "require_password_to_spend_money";
    const char* kIsAlowedgrimmLink = "grimm_mw_links_allowed";
    const char* LocalNodeMiningThreads = "localnode/mining_threads";

    const char* LocalNodecac_symbol = "localnode/cac_symbol";
    const char* LocalNodecac_id1 = "localnode/cac_id1";
    const char* LocalNodecac_id2 = "localnode/cac_id2";
    const char* cacpremine = "lacalnode/cac_premine";

    const char* cacblockreward = "lacalnode/cac_blockreward";
    const char* cacdrop0 = "lacalnode/cac_drop0";
    const char* cacdrop1 = "lacalnode/cac_drop1";
    const char* caccoinbasematurity = "lacalnode/cac_coinbasematurity";
    const char* cacstandartmaturity = "lacalnode/cac_standartmaturity";
    const char* cacblocktime = "lacalnode/cac_blocktime";
    const char* cacdiff = "lacalnode/cac_diff";

    const char* kLocalNodeRun = "localnode/run";
    const char* kLocalNodePort = "localnode/port";
    const char* kLocalNodePeers = "localnode/peers";



    #ifdef XGM_USE_GPU
   const char* LocalNodeUseGpu = "localnode/use_gpu";
   const char* LocalNodeMiningDevices = "localnode/mining_devices";
   #endif


    const char* kDefaultLocale = "en_US";

    const std::map<QString, QString> kSupportedLangs {
        { "en_US", "English" },
        { "ru_RU", "Русский" },
        { "zh_CN", "简体中文"}
    };
}

const char* WalletSettings::WalletCfg = "defis-wallet.cfg";
const char* WalletSettings::LogsFolder = "logs";
const char* WalletSettings::SettingsFile = "settings.ini";
const char* WalletSettings::WalletDBFile = "wallet.db";
const char* WalletSettings::NodeDBFile = "node.db";

WalletSettings::WalletSettings(const QDir& appDataDir)
    : m_data{ appDataDir.filePath(SettingsFile), QSettings::IniFormat }
    , m_appDataDir{appDataDir}
{

}

string WalletSettings::getWalletStorage() const
{
    Lock lock(m_mutex);

    auto version = QString::fromStdString(PROJECT_VERSION);
    if (!m_appDataDir.exists(version))
    {
        m_appDataDir.mkdir(version);
    }

    return m_appDataDir.filePath(version + "/" + WalletDBFile).toStdString();
}

string WalletSettings::getAppDataPath() const
{
    Lock lock(m_mutex);
    return m_appDataDir.path().toStdString();
}

QString WalletSettings::getNodeAddress() const
{
    Lock lock(m_mutex);
    return m_data.value(kNodeAddressName).toString();
}

void WalletSettings::setNodeAddress(const QString& addr)
{
    if (addr != getNodeAddress())
    {
        auto walletModel = AppModel::getInstance()->getWallet();
        if (walletModel)
        {
            walletModel->getAsync()->setNodeAddress(addr.toStdString());
        }
        {
            Lock lock(m_mutex);
            m_data.setValue(kNodeAddressName, addr);
        }

        emit nodeAddressChanged();
    }

}

int WalletSettings::getLockTimeout() const
{
    Lock lock(m_mutex);
    return m_data.value(kLockTimeoutName, 0).toInt();
}

void WalletSettings::setLockTimeout(int value)
{
    if (value != getLockTimeout())
    {
        {
            Lock lock(m_mutex);
            m_data.setValue(kLockTimeoutName, value);
        }
        emit lockTimeoutChanged();
    }
}

bool WalletSettings::isPasswordReqiredToSpendMoney() const
{
    Lock lock(m_mutex);
    return m_data.value(kRequirePasswordToSpendMoney, false).toBool();
}

void WalletSettings::setPasswordReqiredToSpendMoney(bool value)
{
    Lock lock(m_mutex);
    m_data.setValue(kRequirePasswordToSpendMoney, value);
}

bool WalletSettings::isAllowedgrimmLinks() const
{
    Lock lock(m_mutex);
    return m_data.value(kIsAlowedgrimmLink, false).toBool();
}

void WalletSettings::setAllowedgrimmLinks(bool value)
{
    Lock lock(m_mutex);
    m_data.setValue(kIsAlowedgrimmLink, value);
}

bool WalletSettings::getRunLocalNode() const
{
    Lock lock(m_mutex);
    return m_data.value(kLocalNodeRun, false).toBool();
}

void WalletSettings::setRunLocalNode(bool value)
{
    {
        Lock lock(m_mutex);
        m_data.setValue(kLocalNodeRun, value);
    }
    emit localNodeRunChanged();
}

uint WalletSettings::getLocalNodePort() const
{
    Lock lock(m_mutex);
#ifdef XGM_TESTNET
    return m_data.value(kLocalNodePort, 11005).toUInt();
#else
    return m_data.value(kLocalNodePort, 10005).toUInt();
#endif // XGM_TESTNET
}

void WalletSettings::setLocalNodePort(uint port)
{
    {
        Lock lock(m_mutex);
        m_data.setValue(kLocalNodePort, port);
    }
    emit localNodePortChanged();
}

uint WalletSettings::getLocalNodeMiningThreads() const
{
    Lock lock(m_mutex);
    return m_data.value(LocalNodeMiningThreads, 1).toUInt();
}

void WalletSettings::setLocalNodeMiningThreads(uint n)
{
    {
        Lock lock(m_mutex);
        m_data.setValue(LocalNodeMiningThreads, n);
    }
    emit localNodeMiningThreadsChanged();
}

unsigned long long WalletSettings::getcac_premine() const
{
    Lock lock(m_mutex);
    auto ch = m_data.value(cacpremine).toLongLong();
    return ch / 100000000; //to show in coins
}

void WalletSettings::setcac_premine(long long pre)
{
    {
        Lock lock(m_mutex);
        m_data.setValue(cacpremine, (pre * 100000000)); // sets in sats (coins * 10^8)
    }
    emit cac_premineChanged();
}

//
unsigned long long WalletSettings::getcac_blockreward() const
{
    Lock lock(m_mutex);
    auto ch = m_data.value(cacblockreward).toLongLong();
    return ch / 100000000; //to show in coins
}

void WalletSettings::setcac_blockreward(long long blo)
{
    {
        Lock lock(m_mutex);
        m_data.setValue(cacblockreward, (blo * 100000000)); // sets in sats (coins * 10^8)
    }
    emit cac_blockrewardChanged();
}
//
unsigned long long WalletSettings::getcac_drop0() const
{
    Lock lock(m_mutex);
    auto ch = m_data.value(cacdrop0).toLongLong();
    return ch;
}

void WalletSettings::setcac_drop0(long long dro)
{
    {
        Lock lock(m_mutex);
        m_data.setValue(cacdrop0, dro);
    }
    emit cac_drop0Changed();
}
//
unsigned long long WalletSettings::getcac_drop1() const
{
    Lock lock(m_mutex);
    auto ch = m_data.value(cacdrop1).toLongLong();
    return ch;
}

void WalletSettings::setcac_drop1(long long dro1)
{
    {
        Lock lock(m_mutex);
        m_data.setValue(cacdrop1, dro1);
    }
    emit cac_drop1Changed();
}
//
unsigned int WalletSettings::getcac_coinbasematurity() const
{
    Lock lock(m_mutex);
    auto ch = m_data.value(caccoinbasematurity).toInt();
    return ch;
}

void WalletSettings::setcac_coinbasematurity(int coi)
{
    {
        Lock lock(m_mutex);
        m_data.setValue(caccoinbasematurity, coi);
    }
    emit cac_coinbasematurityChanged();
}
//
unsigned int WalletSettings::getcac_standartmaturity() const
{
    Lock lock(m_mutex);
    auto ch = m_data.value(cacstandartmaturity).toInt();
    return ch;
}

void WalletSettings::setcac_standartmaturity(int sta)
{
    {
        Lock lock(m_mutex);
        m_data.setValue(cacstandartmaturity, sta);
    }
    emit cac_standartmaturityChanged();
}
//
unsigned int WalletSettings::getcac_blocktime() const
{
    Lock lock(m_mutex);
    auto ch = m_data.value(cacblocktime).toInt();
    return ch;
}

void WalletSettings::setcac_blocktime(int blo1)
{
    {
        Lock lock(m_mutex);
        m_data.setValue(cacblocktime, blo1);
    }
    emit cac_blocktimeChanged();
}
//
unsigned int WalletSettings::getcac_diff() const
{
    Lock lock(m_mutex);
    auto ch = m_data.value(cacdiff).toInt();
    return ch;
}

void WalletSettings::setcac_diff(int dif)
{
    {
        Lock lock(m_mutex);
        m_data.setValue(cacdiff, dif);
    }
    emit cac_diffChanged();
}
//


QString WalletSettings::getLocalNodecac_symbol() const
{
    Lock lock(m_mutex);
    return m_data.value(LocalNodecac_symbol).toString();
}

void WalletSettings::setLocalNodecac_symbol(const QString& cacsymbol)
{
    {
        Lock lock(m_mutex);
        m_data.setValue(LocalNodecac_symbol, cacsymbol);
    }
    emit localNodecac_symbolChanged();
}

QString WalletSettings::getLocalNodecac_id1() const
{
    Lock lock(m_mutex);
    return m_data.value(LocalNodecac_id1).toString();
}

void WalletSettings::setLocalNodecac_id1(const QString& cacid1)
{
    {
        Lock lock(m_mutex);
        m_data.setValue(LocalNodecac_id1, cacid1);
    }
    emit localNodecac_id1Changed();
}

QString WalletSettings::getLocalNodecac_id2() const
{
    Lock lock(m_mutex);
    return m_data.value(LocalNodecac_id2).toString();
}

void WalletSettings::setLocalNodecac_id2(const QString& cacsid2)
{
    {
        Lock lock(m_mutex);
        m_data.setValue(LocalNodecac_id2, cacsid2);
    }
    emit localNodecac_id2Changed();
}

string WalletSettings::getLocalNodeStorage() const
{
    Lock lock(m_mutex);
    return m_appDataDir.filePath(NodeDBFile).toStdString();
}

string WalletSettings::getTempDir() const
{
    Lock lock(m_mutex);
    return m_appDataDir.filePath("./temp").toStdString();
}

string WalletSettings::getCACNodeStorage() const
{
    Lock lock(m_mutex);
    return m_appDataDir.filePath("CACnode.db").toStdString();
}

#ifdef XGM_USE_GPU
bool WalletSettings::getUseGpu() const
{
    Lock lock(m_mutex);
    return m_data.value(LocalNodeUseGpu, false).toBool();
}

void WalletSettings::setUseGpu(bool value)
{
    if (getUseGpu() != value)
    {
        {
            Lock lock(m_mutex);
            m_data.setValue(LocalNodeUseGpu, value);
        }
        emit localNodeUseGpuChanged();
    }
}

vector<int32_t> WalletSettings::getMiningDevices() const
{
    Lock lock(m_mutex);
    auto t = m_data.value(LocalNodeMiningDevices).value<QStringList>();
    vector<int32_t> v;
    for (const auto& i : t)
    {
        v.push_back(i.toInt());
    }

    return v;
}

void WalletSettings::setMiningDevices(const vector<int32_t>& value)
{
    if (getMiningDevices() != value)
    {
        {
            Lock lock(m_mutex);
            QStringList t;
            for (auto i : value)
            {
                t.push_back(QString::asprintf("%d", i));
            }
            if (t.empty())
            {
                m_data.remove(LocalNodeMiningDevices);
            }
            else
            {
                m_data.setValue(LocalNodeMiningDevices, QVariant::fromValue(t));
            }
        }
        emit localNodeMiningDevicesChanged();
    }
}

#endif



static void zipLocalFile(QuaZip& zip, const QString& path, const QString& folder = QString())
{
    QFile file(path);
    if (file.open(QIODevice::ReadOnly))
    {
        QuaZipFile zipFile(&zip);

        zipFile.open(QIODevice::WriteOnly, QuaZipNewInfo((folder.isEmpty() ? "" : folder) + QFileInfo(file).fileName(), file.fileName()));
        zipFile.write(file.readAll());
        file.close();
        zipFile.close();
    }
}

QStringList WalletSettings::getLocalNodePeers() const
{
    Lock lock(m_mutex);
    return m_data.value(kLocalNodePeers).value<QStringList>();
}

void WalletSettings::setLocalNodePeers(const QStringList& qPeers)
{
    {
        Lock lock(m_mutex);
        m_data.setValue(kLocalNodePeers, QVariant::fromValue(qPeers));
    }
    emit localNodePeersChanged();
}

QString WalletSettings::getLocale() const
{
    QString savedLocale;
    {
        Lock lock(m_mutex);
        savedLocale = m_data.value(kLocaleName).toString();
    }

    if (!savedLocale.isEmpty()) {
        const auto& it = kSupportedLangs.find(savedLocale);
        if (it != kSupportedLangs.end())
        {
            return savedLocale;
        }
    }

    return QString::fromUtf8(kDefaultLocale);
}

QString WalletSettings::getLanguageName() const
{
    return kSupportedLangs.at(getLocale());
}

void WalletSettings::setLocaleByLanguageName(const QString& language)
{
    const auto& it = std::find_if(
            kSupportedLangs.begin(),
            kSupportedLangs.end(),
            [language] (const auto& mapedObject) -> bool
            {
                return mapedObject.second == language;
            });
    auto locale =
            it != kSupportedLangs.end()
                ? it->first
                : QString::fromUtf8(kDefaultLocale);
    {
        Lock lock(m_mutex);
        m_data.setValue(kLocaleName, locale);
    }
    emit localeChanged();
}

// static
QStringList WalletSettings::getSupportedLanguages()
{
    QStringList languagesNames;
    std::transform(kSupportedLangs.begin(),
                   kSupportedLangs.end(),
                   std::back_inserter(languagesNames),
                   [] (const auto& lang) -> QString {
                       return lang.second;
                   });
    return languagesNames;
}

// static
void WalletSettings::openFolder(const QString& path)
{
    QFileInfo fileInfo(path);
    QDesktopServices::openUrl(
        QUrl::fromLocalFile(
            fileInfo.isFile() ? fileInfo.absolutePath() : path));
}

void WalletSettings::reportProblem()
{
    auto logsFolder = QString::fromStdString(LogsFolder) + "/";

    QFile zipFile = m_appDataDir.filePath("grimm v" + QString::fromStdString(PROJECT_VERSION)
        + " " + QSysInfo::productType().toLower() + " report.zip");

    QuaZip zip(zipFile.fileName());
    zip.open(QuaZip::mdCreate);

    // save settings.ini
    zipLocalFile(zip, m_appDataDir.filePath(SettingsFile));

    // save .cfg
    zipLocalFile(zip, QDir(QDir::currentPath()).filePath(WalletCfg));

    // create 'logs' folder
    {
        QuaZipFile zipLogsFile(&zip);
        zipLogsFile.open(QIODevice::WriteOnly, QuaZipNewInfo(logsFolder, logsFolder));
        zipLogsFile.close();
    }

    {
        QDirIterator it(m_appDataDir.filePath(LogsFolder));

        while (it.hasNext())
        {
            zipLocalFile(zip, it.next(), logsFolder);
        }
    }

    {
        QDirIterator it(m_appDataDir);

        while (it.hasNext())
        {
            const auto& name = it.next();
            if (QFileInfo(name).completeSuffix() == "dmp")
            {
                zipLocalFile(zip, m_appDataDir.filePath(name));
            }
        }
    }

    zip.close();

    QString path = QFileDialog::getSaveFileName(nullptr, "Save problem report",
        QDir(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)).filePath(QFileInfo(zipFile).fileName()),
        "Archives (*.zip)");

    if (path.isEmpty())
    {
        zipFile.remove();
    }
    else
    {
        {
            QFile file(path);
            if(file.exists())
                file.remove();
        }

        zipFile.rename(path);
    }
}

void WalletSettings::applyChanges()
{
    AppModel::getInstance()->applySettingsChanges();
}
