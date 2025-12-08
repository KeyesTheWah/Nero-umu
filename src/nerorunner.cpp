/*  Nero Launcher: A very basic Bottles-like manager using UMU.
    Proton Runner backend.

    Copyright (C) 2024  That One Seong

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

#include "nerorunner.h"
#include "neroconstants.h"
#include "nerofs.h"

#include <QApplication>
#include <QProcess>
#include <QDir>
#include <QDebug>

int NeroRunner::StartShortcut(const QString &hash, const bool &prefixAlreadyRunning)
{
    // failsafe for cli runs
    if(NeroFS::GetUmU().isEmpty()) return -1;
    hashVal = hash;
    auto pathSetting = NeroSetting::init(path, *this);
    QString shortcutPath = pathSetting.shortcutSetting;
    QFileInfo fileToRun(pathSetting.shortcutSetting);

    if(pathSetting.SettingValue().startsWith("C:/") && !fileToRun.exists()) {
        return -1;
    }

    QProcess runner;

    // TODO: this is ass for prerun scripts that should be running persistently.
    QString prerun = shortcuts + prerunScript;
    if(HasSetting(prerun)) {
        runner.start(settings->value(prerun).toString(), (QStringList){});

        while(runner.state() != QProcess::NotRunning) {
            runner.waitForReadyRead(-1);
            printf("%s", runner.readAll().constData());
        }

        printf("%s", runner.readAll().constData());
    }

    runner.setProcessChannelMode(QProcess::ForwardedOutputChannel);
    runner.setReadChannel(QProcess::StandardError);

    env = QProcessEnvironment::systemEnvironment();
    QString prefixPath = NeroFS::GetPrefixesPath()->path()+'/'+NeroFS::GetCurrentPrefix();
    env.insert(wineprefix, prefixPath);

    // Only explicit set GAMEID when not already declared by user
    // See SeongGino/Nero-umu#66 for more info
    if(!env.contains(gameId)) {
        env.insert(gameId, FALSE);
    }

    QString protonRunner = settings->value(currentRunner).toString();
    QString runnerPath = NeroFS::GetProtonsPath()->path()+'/'+protonRunner;
    if(!QFile::exists(runnerPath)) {
        printf("Could not find %s in '%s', ", protonRunner.toLocal8Bit().constData(), NeroFS::GetProtonsPath()->absolutePath().toLocal8Bit().constData());
        if(!NeroFS::GetAvailableProtons()->isEmpty()) {
            protonRunner = NeroFS::GetAvailableProtons()->first();
        }
        printf("using %s instead\n", protonRunner.toLocal8Bit().constData());
    }
    env.insert(protonPath, runnerPath);
    bool isRuntimeUpdate = settings->value(runtimeUpdate).toBool();
    if(isRuntimeUpdate && !env.contains(umuRuntimeUpdate)){
        env.insert(umuRuntimeUpdate, TRUE);
    }

    prefixAlreadyRunning
        ? env.insert(ProtonSettings::verb, run)
        : env.insert(ProtonSettings::verb, waitForExitRun);

    // WAS added here to unrotate Switch controllers,
    // but may not actually be necessary on newer versions based on SDL3? iunno
    if(!env.contains(sdlUseButtonLabels)) {
        env.insert(sdlUseButtonLabels, FALSE);
    }

    InitCache();

    // unfortunately, env insert does NOT allow settings bools properly as-is,
    // so all booleans have to be converted to an int string.

    //if(!settings->value("Shortcuts--"+hash+"/CustomEnvVars").toString().isEmpty()) {
    //   qDebug() << settings->value("Shortcuts--"+hash+"/CustomEnvVars").toStringList();
    //}
    NeroSetting dllOverride = NeroSetting::init(dllOverrideStr, *this);
    if(dllOverride.hasShortcutSetting()) {
        NeroSetting ignoreGlobalDll = NeroSetting::init(ignoreGlobalDlls, *this);
        QStringList dllShortcutOverrides = dllOverride.GetSetingList() << env.value(wineDllOverrides);
        if(ignoreGlobalDll.HasSetting() && !dllOverride.HasSetting()) {
            // if overrides are duplicated, last overrides take priority over first overrides
            QStringList prefixArgs = dllOverride.GetSetingList();
            dllShortcutOverrides = prefixArgs << dllShortcutOverrides;
        }
        env.insert(ProtonSettings::wineDllOverrides, dllShortcutOverrides.join(';')+";");
    }

    // D8VK is dependent on DXVK's existence, so forcing WineD3D overrides D8VK.
    NeroSetting forceWine = NeroSetting::init(forceWineD3D, *this);
    NeroSetting d8vk = NeroSetting::init(noD8VK, *this);
    if(forceWine.HasSetting() || !d8vk.HasSetting()) {
        env.insert(ProtonSettings::useWineD3D, forceWine.Setting());
    }
    else if((d8vk.HasSetting())) {
        env.insert(protonDxvkD3D8, TRUE);
    }

    NeroSetting nvApi = NeroSetting::init(enableNvApi, *this);
    // For what it's worth, there's also _DISABLE_NVAPI, but not sure if that's more/less useful.
    if(nvApi.HasSetting()) {
        env.insert(ProtonSettings::forceNvapi, TRUE);
    }
    // TODO: do this better
    addProperty(limitGlExtensions, protonOldGl);
    addProperty(vkCapture, obsVkCapture);
    addProperty(forceIGpu, forceDefaultDevice);
    NeroSetting limitFrames = NeroSetting::init(limitFps, *this);
    QString fpsShortcut = shortcuts + limitFps;
    int fpsLimit = limitFrame.SettingValue().toInt();
    if(fpsLimit) {
        env.insert(ProtonSettings::dxvkFrameRate, QString::number(fpsLimit));
    }
    NeroSetting fileSync(fileSyncMode, *this);
    // TODO: Probably make this a method
    int syncType = fileSync.hasShortcutSetting()
                    ? settings->value(fileSync.shortcutSetting).toInt()
                    : settings->value(fileSync.prefixSetting).toInt();
    switch(syncType) {
        // ntsync SHOULD be better in all scenarios compared to other sync options, but requires kernel 6.14+ and GE-Proton10-9+
        // For older Protons, they should be safely ignoring this and fallback to fsync anyways.
        // Newer protons than GE10-9 should enable this automatically from its end, and doesn't require WOW64
        // (and currently, WOW64 seems problematic for some fringe cases, like TeknoParrot's BudgieLoader not spawning a window)
        case NeroConstant::NTsync:
            if(protonRunner == ge109) {
                env.insert(ProtonSettings::useNtSync, TRUE);
                env.insert(ProtonSettings::useWow64, TRUE);
            }
            break;
        case NeroConstant::Fsync:
            env.insert(ProtonSettings::noNtSync, TRUE);
            break;
        case NeroConstant::NoSync:
            env.insert(ProtonSettings::noEsync, TRUE);
        case NeroConstant::Esync:
            env.insert(ProtonSettings::noNtSync, TRUE);
            env.insert(ProtonSettings::noFsync, TRUE);
            break;
        default: 
            break;
    }
    NeroSetting debug(debugOutput, *this);
    if(debug.HasSetting()) {
        InitDebugProperties(debug.shortcutSetting);
        InitDebugProperties(debug.prefixSetting);
    }
    // TODO: ideally, we should set this as a colon-separated list of whitelisted "0xVID/0xPID" pairs
    //       but I guess this'll do for now.
    NeroSetting hiDraw = NeroSetting::init(allowHidraw, *this);
    hiDraw.HasSetting() && hiDraw.GetSetting().toBool()
            ? env.insert(protonHiDraw, TRUE) 
            : env.insert(ProtonSettings::preferSdl, TRUE);

    NeroSetting xalia(useXalia, *this);
    xalia.HasSetting() && xalia.GetValue().toBool()
            ? env.insert(ProtonSettings::useXalia, TRUE) 
            : env.insert(ProtonSettings::useXalia, FALSE);

    NeroSetting wayland(useWayland, *this);
    bool isWayland = env.contains(wayland.shortcutSetting)
            ? !env.value(wayland.shortcutSetting).isEmpty()
            : false;
    if (isWayland && wayland.HasSetting()) {
        env.insert(protonEnableWayland, TRUE);
        NeroSetting hdr(useHdr, *this);
        if (hdr.HasSetting()) {
            env.insert(protonUseHdr, TRUE);
        }
    }
    // TODO: Iterate through NeroConfig properties and loop on each. 
    QStringList arguments = {NeroFS::GetUmU(), GetProperty(shortcutPath)};
    // some arguments are parsed as stringlists and others as string, so check which first.
    QString argsStr = "Args";
    NeroSetting args(argsStr, *this);

    // TODO: Add get for explicitly list to replicate this functionality
    QVariant argsVar =  settings->value(args.shortcutSetting);

    if (argsVar.canConvert<QStringList>() && !argsVar.toStringList().isEmpty()) {
        arguments.append(argsVar.toStringList());
    } else if (argsVar.canConvert<QString>() && !argsVar.toString().isEmpty()) {
        // SUPER UNGA BUNGA: manually split string into a list
        QString buf = settings->value("Shortcuts--"+hash+"/Args").toString();
        QStringList args;
        args.append("");
        bool quotation = false;
        for(const auto &chara : std::as_const(buf)) {
            if(!quotation) {
                if(chara != ' ' && chara != '"') args.last().append(chara);
                else switch(chara.unicode()) {
                    case '"': quotation = true;
                    case ' ': if(!args.last().isEmpty()) args.append(""); break;
                    default: break;
                }
            } else if(chara != '"') args.last().append(chara);
            else {
                quotation = false;
                args.append("");
            }
        }
        if(args.last().isEmpty()) args.removeLast();
        arguments.append(args);
    }

    NeroSetting game(gamemode, *this);

    if(game.HasSetting()) {
        arguments.prepend(gamemoderun);
    }

    NeroSetting scale(scalingMode, *this);
    int scalingMode = scale.GetSetting().toInt();
    
    switch(scalingMode) {
    // TODO: redo like all of this
    case NeroConstant::ScalingNormal:
        break;
    case NeroConstant::ScalingIntegerScale:
        env.insert(fsrScaling, TRUE);
        env.insert("WINE_FULLSCREEN_INTEGER_SCALING", TRUE); 
        break;
    case NeroConstant::ScalingFSRperformance:
        env.insert(fsrScaling, TRUE);
        env.insert("WINE_FULLSCREEN_FSR_STRENGTH", "0");
        break;
    case NeroConstant::ScalingFSRbalanced:
        env.insert(fsrScaling, TRUE);
        env.insert("WINE_FULLSCREEN_FSR_STRENGTH", "1");
        break;
    case NeroConstant::ScalingFSRquality:
        env.insert(fsrScaling, TRUE);
        env.insert("WINE_FULLSCREEN_FSR_STRENGTH", "2");
        break;
    case NeroConstant::ScalingFSRhighquality:
        env.insert(fsrScaling, TRUE);
        env.insert("WINE_FULLSCREEN_FSR_STRENGTH", "3");
        break;
    case NeroConstant::ScalingFSRhigherquality:
        env.insert(fsrScaling, TRUE);
        env.insert("WINE_FULLSCREEN_FSR_STRENGTH", "4");
        break;
    case NeroConstant::ScalingFSRhighestquality:
        env.insert(fsrScaling, TRUE);
        env.insert("WINE_FULLSCREEN_FSR_STRENGTH", "5");
        break;
    case NeroConstant::ScalingFSRcustom:
        env.insert(fsrScaling, TRUE);
        env.insert(fsrCustom,
                    settings->value("Shortcuts--"+hash+"/FSRcustomResW").toString()+'x'+
                    settings->value("Shortcuts--"+hash+"/FSRcustomResH").toString()
        );
        break;
    case NeroConstant::ScalingGamescopeFullscreen:

        arguments.prepend("--");
        arguments.prepend(fullscreenArg);
        if(settings->value("Shortcuts--"+hash+"/GamescopeOutResH").toInt()) {
            arguments.prepend(settings->value("Shortcuts--"+hash+"/GamescopeOutResH").toString());
            arguments.prepend(heightArg);
        }
        if(settings->value("Shortcuts--"+hash+"/GamescopeOutResW").toInt()) {
            arguments.prepend(settings->value("Shortcuts--"+hash+"/GamescopeOutResW").toString());
            arguments.prepend(widthArg);
        }
        if(settings->value("Shortcuts--"+hash+"/GamescopeFilter").toInt()) {
            switch(settings->value("Shortcuts--"+hash+"/GamescopeFilter").toInt()) {
            case NeroConstant::GSfilterNearest:
                arguments.prepend("nearest"); break;
            case NeroConstant::GSfilterFSR:
                arguments.prepend("fsr"); break;
            case NeroConstant::GSfilterNLS:
                arguments.prepend("nis"); break;
            case NeroConstant::GSfilterPixel:
                arguments.prepend("pixel"); break;
            }
            arguments.prepend("-F");
        }
        if(settings->value("Shortcuts--"+hash+"/LimitFPS").toInt()) {
            arguments.prepend(QByteArray::number(settings->value("Shortcuts--"+hash+"/LimitFPS").toInt()));
            arguments.prepend("-r");
            arguments.prepend(QByteArray::number(settings->value("Shortcuts--"+hash+"/LimitFPS").toInt()));
            arguments.prepend("-o");
        }
        //arguments.prepend("--adaptive-sync");
        arguments.prepend("gamescope");
        break;
    case NeroConstant::ScalingGamescopeBorderless:
        arguments.prepend("--");
        arguments.prepend("-b");
    case NeroConstant::ScalingGamescopeWindowed:
        if(!arguments.contains("--")) arguments.prepend("--");
        if(settings->value("Shortcuts--"+hash+"/GamescopeWinResH").toInt()) {
            arguments.prepend(settings->value("Shortcuts--"+hash+"/GamescopeWinResH").toString());
            arguments.prepend("-H");
        }
        if(settings->value("Shortcuts--"+hash+"/GamescopeWinResW").toInt()) {
            arguments.prepend(settings->value("Shortcuts--"+hash+"/GamescopeWinResW").toString());
            arguments.prepend("-W");
        }
        if(settings->value("Shortcuts--"+hash+"/GamescopeOutResH").toInt()) {
            arguments.prepend(settings->value("Shortcuts--"+hash+"/GamescopeOutResH").toString());
            arguments.prepend("-h");
        }
        if(settings->value("Shortcuts--"+hash+"/GamescopeOutResW").toInt()) {
            arguments.prepend(settings->value("Shortcuts--"+hash+"/GamescopeOutResW").toString());
            arguments.prepend("-w");
        }
        if(settings->value("Shortcuts--"+hash+"/GamescopeFilter").toInt()) {
            switch(settings->value("Shortcuts--"+hash+"/GamescopeFilter").toInt()) {
            case NeroConstant::GSfilterNearest:
                arguments.prepend("nearest"); break;
            case NeroConstant::GSfilterFSR:
                arguments.prepend("fsr"); break;
            case NeroConstant::GSfilterNLS:
                arguments.prepend("nis"); break;
            case NeroConstant::GSfilterPixel:
                arguments.prepend("pixel"); break;
            }
            arguments.prepend("-F");
        }
        if(settings->value("Shortcuts--"+hash+"/LimitFPS").toInt()) {
            arguments.prepend(QByteArray::number(settings->value("Shortcuts--"+hash+"/LimitFPS").toInt()));
            arguments.prepend("-r");
            arguments.prepend(QByteArray::number(settings->value("Shortcuts--"+hash+"/LimitFPS").toInt()));
            arguments.prepend("-o");
        }
        //arguments.prepend("--adaptive-sync");
        arguments.prepend(gamescope);
        break;
    }

    NeroSetting mangohud(mangohudStr, *this);
    bool mangohudSetting = mangohud.HasSetting();
    if(mangohud.HasSetting() && mangohud.GetValue().toBool()) {
        if(arguments.contains(gamescope)) {
            arguments.insert(1, mangoappArg);
        } else if (!env.contains(mangohudStr.toUpper())) {
            arguments.prepend(mangohudStr.toLower());
        }
    }

    runner.setProcessEnvironment(env);
    // some apps requires working directory to be in the right location
    // (corrected if path starts with Windows drive letter prefix)
    QString pathValue = pathSetting.GetSetting();
    // settings->value("Shortcuts--"+hash+"/Path").toString();
    QString cPath = NeroFS::GetPrefixesPath()->path()+'/'+NeroFS::GetCurrentPrefix()+"/drive_c/";
    QString pathDir = pathSetting.GetSetting();
    QString workingDir = pathDir.left(pathDir.lastIndexOf("/")).replace("C:", cPath );
    runner.setWorkingDirectory(workingDir);
    QString command = arguments.takeFirst();

    QString logDir = NeroFS::GetPrefixesPath()->path()+'/'+NeroFS::GetCurrentPrefix();

    QDir logsDir(logDir);
    if(!logsDir.exists(".logs"))
        logsDir.mkdir(".logs");
    logsDir.cd(".logs");
    QString nameStr = "/Name";
    NeroSetting name(nameStr, *this);
    QFile log(logsDir.path() + '/' + name.shortcutSetting + '-'+ hash + ".txt");
    if(loggingEnabled) {
        log.open(QIODevice::WriteOnly);
        log.resize(0);
        log.write("Current running environment:\n");
        log.write(runner.environment().join('\n').toLocal8Bit());
        log.write("\n\nRunning command:\n" + command.toLocal8Bit() + ' ' + arguments.join(' ').toLocal8Bit() + '\n');
        log.write("==============================================\n");
    }
    QString launch = "command: " + command + " arguments: " + arguments.join(' ');
    printf("%s", launch);
    runner.start(command, arguments);
    runner.waitForStarted(-1);

    WaitLoop(runner, log);

    // in case settings changed from manager
    settings = NeroFS::GetCurrentPrefixCfg();

    NeroSetting postrunScript (postRunScriptStr, *this);
    if(postrunScript.hasShortcutSetting()) {
        runner.start(postrunScript.GetSetting(), (QStringList){});

        while(runner.state() != QProcess::NotRunning) {
            runner.waitForReadyRead(-1);
            printf("%s", runner.readAll().constData());
        }

        printf("%s", runner.readAll().constData());
    }

    return runner.exitCode();
}

int NeroRunner::StartOnetime(const QString &path, const bool &prefixAlreadyRunning, const QStringList &args)
{
    // failsafe for cli runs
    if(NeroFS::GetUmU().isEmpty()) return -1;

    QSettings *settings = NeroFS::GetCurrentPrefixCfg();

    QProcess runner;
    // umu seems to direct both umu-run frontend and Proton output to stderr,
    // meaning stdout is virtually unused.
    runner.setProcessChannelMode(QProcess::ForwardedOutputChannel);
    runner.setReadChannel(QProcess::StandardError);

    env = QProcessEnvironment::systemEnvironment();
    env.insert("WINEPREFIX", NeroFS::GetPrefixesPath()->path()+'/'+NeroFS::GetCurrentPrefix());

    // Only explicit set GAMEID when not already declared by user
    // See SeongGino/Nero-umu#66 for more info
    if(!env.contains("GAMEID")) env.insert("GAMEID", "0");

    QString protonRunner = settings->value("PrefixSettings/CurrentRunner").toString();
    if(!QFile::exists(NeroFS::GetProtonsPath()->path()+'/'+protonRunner)) {
        printf("Could not find %s in '%s', ", protonRunner.toLocal8Bit().constData(), NeroFS::GetProtonsPath()->absolutePath().toLocal8Bit().constData());
        if(!NeroFS::GetAvailableProtons()->isEmpty()) protonRunner = NeroFS::GetAvailableProtons()->first();
        printf("using %s instead\n", protonRunner.toLocal8Bit().constData());
    }
    env.insert("PROTONPATH", NeroFS::GetProtonsPath()->path()+'/'+protonRunner);

    if(prefixAlreadyRunning)
        env.insert("PROTON_VERB", "run");
    else env.insert("PROTON_VERB", "waitforexitandrun");

    if(!env.contains("SDL_GAMECONTROLLER_USE_BUTTON_LABELS"))
        env.insert("SDL_GAMECONTROLLER_USE_BUTTON_LABELS", "0");

    InitCache();

    if(settings->value("PrefixSettings/RuntimeUpdateOnLaunch").toBool())
        env.insert("UMU_RUNTIME_UPDATE", "1");
    if(!settings->value("PrefixSettings/DLLoverrides").toStringList().isEmpty())
        env.insert("WINEDLLOVERRIDES", settings->value("PrefixSettings/DLLoverrides").toStringList().join(';')+';'+env.value("WINEDLLOVERRIDES"));
    if(settings->value("PrefixSettings/ForceWineD3D").toBool())
        env.insert("PROTON_USE_WINED3D", "1");
    else if(!settings->value("PrefixSettings/NoD8VK").toBool())
        env.insert("PROTON_DXVK_D3D8", "1");
    if(settings->value("PrefixSettings/EnableNVAPI").toBool())
        env.insert("PROTON_FORCE_NVAPI", "1");
    if(settings->value("PrefixSettings/LimitGLextensions").toBool())
        env.insert("PROTON_OLD_GL_STRING", "1");
    if(settings->value("PrefixSettings/VKcapture").toBool())
        env.insert("OBS_VKCAPTURE", "1");
    if(settings->value("PrefixSettings/ForceiGPU").toBool())
        env.insert("MESA_VK_DEVICE_SELECT_FORCE_DEFAULT_DEVICE", "1");

    switch(settings->value("PrefixSettings/FileSyncMode").toInt()) {
    // ntsync SHOULD be better in all scenarios compared to other sync options, but requires kernel 6.14+ and GE-Proton10-9+
    // For older Protons, they should be safely ignoring this and fallback to fsync anyways.
    // Newer protons than GE10-9 should enable this automatically from its end, and doesn't require WOW64
    // (and currently, WOW64 seems problematic for some fringe cases, like TeknoParrot's BudgieLoader not spawning a window)
    case NeroConstant::NTsync:
        if(protonRunner == "GE-Proton10-9") {
            env.insert("PROTON_USE_NTSYNC", "1");
            env.insert("PROTON_USE_WOW64", "1");
        }
        break;
    case NeroConstant::Fsync:
        env.insert("PROTON_NO_NTSYNC", "1");
        break;
    case NeroConstant::NoSync:
        env.insert("PROTON_NO_ESYNC", "1");
    case NeroConstant::Esync:
        env.insert("PROTON_NO_NTSYNC", "1");
        env.insert("PROTON_NO_FSYNC", "1");
        break;
    default: break;
    }

    switch(settings->value("PrefixSettings/DebugOutput").toInt()) {
    case NeroConstant::DebugDisabled:
        break;
    case NeroConstant::DebugFull:
        loggingEnabled = true;
        env.insert("WINEDEBUG", "+loaddll,debugstr,mscoree,seh");
        break;
    case NeroConstant::DebugLoadDLL:
        loggingEnabled = true;
        env.insert("WINEDEBUG", "+loaddll");
        break;
    }

    if(settings->value("PrefixSettings/AllowHidraw").toBool()) {
        if(!env.contains("PROTON_ENABLE_HIDRAW")) env.insert("PROTON_ENABLE_HIDRAW", "1");
    // Forces controllers (that otherwise get preferred by hidraw by default) to go through SDL backend instead
    } else env.insert("PROTON_PREFER_SDL", "1");

    if(settings->value("PrefixSettings/UseXalia").toBool()) {
        if(!env.contains("PROTON_USE_XALIA")) env.insert("PROTON_USE_XALIA", "1");
    } else if(!env.contains("PROTON_USE_XALIA")) env.insert("PROTON_USE_XALIA", "0");

    if(env.contains("WAYLAND_DISPLAY") && !env.value("WAYLAND_DISPLAY").isEmpty()) {
        if(settings->value("PrefixSettings/UseWayland").toBool()) {
            env.insert("PROTON_ENABLE_WAYLAND", "1");
            if(settings->value("PrefixSettings/UseHDR").toBool())
                env.insert("PROTON_ENABLE_HDR", "1");
        }
    }

    QStringList arguments;
    arguments.append(NeroFS::GetUmU());

    // Proton/umu should be able to translate Windows-type paths on its own, no conversion needed
    arguments.append(path);

    if(!args.isEmpty())
        arguments.append(args);

    if(settings->value("PrefixSettings/Gamemode").toBool())
        arguments.prepend("gamemoderun");

    switch(settings->value("PrefixSettings/ScalingMode").toInt()) {
    case NeroConstant::ScalingIntegerScale:
        env.insert("WINE_FULLSCREEN_INTEGER_SCALING", "1");
        break;
    case NeroConstant::ScalingFSRperformance:
        env.insert("WINE_FULLSCREEN_FSR", "1");
        env.insert("WINE_FULLSCREEN_FSR_STRENGTH", "0");
        break;
    case NeroConstant::ScalingFSRbalanced:
        env.insert("WINE_FULLSCREEN_FSR", "1");
        env.insert("WINE_FULLSCREEN_FSR_STRENGTH", "1");
        break;
    case NeroConstant::ScalingFSRquality:
        env.insert("WINE_FULLSCREEN_FSR", "1");
        env.insert("WINE_FULLSCREEN_FSR_STRENGTH", "2");
        break;
    case NeroConstant::ScalingFSRhighquality:
        env.insert("WINE_FULLSCREEN_FSR", "1");
        env.insert("WINE_FULLSCREEN_FSR_STRENGTH", "3");
        break;
    case NeroConstant::ScalingFSRhigherquality:
        env.insert("WINE_FULLSCREEN_FSR", "1");
        env.insert("WINE_FULLSCREEN_FSR_STRENGTH", "4");
        break;
    case NeroConstant::ScalingFSRhighestquality:
        env.insert("WINE_FULLSCREEN_FSR", "1");
        env.insert("WINE_FULLSCREEN_FSR_STRENGTH", "5");
        break;
    case NeroConstant::ScalingFSRcustom:
        env.insert("WINE_FULLSCREEN_FSR", "1");
        env.insert("WINE_FULLSCREEN_FSR_CUSTOM_MODE", QString("%1x%2").arg(settings->value("PrefixSettings/FSRcustomResW").toString(),
                                                                           settings->value("PrefixSettings/FSRcustomResH").toString()));
        break;
    case NeroConstant::ScalingGamescopeFullscreen:
        arguments.prepend("--");
        arguments.prepend("-f");
        if(settings->value("PrefixSettings/GamescopeOutResH").toInt()) {
            arguments.prepend(settings->value("PrefixSettings/GamescopeOutResH").toString());
            arguments.prepend("-h");
        }
        if(settings->value("PrefixSettings/GamescopeOutResW").toInt()) {
            arguments.prepend(settings->value("PrefixSettings/GamescopeOutResW").toString());
            arguments.prepend("-w");
        }
        if(settings->value("PrefixSettings/GamescopeFilter").toInt()) {
            switch(settings->value("PrefixSettings/GamescopeFilter").toInt()) {
            case NeroConstant::GSfilterNearest:
                arguments.prepend("nearest"); break;
            case NeroConstant::GSfilterFSR:
                arguments.prepend("fsr"); break;
            case NeroConstant::GSfilterNLS:
                arguments.prepend("nis"); break;
            case NeroConstant::GSfilterPixel:
                arguments.prepend("pixel"); break;
            }
            arguments.prepend("-F");
        }
        if(settings->value("PrefixSettings/LimitFPS").toInt()) {
            arguments.prepend(QByteArray::number(settings->value("PrefixSettings/LimitFPS").toInt()));
            arguments.prepend("--framerate-limit");
        }
        arguments.prepend("--adaptive-sync");
        arguments.prepend("gamescope");
        break;
    case NeroConstant::ScalingGamescopeBorderless:
        arguments.prepend("--");
        arguments.prepend("-b");
    case NeroConstant::ScalingGamescopeWindowed:
        if(!arguments.contains("--")) arguments.prepend("--");
        if(settings->value("PrefixSettings/GamescopeWinResH").toInt()) {
            arguments.prepend(settings->value("PrefixSettings/GamescopeWinResH").toString());
            arguments.prepend("-H");
        }
        if(settings->value("PrefixSettings/GamescopeWinResW").toInt()) {
            arguments.prepend(settings->value("PrefixSettings/GamescopeWinResW").toString());
            arguments.prepend("-W");
        }
        if(settings->value("PrefixSettings/GamescopeOutResH").toInt()) {
            arguments.prepend(settings->value("PrefixSettings/GamescopeOutResH").toString());
            arguments.prepend("-h");
        }
        if(settings->value("PrefixSettings/GamescopeOutResW").toInt()) {
            arguments.prepend(settings->value("PrefixSettings/GamescopeOutResW").toString());
            arguments.prepend("-w");
        }
        if(settings->value("PrefixSettings/GamescopeFilter").toInt()) {
            switch(settings->value("PrefixSettings/GamescopeFilter").toInt()) {
            case NeroConstant::GSfilterNearest:
                arguments.prepend("nearest"); break;
            case NeroConstant::GSfilterFSR:
                arguments.prepend("fsr"); break;
            case NeroConstant::GSfilterNLS:
                arguments.prepend("nis"); break;
            case NeroConstant::GSfilterPixel:
                arguments.prepend("pixel"); break;
            }
            arguments.prepend("-F");
        }
        if(settings->value("PrefixSettings/LimitFPS").toInt()) {
            arguments.prepend(QByteArray::number(settings->value("PrefixSettings/LimitFPS").toInt()));
            arguments.prepend("--framerate-limit");
        }
        arguments.prepend("--adaptive-sync");
        arguments.prepend("gamescope");
        break;
    }

    if(settings->value("PrefixSettings/Mangohud").toBool()) {
        if(arguments.contains("gamescope"))
            arguments.insert(1, "--mangoapp");
        else if(!env.contains("MANGOHUD")) arguments.prepend("mangohud");
    }
    runner.setProcessEnvironment(env);
    if(path.startsWith('/') || path.startsWith("~/") || path.startsWith("./"))
        runner.setWorkingDirectory(path.left(path.lastIndexOf("/")).replace("C:", NeroFS::GetPrefixesPath()->canonicalPath()+'/'+NeroFS::GetCurrentPrefix()+"/drive_c/"));

    QString command = arguments.takeFirst();

    QDir logsDir(NeroFS::GetPrefixesPath()->path()+'/'+NeroFS::GetCurrentPrefix());
    if(!logsDir.exists(".logs"))
        logsDir.mkdir(".logs");
    logsDir.cd(".logs");

    QFile log(logsDir.path()+'/'+path.mid(path.lastIndexOf('/')+1)+".txt");
    if(loggingEnabled) {
        log.open(QIODevice::WriteOnly);
        log.resize(0);
        log.write("Current running environment:\n");
        log.write(runner.environment().join('\n').toLocal8Bit());
        log.write("\n\nRunning command:\n" + command.toLocal8Bit() + ' ' + arguments.join(' ').toLocal8Bit() + '\n');
        log.write("==============================================\n");
    }
    printf("command: %s, arguments:%s", command, arguments.join(' '));
    runner.start(command, arguments);
    runner.waitForStarted(-1);

    WaitLoop(runner, log);

    return runner.exitCode();
}

void NeroRunner::WaitLoop(QProcess &runner, QFile &log)
{
    QByteArray stdout;

    while(runner.state() != QProcess::NotRunning) {
        if(!halt) {
            runner.waitForReadyRead(1000);
            if(runner.canReadLine()) {
                stdout = runner.readLine();
                printf("%s", stdout.constData());
                if(loggingEnabled)
                    log.write(stdout);

                if(stdout.contains("umu-launcher"))
                    emit StatusUpdate(NeroRunner::RunnerStarting);
                else if(stdout.contains("steamrt3 is up to date"))
                    emit StatusUpdate(NeroRunner::RunnerUpdated);
                else if(stdout.startsWith("Proton: Executable") || stdout.contains("SteamAPI_Init"))
                    emit StatusUpdate(NeroRunner::RunnerProtonStarted);
            }
        } else {
            emit StatusUpdate(NeroRunner::RunnerProtonStopping);
            StopProcess();
            emit StatusUpdate(NeroRunner::RunnerProtonStopped);
            break;
        }
    }

    while(!runner.atEnd()) {
        stdout = runner.readLine();
        printf("%s", stdout.constData());
        if(loggingEnabled)
            log.write(stdout);
    }

    if(loggingEnabled)
        log.close();
}

void NeroRunner::InitDebugProperties(QString property) {
    int value = settings->value(property).toInt();
    switch (value) {
        case NeroConstant::DebugDisabled:
            break;
        case NeroConstant::DebugFull:
            loggingEnabled = true;
            env.insert("WINEDEBUG", "+loaddll,debugstr,mscoree,seh");
            break;
        case NeroConstant::DebugLoadDLL:
            loggingEnabled = true;
            env.insert("WINEDEBUG", "+loaddll");
            break;
    }
}

void NeroRunner::StopProcess()
{
    // TODO: there's almost certainly a not-shitty way of stopping spawned Wine processes
    //       currently, the Kingdom Hearts Re-Fined patches will consistently lock up the prefix and remains resident
    //       even after this shutdown op has supposedly completed.
    QApplication::processEvents();
    QProcess wineStopper;
    env.insert("UMU_NO_PROTON", "1");
    env.remove("UMU_RUNTIME_UPDATE");
    env.insert("UMU_RUNTIME_UPDATE", "0");
    wineStopper.setProcessEnvironment(env);
    wineStopper.start(NeroFS::GetUmU(), { NeroFS::GetProtonsPath()->path()+'/'+NeroFS::GetCurrentRunner()+'/'+"proton", "runinprefix", "wineboot", "-e" });
    wineStopper.waitForFinished();
}



void NeroRunner::InitCache()
{
    QDir cachePath(NeroFS::GetPrefixesPath()->path()+'/'+NeroFS::GetCurrentPrefix());
    if(!cachePath.exists(".shaderCache")) {
        cachePath.mkdir(".shaderCache");
        if(!cachePath.cd(".shaderCache")) printf("Prefix cache directory not available! Not using cache...");
        else {
            env.insert("DXVK_STATE_CACHE_PATH", cachePath.absolutePath());
            env.insert("VKD3D_SHADER_CACHE_PATH", cachePath.absolutePath());
        }
    }
}
