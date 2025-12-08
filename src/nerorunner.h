/*  Nero Launcher: A very basic Bottles-like manager using UMU.
    Proton Runner backend.

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

#ifndef NERORUNNER_H
#define NERORUNNER_H

#include "nerofs.h"

#include <QString>
#include <QProcessEnvironment>
#include <QFile>

class NeroRunner : public QObject
{
    Q_OBJECT
public:
    NeroRunner() {};

    int StartShortcut(const QString &, const bool & = false);
    int StartOnetime(const QString &, const bool & = false, const QStringList & = {});
    QString GetHash() {return hashVal;}
    void WaitLoop(QProcess &, QFile &);
    void StopProcess();
    void InitCache();
    QSettings *settings = NeroFS::GetCurrentPrefixCfg();
    bool halt = false;
    bool loggingEnabled = false;
    QProcessEnvironment env;
    enum {
        RunnerStarting = 0,
        RunnerUpdated,
        RunnerProtonStarted,
        RunnerProtonStopping,
        RunnerProtonStopped
    } RunnerStatus_e;

    struct NeroSetting {
    public:
        QString shortcutSetting;
        QString prefixSetting;
        QVariant settingVariant;
        NeroSetting(){}
        static NeroSetting init(QString &settingName, NeroRunner &parent) {
            NeroSetting currentSetting(settingName, parent);
            QString val = currentSetting.DetermineValue(parent);
            currentSetting.SetSettingValue(val);
            return currentSetting;
        }

        QStringList GetSetingList() {return settingVariant.toStringList();}
        void setValue() {}
        QVariant GetSettingVariant() {return settingVariant;}

        QString DetermineValue(NeroRunner &parent) {
            QString query = hasShortcutSetting() ? shortcutSetting : prefixSetting;
            settingVariant = parent.settings->value(query);
            // TODO: Change to switch
            if (settingVariant == QVariant()) {
                return "";
            } else if (settingVariant.canConvert<QString>()) {
                return settingVariant.toString();
            } else if (settingVariant.canConvert<QStringList>()) {
                return settingVariant.toStringList().join(";");
            } else if (settingVariant.canConvert<bool>()) {
                return QString::number(settingVariant.toBool());
            }
            return settingVariant.toString();
        }

        bool hasShortcutSetting() {
            return CheckSetting(shortcutSetting);
        }

        bool HasSetting() {
            return CheckSetting(shortcutSetting) || CheckSetting(prefixSetting);
        }

        bool CheckSetting(QString setting) {
            return settingVariant != QVariant(); //check if its a default variant for missing property
        }
        void SetSettingValue(QString settingValue) {this->settingValue = settingValue;}
        QString GetSettingValue() {return settingValue;}

    private:
        NeroSetting (QString &settingName, NeroRunner &parent) {
            QString hash = parent.GetHash();
            this->shortcutSetting =  shortcuts.replace("%s", hash) + settingName;
            this->prefixSetting = prefixSettings + settingName;
        }
        QString settingValue;
        QString shortcuts = "Shortcuts--%s/";
        QString prefixSettings = "PrefixSettings/";
    };
private:
    // TODO: All Structs are TBD, im just fuckin around with what these could be
    // enum {
    //     SHORTCUTS = 0,
    //     PREFIX_SETTINGS
    // } PrefixSetting_e;

    // struct Command {
    //     PrefixSetting_e settingLocation;
    //     QString command;
    // };


    struct GamescopeFilter {
        QString scalingType;
        int scalingValue;
    };

    struct Resolution {
        QString property;
        QString width;
        QString height;
    };

    //TODO: Fix all of the struct stuff.
    struct NeroConfig {
        const QString dlssUpgrade = "DlssUpgrade";
        const QString dlssIndicator = "DlssIndicator";
        const QString nvidiaLibs = "NvidiaLibs";
        const QString fsr4 = "Fsr4";
        const QString fsr4Rdna3 = "Fsr4Rdna3";
        const QString xessUpgrade = "XessUpgrade";
        const QString disableWindowDeclartion = "NoDecoration";
        const QString noSteamInput = "NoSteamInput";
        const QString wineCpuTopology = "WineCpuTopology";

    };
    QString dlssIndicator = "DlssIndicator";
    QString nvidiaLibs = "NvidiaLibs";
    QString fsr4 = "Fsr4";
    QString fsr4Rdna3 = "Fsr4Rdna3";
    QString xessUpgrade = "XessUpgrade";
    QString disableWindowDeclartion = "NoDecoration";
    QString noSteamInput = "NoSteamInput";
    QString wineCpuTopology = "WineCpuTopology";

    QString FALSE = "0";
    QString TRUE = "1";
    // TODO: set it up programatically somewhere.
    QString shortcuts = "Shortcuts--[]/";
    QString prefixSettings = "PrefixSettings/";
    QString path = "Path";
    QString cPath = "C:/";
    QString prerunScript = "PreRunScript";
    QString wineprefix = "WINEPREFIX";
    QString gameId = "GAMEID";

    QString currentRunner = prefixSettings + "CurrentRunner";
    QString protonPath = "PROTONPATH";
    QString runtimeUpdate = prefixSettings + "RuntimeUpdateOnLaunch";
    QString umuRuntimeUpdate = "UMU_RUNTIME_UPDATE";

    QString protonVerb = "PROTON_VERB";
    QString run = "run";
    QString waitForExitRun = "waitforexitandrun";

    QString sdlUseButtonLabels = "SDL_GAMECONTROLLER_USE_BUTTON_LABELS";

    QString dllOverrideStr = "DLLoverrides";
    QString ignoreGlobalDlls = "IgnoreGlobalDLLs";
    QString wineDllOverrides = "WINEDLLOVERRIDES";

    QString forceWineD3D = "ForceWineD3D";
    QString protonUseWineD3D = "PROTON_USE_WINED3D";
    QString noD8VK = "NoD8VK";
    QString protonDxvkD3D8 = "PROTON_DXVK_D3D8";

    QString enableNvApi = "EnableNVAPI";
    QString protonForceNvapi = "PROTON_FORCE_NVAPI";
    QString limitGlExtensions = "LimitGLextensions";
    QString protonOldGl = "PROTON_OLD_GL_STRING";
    QString vkCapture = "VKcapture";
    QString obsVkCapture = "OBS_VKCAPTURE";
    QString forceIGpu = "ForceiGPU";
    QString forceDefaultDevice = "MESA_VK_DEVICE_SELECT_FORCE_DEFAULT_DEVICE";

    QString limitFps = "LimitFPS";

    QString dxvkFrameRate = "DXVK_FRAME_RATE";

    QString fileSyncMode = "FileSyncMode";

    QString ge109 = "GE-Proton10-9";

    QString protonUseNtSync = "PROTON_USE_NTSYNC";

    QString protonNoNtSync = "PROTON_NO_NTSYNC";

    QString protonNoEsync = "PROTON_NO_ESYNC";
    QString protonNoFsync = "PROTON_NO_FSYNC";
    QString gamemode = "Gamemode";
    QString gamemoderun = "gamemoderun";

    QString scalingMode = "Scaling Mode";

    QString intScaling = "WINE_FULLSCREEN_INTEGER_SCALING";
    QString fsrScaling = "WINE_FULLSCREEN_FSR";
    QString fsrStrength = "WINE_FULLSCREEN_FSR_STRENGTH";

    QString fsrCustom = "WINE_FULLSCREEN_FSR_CUSTOM_MODE";
    QString fsrCustomW = "FSRcustomResW";
    QString fsrCustomH = "FSRcustomResH";
    // TODO: Standardize
    QString gamescopeOutResH = "GamescopeOutResH";
    QString gamescopeOutResW = "GamescopeOutResW";
    QString fsrArg = "-F";
    QString fullscreenArg = "-f";
    QString widthArg = "-w";
    QString heightArg = "-h";

    QString gamescopeFilter = "GamescopeFilter";
    QString debugOutput = "DebugOutput";

    QString allowHidraw = "AllowHidraw";
    QString useXalia = "UseXalia";
    QString protonPreferSdl = "PROTON_PREFER_SDL";
    QString protonHiDraw = "PROTON_ENABLE_HIDRAW";
    QString protonUseXalia = "PROTON_USE_XALIA";

    QString waylandDisplay = "WAYLAND_DISPLAY";
    QString useWayland = "UseWayland";
    QString protonEnableWayland = "PROTON_ENABLE_WAYLAND";

    QString useHdr = "/UseHdr";
    QString protonUseHdr = "PROTON_ENABLE_HDR";

    QString mangohudStr = "Mangohud";
    QString gamescope = "gamescope";
    QString mangoappArg = "--mangoapp";

    QString postRunScriptStr = "PostRunScript";

    void InitDebugProperties(QString property);
    QString hashVal;
signals:
    void StatusUpdate(int);
};
namespace {
    namespace ProtonSettings {
        const QString dlssUpgrade = "PROTON_DLSS_UPGRADE"; //(Upgrade DLSS to latest version) (cachyos)
        const QString dlssIndicator = "PROTON_DLSS_INDICATOR"; //Show watermark when DLSS is working) (cachyos)
        const QString nvidiaLibs = "PROTON_NVIDIA_LIBS"; //(Enables NVIDIA libraries (PhysX, CUDA)) (cachyos)
        const QString fsr4Upgrade = "PROTON_FSR4_UPGRADE"; //(Enable FSR4 for RDNA4 cards) (GE, cachyos, EM)
        const QString fsr4Rdna3 = "PROTON_FSR_RDNA3_UPGRADE"; //(Enable FSR4 for RDNA3 cards) (GE, cachyos, EM)
        const QString fsr4Indicator = "PROTON_FSR4_INDICATOR"; //(Show watermark when FSR4 is working) (cachyos, EM)
        const QString xessUpgrade = "PROTON_XESS_UPGRADE"; //(Upgrade XeSS to latest version) (cachyos)
        const QString noWindowDecoration = "PROTON_NO_WM_DECORATION"; //(Disable window decorations) (GE, cachyos)
        const QString localShaderCache = "PROTON_LOCAL_SHADER_CACHE"; //(Disable per-game shader cache) (cachyos)
        const QString noSteamInput = "PROTON_NO_STEAMINPUT"; //(Disable Steam Input) (GE, cachyos)
        const QString wineCpuTopology = "WINE_CPU_TOPOLOGY"; //(Set process affinity. If umu-protonfixes support is implemented in Nero, probably irrelevant)

        const QString useWineD3D = "PROTON_USE_WINED3D";
        const QString dxvkD3D8 = "PROTON_DXVK_D3D8";
        const QString useHdr = "PROTON_ENABLE_HDR";
        const QString oldGl = "PROTON_OLD_GL_STRING";
        const QString forceNvapi = "PROTON_FORCE_NVAPI";
        const QString umuRuntimeUpdate = "UMU_RUNTIME_UPDATE";
        const QString sdlUseButtonLabels = "SDL_GAMECONTROLLER_USE_BUTTON_LABELS";
        const QString wineDllOverrides = "WINEDLLOVERRIDES";
        const QString intScaling = "WINE_FULLSCREEN_INTEGER_SCALING";
        const QString fsrScaling = "WINE_FULLSCREEN_FSR";
        const QString fsrStrength = "WINE_FULLSCREEN_FSR_STRENGTH";
        const QString fsrCustom = "WINE_FULLSCREEN_FSR_CUSTOM_MODE";
        const QString preferSdl = "PROTON_PREFER_SDL";
        const QString hiDraw = "PROTON_ENABLE_HIDRAW";
        const QString useXalia = "PROTON_USE_XALIA";
        const QString waylandDisplay = "WAYLAND_DISPLAY";
        const QString useNtSync = "PROTON_USE_NTSYNC";
        const QString useWow64 = "PROTON_USE_WOW64";
        const QString noNtSync = "PROTON_NO_NTSYNC";
        const QString noEsync = "PROTON_NO_ESYNC";
        const QString noFsync = "PROTON_NO_FSYNC";
        const QString verb = "PROTON_VERB";
        const QString dxvkFrameRate = "DXVK_FRAME_RATE";
        //TODO: Use enum names
        enum {
            PROTON_USE_NSYNC,
            PROTON_NO_NTSYNC,
            PROTON_NO_ESYNC,
            PROTON_NO_FSYNC
        }Sync_e;
        enum {
            WINE_FULLSCREEN_INTEGER_SCALING,
            WINE_FULLSCREEN_FSR,
            WINE_FULLSCREEN_FSR_STRENGTH,
            WINE_FULLSCREEN_FSR_CUSTOM_MODE
        }WineScaling_e;
        }
}
#endif // NERORUNNER_H
