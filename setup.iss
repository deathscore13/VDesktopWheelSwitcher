[Setup]
AppName=VDesktopWheelSwitcher
AppVerName=Virtual Desktop Wheel Switcher
AppVersion=1.0.2
VersionInfoVersion=1.0.2.0
AppPublisher=DeathScore13
AppPublisherURL=https://github.com/deathscore13
AppSupportURL=https://github.com/deathscore13/{#SetupSetting("AppName")}
AppUpdatesURL=https://github.com/deathscore13/{#SetupSetting("AppName")}/releases
AppId={{1D83DE75-7D3A-4E82-B3E3-D8AEC89BEA81}

VersionInfoCompany=EvilCryDie
VersionInfoOriginalFileName={#SetupSetting("AppName")}
VersionInfoDescription={#SetupSetting("AppVerName")}
VersionInfoProductName={#SetupSetting("AppVerName")}
VersionInfoProductTextVersion={#SetupSetting("AppVersion")}
VersionInfoProductVersion={#SetupSetting("AppVersion")}
VersionInfoTextVersion={#SetupSetting("AppVersion")}

PrivilegesRequired=admin
CloseApplications=yes 
DefaultDirName={autopf}\{#SetupSetting("AppName")}
DefaultGroupName={#SetupSetting("AppName")}
OutputDir={#SourcePath}\Setup
OutputBaseFilename={#SetupSetting("AppName")}

SetupIconFile=shell32_35.ico
UninstallDisplayIcon={app}\{#SetupSetting("SetupIconFile")}

[Files]
Source: "Release\{#SetupSetting("AppName")}.exe"; \
DestDir: "{app}"; \
Flags: ignoreversion

Source: "{#SetupSetting("SetupIconFile")}"; \
DestDir: "{app}"; \
Flags: ignoreversion

[Run]
Filename: "{sys}\schtasks.exe"; \
Parameters: "/create /tn ""{#SetupSetting("AppName")}"" /tr ""\""{app}\{#SetupSetting("AppName")}.exe\"""" /sc onlogon /rl highest /f"; \
Flags: runhidden

Filename: "{sys}\schtasks.exe"; \
Parameters: "/run /tn ""{#SetupSetting("AppName")}"""; \
Flags: runhidden

[UninstallRun]
Filename: "{sys}\schtasks.exe"; \
Parameters: "/delete /tn ""{#SetupSetting("AppName")}"" /f"; \
Flags: runhidden; \
RunOnceId: "RemoveTask"

Filename: "{sys}\taskkill.exe"; \
Parameters: "/f /im ""{#SetupSetting("AppName")}.exe"""; \
Flags: runhidden;  \
RunOnceId: "KillProcess"
