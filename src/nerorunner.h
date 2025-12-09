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
    void writeToLog(QString... lines);
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

        static NeroSetting init(QString settingName, NeroRunner &parent) {
            NeroSetting currentSetting(settingName, parent);
            QString val = currentSetting.DetermineValue(parent);
            currentSetting.SetSettingValue(val);
            return currentSetting;
        }
        QStringList toList() {return settingVariant.toStringList();}
        QStringList GetSetingList() {return settingVariant.toStringList();}
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

        int toInt() {
            return settingVariant.toInt();
        }

        QString toString() {
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
    struct FilterResolution {
        NeroSetting setting;
        QString width;
        QString height;
    }
    struct GamescopeFilter {
        QString scalingType;
        int scalingValue;
    };

    struct Resolution {
        NeroSetting property;

    };
    QString FALSE = "0";
    QString TRUE = "1";

    QString cPath = "C:/";

    QString ge109 = "GE-Proton10-9";

    void InitDebugProperties(QString property);
    QString hashVal;
    bool HasSetting(QString setting) {
        return !settings->value(setting).toStringList().isEmpty()
               || !settings->value(setting).toString().isEmpty();
    }

    void addProperty(QString field, QString property) {
        NeroSetting setting = NeroSetting::init(field, *this);
        if (setting.HasSetting()) {
            env.insert(property, TRUE);
        }
    }
signals:
    void StatusUpdate(int);
};

namespace {
    namespace ProtonArgs {
        //Nvidia Launch Arguments
        const QString forceNvapi = "PROTON_FORCE_NVAPI";
        const QString dlssUpgrade = "PROTON_DLSS_UPGRADE"; //(Upgrade DLSS to latest version) (cachyos)
        const QString nvidiaLibs = "PROTON_NVIDIA_LIBS"; //(Enables NVIDIA libraries (PhysX, CUDA)) (cachyos)
        const QString dlssIndicator = "PROTON_DLSS_INDICATOR"; //Show watermark when DLSS is working) (cachyos)

        //AMD Launch Arguments
        const QString fsr4Upgrade = "PROTON_FSR4_UPGRADE"; //(Enable FSR4 for RDNA4 cards) (GE, cachyos, EM)
        const QString fsr4Rdna3 = "PROTON_FSR_RDNA3_UPGRADE"; //(Enable FSR4 for RDNA3 cards) (GE, cachyos, EM)
        const QString fsr4Indicator = "PROTON_FSR4_INDICATOR"; //(Show watermark when FSR4 is working) (cachyos, EM)

        //XESS Launch Arguments
        const QString xessUpgrade = "PROTON_XESS_UPGRADE"; //(Upgrade XeSS to latest version) (cachyos)

        const QString localShaderCache = "PROTON_LOCAL_SHADER_CACHE"; //(Disable per-game shader cache) (cachyos)
        const QString noSteamInput = "PROTON_NO_STEAMINPUT"; //(Disable Steam Input) (GE, cachyos)
        const QString wineCpuTopology = "WINE_CPU_TOPOLOGY"; //(Set process affinity. If umu-protonfixes support is implemented in Nero, probably irrelevant)

        //Wine Compat Options
        const QString useWineD3D = "PROTON_USE_WINED3D";
        const QString dxvkD3D8 = "PROTON_DXVK_D3D8";
        const QString oldGl = "PROTON_OLD_GL_STRING";
        const QString wineprefix = "WINEPREFIX";

        //Wayland/HDR
        const QString useHdr = "PROTON_ENABLE_HDR";
        const QString waylandDisplay = "WAYLAND_DISPLAY";
        const QString enableWayland = "PROTON_ENABLE_WAYLAND";
        const QString noWindowDecoration = "PROTON_NO_WM_DECORATION"; //(Disable window decorations, good for Wayland) (GE, cachyos)
        const QString wineDllOverrides = "WINEDLLOVERRIDES";

        //FSR & Gamescope
        const QString gamescope = "gamescope";
        const QString intScaling = "WINE_FULLSCREEN_INTEGER_SCALING"; //no fsr
        const QString fsrScaling = "WINE_FULLSCREEN_FSR"; //enables/disables fsr
        const QString fsrStrength = "WINE_FULLSCREEN_FSR_STRENGTH"; //strength; can be 1-5, enum?
        const QString fsrCustom = "WINE_FULLSCREEN_FSR_CUSTOM_MODE"; //manaully set FSR width/height.
        //Gamescope Args
        const QString fsrArg = "-F";
        const QString fullscreenArg = "-f";
        const QString widthArg = "-w";
        const QString heightArg = "-h";

        //Controller Stuff
        const QString preferSdl = "PROTON_PREFER_SDL";
        const QString hiDraw = "PROTON_ENABLE_HIDRAW";
        const QString useXalia = "PROTON_USE_XALIA";
        const QString sdlUseButtonLabels = "SDL_GAMECONTROLLER_USE_BUTTON_LABELS";

        // Sync Options
        const QString useNtSync = "PROTON_USE_NTSYNC";
        const QString noNtSync = "PROTON_NO_NTSYNC";
        const QString noEsync = "PROTON_NO_ESYNC";
        const QString noFsync = "PROTON_NO_FSYNC";

        // Misc
        const QString obsVkCapture = "OBS_VKCAPTURE";
        const QString protonPath = "PROTONPATH";
        const QString mangoapp = "--mangoapp"; // technically not a proton launch arg but w/e
        const QString forceDefaultDevice = "MESA_VK_DEVICE_SELECT_FORCE_DEFAULT_DEVICE";
        const QString umuRuntimeUpdate = "UMU_RUNTIME_UPDATE";
        const QString gamemoderun = "gamemoderun";
        const QString gameId = "GAMEID";
        const QString useWow64 = "PROTON_USE_WOW64";

        const QString verb = "PROTON_VERB";
        const QString run = "run";
        const QString waitForExitRun = "waitforexitandrun";

        // TODO: Remove
        const QString dxvkFrameRate = "DXVK_FRAME_RATE";

        // TODO: Use enum names somehow
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
namespace {
    namespace NeroConfig {
        const QString name = "Name";
        const QString currentRunner = "CurrentRunner";
        const QString runtimeUpdate = "RuntimeUpdateOnLaunch";
        const QString dlssIndicator = "DlssIndicator";
        const QString forceWineD3D = "ForceWineD3D";
        const QString allowHidraw = "AllowHidraw";
        const QString useXalia = "UseXalia";
        const QString noD8VK = "NoD8VK";
        const QString useWayland = "UseWayland";
        const QString useHdr = "UseHdr";
        const QString path = "Path";
        const QString limitGlExtensions = "LimitGLextensions";

        //FSR Custom Resolutions
        const QString fsrCustomW = "FSRcustomResW";
        const QString fsrCustomH = "FSRcustomResH";

        //OBS
        const QString vkCapture = "VKcapture";

        //Force Integrated GPU
        const QString forceIGpu = "ForceiGPU";

        const QString scalingMode = "Scaling Mode";

        // TODO: Standardize
        const QString gamescopeFilter = "GamescopeFilter";
        const QString gamescopeOutResH = "GamescopeOutResH";
        const QString gamescopeOutResW = "GamescopeOutResW";

        const QString debugOutput = "DebugOutput";
        const QString enableNvApi = "EnableNVAPI";

        const QString limitFps = "LimitFPS";

        const QString fileSyncMode = "FileSyncMode";
        const QString ignoreGlobalDlls = "IgnoreGlobalDLLs";
        const QString dllOverride = "DLLoverrides";
        const QString gamemode = "Gamemode";
        const QString args = "Args";

        //TBD
        const QString nvidiaLibs = "NvidiaLibs";
        const QString fsr4 = "Fsr4";
        const QString fsr4Rdna3 = "Fsr4Rdna3";
        const QString xessUpgrade = "XessUpgrade";
        const QString disableWindowDeclartion = "NoDecoration";
        const QString noSteamInput = "NoSteamInput";
        const QString wineCpuTopology = "WineCpuTopology";
        const QString prerunScript = "PreRunScript";
        const QString postRunScript = "PostRunScript";
        const QString mangohud = "Mangohud";
    }
}
#endif // NERORUNNER_H
