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
        QVariant value;
        NeroSetting(){}
        NeroSetting (QString &settingName, NeroRunner &parent) {
            value = parent.settings->value(settingName);
            QString hash = parent.GetHash();
            this->shortcutSetting =  shortcuts.replace("%s", hash) + settingName;
            this->prefixSetting = prefixSetting + settingName;
        }

        QStringList GetSetingList() {
            return value.toStringList();
        }
        QVariant GetValue() {return value;}
        QString GetSetting() {
            if (value == QVariant()) {
                return "";
            } else if (value.canConvert<QString>()) {
                return value.toString();
            } else if (value.canConvert<QStringList>()) {
                return value.toStringList().join(";");
            } else if (value.canConvert<bool>()) {
                return QString::number(value.toBool());
            }
            return value.toString();
        }

        bool hasShortcutSetting() {
            return CheckSetting(shortcutSetting);
        }

        bool HasSetting() {
            return CheckSetting(shortcutSetting) || CheckSetting(prefixSetting);
        }

        bool CheckSetting(QString setting) {
            return value != QVariant(); //check if its a default variant for missing property
        }
    private:
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
    struct ProtonSettings {
        inline static const QString dlssUpgrade = "PROTON_DLSS_UPGRADE"; //(Upgrade DLSS to latest version) (cachyos)
        inline static const QString dlssIndicator = "PROTON_DLSS_INDICATOR"; //Show watermark when DLSS is working) (cachyos)
        inline static const QString nvidiaLibs = "PROTON_NVIDIA_LIBS"; //(Enables NVIDIA libraries (PhysX, CUDA)) (cachyos)
        inline static const QString fsr4Upgrade = "PROTON_FSR4_UPGRADE"; //(Enable FSR4 for RDNA4 cards) (GE, cachyos, EM)
        inline static const QString fsr4Rdna3 = "PROTON_FSR_RDNA3_UPGRADE"; //(Enable FSR4 for RDNA3 cards) (GE, cachyos, EM)
        inline static const QString fsr4Indicator = "PROTON_FSR4_INDICATOR"; //(Show watermark when FSR4 is working) (cachyos, EM)
        inline static const QString xessUpgrade = "PROTON_XESS_UPGRADE"; //(Upgrade XeSS to latest version) (cachyos)
        inline static const QString noWindowDecoration = "PROTON_NO_WM_DECORATION"; //(Disable window decorations) (GE, cachyos)
        inline static const QString localShaderCache = "PROTON_LOCAL_SHADER_CACHE"; //(Disable per-game shader cache) (cachyos)
        inline static const QString noSteamInput = "PROTON_NO_STEAMINPUT"; //(Disable Steam Input) (GE, cachyos)
        inline static const QString wineCpuTopology = "WINE_CPU_TOPOLOGY"; //(Set process affinity. If umu-protonfixes support is implemented in Nero, probably irrelevant)
        
        inline static const QString useWineD3D = "PROTON_USE_WINED3D";
        inline static const QString dxvkD3D8 = "PROTON_DXVK_D3D8";
        inline static const QString useHdr = "PROTON_ENABLE_HDR";
        inline static const QString oldGl = "PROTON_OLD_GL_STRING";
        inline static const QString forceNvapi = "PROTON_FORCE_NVAPI";
        inline static const QString umuRuntimeUpdate = "UMU_RUNTIME_UPDATE";
        inline static const QString sdlUseButtonLabels = "SDL_GAMECONTROLLER_USE_BUTTON_LABELS";
        inline static const QString wineDllOverrides = "WINEDLLOVERRIDES";
        inline static const QString intScaling = "WINE_FULLSCREEN_INTEGER_SCALING";
        inline static const QString fsrScaling = "WINE_FULLSCREEN_FSR";
        inline static const QString fsrStrength = "WINE_FULLSCREEN_FSR_STRENGTH";
        inline static const QString fsrCustom = "WINE_FULLSCREEN_FSR_CUSTOM_MODE";
        inline static const QString preferSdl = "PROTON_PREFER_SDL";
        inline static const QString hiDraw = "PROTON_ENABLE_HIDRAW";
        inline static const QString useXalia = "PROTON_USE_XALIA";
        inline static const QString waylandDisplay = "WAYLAND_DISPLAY";
        inline static const QString useNtSync = "PROTON_USE_NTSYNC";
        inline static const QString useWow64 = "PROTON_USE_WOW64";
        inline static const QString noNtSync = "PROTON_NO_NTSYNC";
        inline static const QString noEsync = "PROTON_NO_ESYNC";
        inline static const QString noFsync = "PROTON_NO_FSYNC";
        inline static const QString verb = "PROTON_VERB";
        inline static const QString dxvkFrameRate = "DXVK_FRAME_RATE";
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
    };
    struct NeroConfig {
        static inline const QString dlssUpgrade = "DlssUpgrade";
        static inline const QString dlssIndicator = "DlssIndicator";
        static inline const QString nvidiaLibs = "NvidiaLibs";
        static inline const QString fsr4 = "Fsr4";
        static inline const QString fsr4Rdna3 = "Fsr4Rdna3";
        static inline const QString xessUpgrade = "XessUpgrade";
        static inline const QString disableWindowDeclartion = "NoDecoration";
        static inline const QString noSteamInput = "NoSteamInput";
        static inline const QString wineCpuTopology = "WineCpuTopology";

    };
    QString dlssUpgrade = "DlssUpgrade";
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

    QString dllOverride = "DLLoverrides";
    QString ignoreGlobalDlls = "IgnoreGlobalDLLs";
    QString wineDllOverrides = "WINEDLLOVERRIDES";
    QString dllOverridePrefix = prefixSettings + dllOverride;

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

    bool HasSetting(QString setting) {
        //setting.toStringList.isEmpty() -- false
        //non empty list returned as true
        return !settings->value(setting).toStringList().isEmpty() || !settings->value(setting).toString().isEmpty();
    }

    QString GetProperty(QString& setting) {
        QVariant v = settings->value(setting);
        if (v.canConvert<QString>()) {
            return v.toString();
        } else if (v.canConvert<QStringList>()) {
            return v.toStringList().join(";");
        } else if (v.canConvert<bool>()) {
            return QString::number(settings->value(setting).toBool());
        }
        return settings->value(setting).toString();
    }

    // TODO: Rename to be more informative.
    bool IsMissingSetting(QString setting) {
        return !settings->value(setting).toBool();
    }

    void addProperty(QString field, QString property) {
        QString shortcut = shortcuts + field;
        QString prefix = prefixSettings + field;
        if ((HasSetting(shortcut) && !IsMissingSetting(shortcut)) || !IsMissingSetting(prefix)) {
            env.insert(property, TRUE);
        }
    }
    QString hashVal;
    NeroSetting setting(QString property);
signals:
    void StatusUpdate(int);
};

#endif // NERORUNNER_H
