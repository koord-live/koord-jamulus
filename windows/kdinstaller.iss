; Inno Setup 6 or later is required for this script to work.

[Setup]
AppID=Koord-RT
AppName=Koord-RT
AppVerName=Koord
AppVersion=1.9.6
AppPublisher=Koord.Live
AppPublisherURL=https://koord.live
AppSupportURL=https://github.com/koord-live/koord-realtime/issues
AppUpdatesURL=https://github.com/koord-live/koord-realtime/releases
AppContact=contact@koord.live
WizardStyle=modern

DefaultDirName={autopf}\Koord-RT
AppendDefaultDirName=no
ArchitecturesInstallIn64BitMode=x64

; for 100% dpi setting should be 164x314 - https://jrsoftware.org/ishelp/
WizardImageFile=windows\koord-rt.bmp
; for 100% dpi setting should be 55x55 
WizardSmallImageFile=windows\koord-rt-small.bmp

[Files]
; install everything else in deploy dir, including portaudio.dll, kdasioconfig.exe and all Qt dll deps
Source:"deploy\x86_64\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs 64bit; Check: Is64BitInstallMode

[Icons]
Name: "{group}\Koord-RT"; Filename: "{app}\Koord-RT.exe"; WorkingDir: "{app}"
Name: "{group}\KoordASIO Config"; Filename: "{app}\kdasioconfig.exe"; WorkingDir: "{app}"

[Run]
Filename: "{app}\kdasioconfig.exe"; Description: "Run KoordASIO-builtin Config"; Flags: postinstall nowait skipifsilent
Filename: "{app}\Koord-RT.exe"; Description: "Launch Koord-RT"; Flags: postinstall nowait skipifsilent unchecked

; install reg key to locate builtin kdasioconfig at runtime
[Registry]
Root: HKLM64; Subkey: "Software\Koord"; Flags: uninsdeletekeyifempty
Root: HKLM64; Subkey: "Software\Koord\KoordASIOi"; Flags: uninsdeletekey
Root: HKLM64; Subkey: "Software\Koord\KoordASIOi\Install"; ValueType: string; ValueName: "InstallPath"; ValueData: "{app}"