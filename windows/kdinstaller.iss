; Inno Setup 6 or later is required for this script to work.

[Setup]
AppID=Koord-RealTime
AppName=Koord-RealTime
AppVerName=Koord-RealTime_3.8.0-k01
AppVersion=3.8.0-k01
AppPublisher=Koord.Live
AppPublisherURL=https://koord.live
AppSupportURL=https://github.com/koord-live/koord-realtime/issues
AppUpdatesURL=https://github.com/koord-live/koord-realtime/releases
AppContact=contact@koord.live
WizardStyle=modern

DefaultDirName={autopf}\Koord-RealTime
AppendDefaultDirName=no
ArchitecturesInstallIn64BitMode=x64

; for 100% dpi setting should be 164x314 - https://jrsoftware.org/ishelp/
WizardImageFile=windows\koord-realtime.bmp
; for 100% dpi setting should be 55x55 
WizardSmallImageFile=windows\koord-realtime-small.bmp

[Files]
; install everything else in deploy dir, including portaudio.dll, kdasioconfig.exe and all Qt dll deps
Source:"deploy\x86_64\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs 64bit; Check: Is64BitInstallMode

[Run]
Filename: "{app}\Koord-RealTime.exe"; Description: "Launch Koord-RealTime"; Flags: postinstall nowait skipifsilent unchecked
