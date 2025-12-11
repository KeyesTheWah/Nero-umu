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
#include <QStringBuilder>
#include <qvariant.h>

class NeroRunner : public QObject
{
    Q_OBJECT
public:
    NeroRunner() {};

    int StartShortcut(const QString &, const bool & = false);
    int StartOnetime(const QString &, const bool & = false, const QStringList & = {});
    QString GetHash() {return hashVal;}
    void WaitLoop(QProcess &, QFile &);
    void writeToLog(QStringList lines);
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
        NeroSetting(){}

        NeroSetting (const QString settingName, NeroRunner &parent) {
            QString hash = parent.GetHash();
            this->shortcutSetting =  shortcuts % hash % '/' % settingName;
            this->prefixSetting = prefixSettings % '/' % settingName;
            shortcut = parent.settings->value(shortcutSetting);
            prefix = parent.settings->value(prefixSetting);
            if (shortcut != QVariant()) { //blank QVariant means its default and has no value
                settingVariant = shortcut;
                this->hasShortcut = true;
            } else {
                settingVariant = prefix;
            }
        }

        int toInt() { return settingVariant.toInt(); }

        bool toBool() { return settingVariant.toBool(); }

        QString toString() { return settingVariant.toString(); }

        bool hasShortcutSetting() { return hasShortcut; }

        bool hasSetting() { return shortcutSetting != QVariant() || prefixSetting != QVariant(); }

        bool hasSettingAndToBool() { return hasSetting() && settingVariant.toBool(); }

        QStringList toStringList() { return settingVariant.toStringList(); }
        
        QVariant getSettingVariant() { return settingVariant; }
        
        QString convertBoolToIntString() { return QString::number(settingVariant.toBool()); }
    private:
        bool hasShortcut = false;
        QString shortcutSetting;
        QString prefixSetting;
        QVariant shortcut;
        QVariant prefix;
        QVariant settingVariant;
        const QString shortcuts = "Shortcuts--";
        const QString prefixSettings = "PrefixSettings";
    };
private:
    QString GamescopeFilterType(int filterVal);
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
    };
    struct GamescopeFilter {
        QString scalingType;
        int scalingValue;
    };

    struct Resolution {
        NeroSetting property;

    };
    const QString FALSE = "0";
    const QString TRUE = "1";

    const QString cDrive = "C:/";
    const QString drive_c = "drive_c/";

    const QString ge109 = "GE-Proton10-9";
    void InitSimpleBoolSettings();
    void InitDebugProperties(int value);
    QString hashVal;

    
signals:
    void StatusUpdate(int);
};

namespace CliArgs {
    const QString doubleDash = "--";


    //Wine Compat Options
    namespace Wine {
        const QString prefix = "WINEPREFIX";
        const QString dllOverrides = "WINEDLLOVERRIDES";
        const QString cpuTopology = "WINE_CPU_TOPOLOGY"; //(Set process affinity. If umu-protonfixes support is implemented in Nero, probably irrelevant)
    }

    //Wayland/HDR
    const QString waylandDisplay = "WAYLAND_DISPLAY";

    //Controller Stuff
    const QString sdlUseButtonLabels = "SDL_GAMECONTROLLER_USE_BUTTON_LABELS";

    // Sync Options
    namespace Proton {
        const QString localShaderCache = "PROTON_LOCAL_SHADER_CACHE"; //(Enable per-game shader cache even if "Shader Pre-caching is disabled) (cachyos)
        const QString noSteamInput = "PROTON_NO_STEAMINPUT"; //(Disable Steam Input) (GE, cachyos)
        const QString useWineD3D = "PROTON_USE_WINED3D";
        const QString dxvkD3D8 = "PROTON_DXVK_D3D8";
        const QString oldGl = "PROTON_OLD_GL_STRING";

        //Wayland/HDR
        const QString enableWayland = "PROTON_ENABLE_WAYLAND";
        const QString useHdr = "PROTON_ENABLE_HDR";
        const QString noWindowDecoration = "PROTON_NO_WM_DECORATION"; //(Disable window decorations, good for Wayland) (GE, cachyos)
        const QString preferSdl = "PROTON_PREFER_SDL";
        const QString hiDraw = "PROTON_ENABLE_HIDRAW";
        const QString useXalia = "PROTON_USE_XALIA";
        namespace Sync {
            const QString ntSync = "PROTON_USE_NTSYNC";
            const QString noNtSync = "PROTON_NO_NTSYNC";
            const QString noEsync = "PROTON_NO_ESYNC";
            const QString noFsync = "PROTON_NO_FSYNC";
        }
        namespace Nvidia {
            //Nvidia Launch Arguments
            const QString forceNvapi = "PROTON_FORCE_NVAPI";
            const QString dlssUpgrade = "PROTON_DLSS_UPGRADE"; //(Upgrade DLSS to latest version) (cachyos)
            const QString libs = "PROTON_NVIDIA_LIBS"; //(Enables NVIDIA libraries (PhysX, CUDA)) (cachyos)
            const QString dlssIndicator = "PROTON_DLSS_INDICATOR"; //Show watermark when DLSS is working) (cachyos)
        }
        namespace Amd {
            //AMD Launch Arguments
                const QString fsr4Upgrade = "PROTON_FSR4_UPGRADE"; //(Enable FSR4 for RDNA4 cards) (GE, cachyos, EM)
                const QString fsr4Rdna3 = "PROTON_FSR_RDNA3_UPGRADE"; //(Enable FSR4 for RDNA3 cards) (GE, cachyos, EM)
                const QString fsr4Indicator = "PROTON_FSR4_INDICATOR"; //(Show watermark when FSR4 is working) (cachyos, EM)
        }
        namespace Intel {
            //XESS Launch Arguments
            const QString xessUpgrade = "PROTON_XESS_UPGRADE"; //(Upgrade XeSS to latest version) (cachyos)
        }
    }
    // Misc
    const QString obsVkCapture = "OBS_VKCAPTURE";
    const QString protonPath = "PROTONPATH";
    const QString mangoapp = "--mangoapp";
    const QString forceIgpu = "MESA_VK_DEVICE_SELECT_FORCE_DEFAULT_DEVICE";
    const QString umuRuntimeUpdate = "UMU_RUNTIME_UPDATE";
    const QString gamemoderun = "gamemoderun";
    const QString gameId = "GAMEID";
    const QString useWow64 = "PROTON_USE_WOW64";

    const QString verb = "PROTON_VERB";
    const QString run = "run";
    const QString waitForExitRun = "waitforexitandrun";

    // TODO: Remove
    const QString dxvkFrameRate = "DXVK_FRAME_RATE";

    namespace Gamescope {
        const QString name = "gamescope";
        const QString intScaling = "WINE_FULLSCREEN_INTEGER_SCALING"; //no fsr
        const QString fsrScaling = "WINE_FULLSCREEN_FSR"; //enables/disables fsr
        const QString fsrStrength = "WINE_FULLSCREEN_FSR_STRENGTH"; //strength; can be 1-5, enum?
        const QString fsrCustom = "WINE_FULLSCREEN_FSR_CUSTOM_MODE"; //manaully set FSR width/height.
        const QString fullscreen = "-f";
        const QString height = "-h";
        const QString width = "-w";
        namespace Upscaler {
            const QString nearest = "nearest";
            const QString fsr = "fsr";
            const QString nvidiaImageSharpening = "nis";
            const QString pixel = "pixel";
        }

        const QString filter = "-F";
        const QString fpsLimit = "-r";
        const QString unfocusedFpsLimit = "-o";
        const QString borderless = "-b";
        const QString windowedWidth = "-W";
        const QString windowedHeight = "-H";
    }

        // // TODO: Use enum names somehow
        // enum {
        //     PROTON_USE_NSYNC,
        //     PROTON_NO_NTSYNC,
        //     PROTON_NO_ESYNC,
        //     PROTON_NO_FSYNC
        // }Sync_e;
        // enum {
        //     WINE_FULLSCREEN_INTEGER_SCALING,
        //     WINE_FULLSCREEN_FSR,
        //     WINE_FULLSCREEN_FSR_STRENGTH,
        //     WINE_FULLSCREEN_FSR_CUSTOM_MODE
        // }WineScaling_e;
}
namespace NeroConfig {
    const QString name = "Name";
    const QString currentRunner = "CurrentRunner";
    const QString runtimeUpdate = "RuntimeUpdateOnLaunch";
    const QString dlssIndicator = "DlssIndicator";


    const QString path = "Path";
    //OBS
    const QString vkCapture = "VKcapture";

    //Force Integrated GPU
    const QString forceIGpu = "ForceiGPU";


    namespace Proton {
        const QString forceWineD3D = "ForceWineD3D";
        const QString allowHidraw = "AllowHidraw";
        const QString useXalia = "UseXalia";
        const QString noD8VK = "NoD8VK";
        const QString useWayland = "UseWayland";
        const QString useHdr = "UseHdr";
        const QString limitGlExtensions = "LimitGLextensions";
    }

    // TODO: Standardize Resolutions
    namespace Gamescope{
        //FSR Custom Resolutions
        const QString scalingMode = "ScalingMode";
        const QString fsrCustomW = "FSRcustomResW";
        const QString fsrCustomH = "FSRcustomResH";
        const QString filter = "GamescopeFilter";
        const QString outputResH = "GamescopeOutResH";
        const QString outputResW = "GamescopeOutResW";
        const QString windowedResH = "GamescopeWinResH";
        const QString windowedResW = "GamescopeWinResW";
    }
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
    const QString fsr4Upgrade = "Fsr4Upgrade";
    const QString fsr4Indicator = "Fsr4Indicator";
    const QString fsr4Rdna3 = "Fsr4Rdna3";
    const QString xessUpgrade = "XessUpgrade";
    const QString noWindowDecoration = "NoDecoration";
    const QString noSteamInput = "NoSteamInput";
    const QString wineCpuTopology = "WineCpuTopology";
    const QString localShaderCache = "LocalShaderCache";
    const QString prerunScript = "PreRunScript";
    const QString postRunScript = "PostRunScript";
    const QString mangohud = "Mangohud";
}
#endif // NERORUNNER_H
