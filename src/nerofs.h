/*  Nero Launcher: A very basic Bottles-like manager using UMU.
    Host Filesystem Management.

    Copyright (C) 2024 That One Seong

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef NEROFS_H
#define NEROFS_H

#include <QDir>
#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>

class NeroFS
{
private:
    // VARS
    static QDir prefixesPath;
    static QDir protonsPath;
    static QString currentPrefix;
    static QString currentRunner;
    static QString currentUMU;
    static QSettings managerCfg;
    static QStringList currentPrefixOverrides;
    static QStringList prefixes;
    static QStringList availableProtons;

public:
    NeroFS();
    struct CustomRunner {
        QStringList validOptions;
        QStringList reconstructUpgrades;
        bool isCustomProton = true;
        bool isProton10OrLater = true;
        bool isNtSync = true;
        CustomRunner(QString runner) {
            if (runner.contains("EM-")) {
                int version = runner.mid(runner.lastIndexOf("EM-"), 2).toInt();
                this->isProton10OrLater = version > 10;
                this->validOptions = universalOptions << emOptions;
                this->reconstructUpgrades = {"fsr4", "fsr4rdna3"};
            } else if (runner.contains("GE-") || runner.contains("-GE")) {
                int version = runner.mid(runner.lastIndexOf("-") - 2 , 2).toInt();
                int subVersion = runner.mid(runner.lastIndexOf("-") + 1, 2).toInt();
                this->isProton10OrLater = version >= 10;
                this->validOptions = universalOptions << geOptions;
                this->reconstructUpgrades = {"fsr4", "fsr4rdna3"};
                this->isNtSync = isProton10OrLater && subVersion > 9;
            } else if (runner.contains("Cachy")) {
                this->validOptions = universalOptions << cachyOsOptions;
                this->reconstructUpgrades = {"dlss", "xess", "fsr4", "fsr4rdna3"};
            } else {
                this->isProton10OrLater = false;
                this->isCustomProton = false;
                this->isNtSync = false;
            }
        }
    private:
        QStringList universalOptions = {
            "UseWayland",
            "UseWaylandHdr",
            "ImageReconstructionUpgrade",
        };
        QStringList geOptions = {
            "SteamInputDisabled",
            "NoWindowDecorations",
        };
        //none atm but just in case?
        QStringList emOptions = {
        };
        QStringList cachyOsOptions = {
            "UseNvidiaLibs",
            "ImageReconstructionIndicator",
            "UseLocalShaderCache",
            "SteamInputDisabled",
            "NoWindowDecorations",
        };
    };

    // METHODS
    static bool InitPaths();
    static QDir* GetPrefixesPath() { return &prefixesPath; }
    static QDir* GetProtonsPath() { return &protonsPath; }
    static QString GetCurrentPrefix() { return currentPrefix; }
    static QString GetCurrentRunner() { return currentRunner; }
    static QStringList GetCurrentOverrides() { return currentPrefixOverrides; }
    static QStringList* GetAvailableProtons();
    static QStringList GetPrefixes();
    static QStringList GetCurrentPrefixShortcuts();
    static QMap<QString, QVariant> GetCurrentPrefixSettings();
    static QMap<QString, QString> GetCurrentShortcutsMap();
    static QMap<QString, QVariant> GetShortcutSettings(const QString &);
    static QSettings* GetManagerCfg() { return &managerCfg; }
    static void openLogDirectory();
    static void CreateUserLinks(const QString &);
    static void AddNewPrefix(const QString &, const QString &);
    static void AddNewShortcut(const QString &, const QString &, const QString &);
    static bool DeletePrefix(const QString &);
    static void DeleteShortcut(const QString &);

    static QSettings* GetCurrentPrefixCfg();
    static QString GetIcoextract();
    static QString GetIcoutils();
    static QString GetUmU();
    static QString GetWinetricks(const QString & = "");
    static bool SetUmU(const QString & = "");

    static void SetCurrentPrefix(const QString &);
    static bool SetCurrentPrefixCfg(const QString &, const QString &, const QVariant &);
    static void AddNewShortcutSetting(const QString &shortcutHash, const QString &key, const QVariant &value) {
        SetCurrentPrefixCfg(QString("Shortcuts--%1").arg(shortcutHash), key, value);
    }
};

#endif // NEROFS_H
