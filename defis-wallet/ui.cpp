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

#include <QApplication>
#include <QtQuick>
#include <QQmlApplicationEngine>

#include <QInputDialog>
#include <QMessageBox>

#include <qqmlcontext.h>
#include "viewmodel/start_view.h"
#include "viewmodel/loading_view.h"
#include "viewmodel/main_view.h"
#include "viewmodel/utxo_view.h"
#include "viewmodel/utxo_view_status.h"
#include "viewmodel/utxo_view_type.h"
#include "viewmodel/dashboard_view.h"
#include "viewmodel/address_book_view.h"
#include "viewmodel/wallet_view.h"
#include "viewmodel/notifications_view.h"
#include "viewmodel/help_view.h"
#include "viewmodel/settings_view.h"
#include "viewmodel/messages_view.h"
#include "viewmodel/statusbar_view.h"
#include "viewmodel/theme.h"
#include "model/app_model.h"

#include "wallet/wallet_db.h"
#include "utility/log_rotation.h"
#include "core/ecc_native.h"

#include "utility/cli/options.h"

#include <QtCore/QtPlugin>

#include "version.h"

#include "utility/string_helpers.h"
#include "utility/helpers.h"

#if defined(XGM_USE_STATIC)

#if defined Q_OS_WIN
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
Q_IMPORT_PLUGIN(QWindowsPrinterSupportPlugin)
#elif defined Q_OS_MAC
Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin)
Q_IMPORT_PLUGIN(QCocoaPrinterSupportPlugin)
#elif defined Q_OS_LINUX
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)
Q_IMPORT_PLUGIN(QXcbGlxIntegrationPlugin)
Q_IMPORT_PLUGIN(QCupsPrinterSupportPlugin)
#endif

Q_IMPORT_PLUGIN(QtQuick2Plugin)
Q_IMPORT_PLUGIN(QtQuick2WindowPlugin)
Q_IMPORT_PLUGIN(QtQuickControls1Plugin)
Q_IMPORT_PLUGIN(QtQuickControls2Plugin)
Q_IMPORT_PLUGIN(QtGraphicalEffectsPlugin)
Q_IMPORT_PLUGIN(QtGraphicalEffectsPrivatePlugin)
Q_IMPORT_PLUGIN(QSvgPlugin)
Q_IMPORT_PLUGIN(QtQuickLayoutsPlugin)
Q_IMPORT_PLUGIN(QtQuickTemplates2Plugin)


#endif

using namespace grimm;
using namespace std;
using namespace ECC;

#ifdef APP_NAME
static const char* AppName = APP_NAME;
#else
static const char* AppName = "XGM Wallet";
#endif

int main (int argc, char* argv[])
{
#if defined Q_OS_WIN
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    block_sigpipe();

    QApplication app(argc, argv);

	app.setWindowIcon(QIcon(Theme::iconPath()));

    QApplication::setApplicationName(AppName);

    QDir appDataDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));

    try
    {

        // TODO: ugly temporary fix for unused variable, GCC only
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

        auto [options, visibleOptions] = createOptionsDescription(GENERAL_OPTIONS | UI_OPTIONS | WALLET_OPTIONS);

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

        po::variables_map vm;

        try
        {
            vm = getOptions(argc, argv, WalletSettings::WalletCfg, options, true);
        }
        catch (const po::error& e)
        {
            QMessageBox msgBox;
            msgBox.setText(e.what());
            msgBox.exec();
            return -1;
        }

        if (vm.count(cli::VERSION))
        {
            QMessageBox msgBox;
            msgBox.setText(PROJECT_VERSION.c_str());
            msgBox.exec();
            return 0;
        }

        if (vm.count(cli::GIT_COMMIT_HASH))
        {
            QMessageBox msgBox;
            msgBox.setText(GIT_COMMIT_HASH.c_str());
            msgBox.exec();
            return 0;
        }

        if (vm.count(cli::APPDATA_PATH))
        {
            appDataDir = QString::fromStdString(vm[cli::APPDATA_PATH].as<string>());
        }

        int logLevel = getLogLevel(cli::LOG_LEVEL, vm, LOG_LEVEL_DEBUG);
        int fileLogLevel = getLogLevel(cli::FILE_LOG_LEVEL, vm, LOG_LEVEL_DEBUG);

        grimm::Crash::InstallHandler(appDataDir.filePath(AppName).toStdString().c_str());

#define LOG_FILES_PREFIX "grimm_ui_"

        const auto logFilesPath = appDataDir.filePath(WalletSettings::LogsFolder).toStdString();
        auto logger = grimm::Logger::create(logLevel, logLevel, fileLogLevel, LOG_FILES_PREFIX, logFilesPath);

        unsigned logCleanupPeriod = vm[cli::LOG_CLEANUP_DAYS].as<uint32_t>() * 24 * 3600;

        clean_old_logfiles(logFilesPath, LOG_FILES_PREFIX, logCleanupPeriod);

        try
        {
          Rules::get().UpdateChecksum();


  if (Rules::get().isAssetchain) {
           LOG_INFO() << "Confidential Assetchain Symbol: " << vm[cli::CAC_SYMBOL].as<string>();
           LOG_INFO() << vm[cli::CAC_SYMBOL].as<string>() << " Node " << PROJECT_VERSION << " (" << BRANCH_NAME << ")";
           LOG_INFO() << "Confidential Assetchain node connection id1 " << vm[cli::CAC_ID1].as<string>();
           LOG_INFO() << "Confidential Assetchain node connection id2 " << vm[cli::CAC_ID2].as<string>();




     } else {
        LOG_INFO() << "Grimm Node " << PROJECT_VERSION << " (" << BRANCH_NAME << ")";

     }

  LOG_INFO() << "Rules signature: " << Rules::get().get_SignatureStr();

            WalletSettings settings(appDataDir);
            QQmlApplicationEngine engine;
            AppModel appModel(settings, engine);


              string cac_symbol = vm[cli::CAC_SYMBOL].as<string>();
              settings.setLocalNodecac_symbol(cac_symbol.c_str());

              string cac_id1 = vm[cli::CAC_ID1].as<string>();
              settings.setLocalNodecac_id1(cac_id1.c_str());

              string cac_id2 = vm[cli::CAC_ID2].as<string>();
              settings.setLocalNodecac_id2(cac_id2.c_str());

              grimm::Amount pre_cac = vm[cli::CAC_PREMINE].as<grimm::Amount>();
              long long cac_premine = int64_t(pre_cac)/100000000; //to show in coins, not in sats (10*8)
              settings.setcac_premine(cac_premine);

              grimm::Amount pre1_cac = vm[cli::CAC_EMISSION_BLOCK].as<grimm::Amount>();
              long long cac_blockreward = int64_t(pre1_cac)/100000000; //to show in coins, not in sats (10*8)
              settings.setcac_blockreward(cac_blockreward);

              grimm::Amount pre2_cac = vm[cli::CAC_EMISSION_DROP0].as<grimm::Amount>();
              long long cac_drop0 = int64_t(pre2_cac);
              settings.setcac_drop0(cac_drop0);

              grimm::Amount pre3_cac = vm[cli::CAC_EMISSION_DROP1].as<grimm::Amount>();
              long long cac_drop1 = int64_t(pre3_cac);
              settings.setcac_drop1(cac_drop1);

              grimm::Height pre4_cac = vm[cli::CAC_MATURITY_COINBASE].as<grimm::Height>();
              int cac_coinbasematurity = int64_t(pre4_cac);
              settings.setcac_coinbasematurity(cac_coinbasematurity);

              grimm::Height pre5_cac = vm[cli::CAC_MATURITY_STANDART].as<grimm::Height>();
              int cac_standartmaturity = int64_t(pre5_cac);
              settings.setcac_standartmaturity(cac_standartmaturity);

              uint32_t pre6_cac = vm[cli::CAC_BLOCK_TIME].as<uint32_t>();
              int cac_blocktime = int32_t(pre6_cac);
              settings.setcac_blocktime(cac_blocktime);

              uint32_t pre7_cac = vm[cli::CAC_DIFF_S].as<uint32_t>();
              int cac_diff = int32_t(pre7_cac);
              settings.setcac_diff(cac_diff);



            if (settings.getNodeAddress().isEmpty())
            {
                if (vm.count(cli::NODE_ADDR))
                {
                    string nodeAddr = vm[cli::NODE_ADDR].as<string>();
                    settings.setNodeAddress(nodeAddr.c_str());
                }
            }

            qmlRegisterSingletonType<Theme>(
                    "Grimm.Wallet", 1, 0, "Theme",
                    [](QQmlEngine* engine, QJSEngine* scriptEngine) -> QObject* {
                        Q_UNUSED(engine)
                        Q_UNUSED(scriptEngine)
                        return new Theme;
                    });
            qmlRegisterType<StartViewModel>("Grimm.Wallet", 1, 0, "StartViewModel");
            qmlRegisterType<LoadingViewModel>("Grimm.Wallet", 1, 0, "LoadingViewModel");
            qmlRegisterType<MainViewModel>("Grimm.Wallet", 1, 0, "MainViewModel");
            qmlRegisterType<DashboardViewModel>("Grimm.Wallet", 1, 0, "DashboardViewModel");
            qmlRegisterType<WalletViewModel>("Grimm.Wallet", 1, 0, "WalletViewModel");
            qmlRegisterType<UtxoViewStatus>("Grimm.Wallet", 1, 0, "UtxoStatus");
            qmlRegisterType<UtxoViewType>("Grimm.Wallet", 1, 0, "UtxoType");
            qmlRegisterType<UtxoViewModel>("Grimm.Wallet", 1, 0, "UtxoViewModel");
            qmlRegisterType<SettingsViewModel>("Grimm.Wallet", 1, 0, "SettingsViewModel");
            qmlRegisterType<AddressBookViewModel>("Grimm.Wallet", 1, 0, "AddressBookViewModel");
            qmlRegisterType<NotificationsViewModel>("Grimm.Wallet", 1, 0, "NotificationsViewModel");
            qmlRegisterType<HelpViewModel>("Grimm.Wallet", 1, 0, "HelpViewModel");
            qmlRegisterType<MessagesViewModel>("Grimm.Wallet", 1, 0, "MessagesViewModel");
            qmlRegisterType<StatusbarViewModel>("Grimm.Wallet", 1, 0, "StatusbarViewModel");

            qmlRegisterType<AddressItem>("Grimm.Wallet", 1, 0, "AddressItem");
            qmlRegisterType<ContactItem>("Grimm.Wallet", 1, 0, "ContactItem");
            qmlRegisterType<TxObject>("Grimm.Wallet", 1, 0, "TxObject");
            qmlRegisterType<UtxoItem>("Grimm.Wallet", 1, 0, "UtxoItem");
            qmlRegisterType<DeviceItem>("Grimm.Wallet", 1, 0, "DeviceItem");

            qmlRegisterType<PaymentInfoItem>("Grimm.Wallet", 1, 0, "PaymentInfoItem");
            qmlRegisterType<WalletDBPathItem>("Grimm.Wallet", 1, 0, "WalletDBPathItem");

            engine.load(QUrl("qrc:/root.qml"));

            if (engine.rootObjects().count() < 1)
            {
                LOG_ERROR() << "Probmlem with QT";
                return -1;
            }

            QObject* topLevel = engine.rootObjects().value(0);
            QQuickWindow* window = qobject_cast<QQuickWindow*>(topLevel);

            if (!window)
            {
                LOG_ERROR() << "Probmlem with QT";
                return -1;
            }

            window->setMinimumSize(QSize(768, 540));
            window->setFlag(Qt::WindowFullscreenButtonHint);

            window->show();

            return app.exec();
        }
        catch (const po::error& e)
        {
            LOG_ERROR() << e.what();
            return -1;
        }
    }
    catch (const std::exception& e)
    {
        QMessageBox msgBox;
        msgBox.setText(e.what());
        msgBox.exec();
        return -1;
    }
}
