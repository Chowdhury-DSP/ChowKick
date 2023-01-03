#define MyAppPublisher "Chowdhury DSP"
#define MyAppURL "https://chowdsp.com"
#define MyAppName "ChowKick"
#define MyAppVersion "##APPVERSION##"

[Setup]
AppName="{#MyAppName} {#MyAppVersion}"
AppVersion={#MyAppVersion}
AppVerName={#MyAppName}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={commonpf32}\Chowdhury DSP\
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
DisableDirPage=yes
DisableWelcomePage=no
LicenseFile=../../LICENSE
OutputBaseFilename="{#MyAppName}-Win-32bit-{#MyAppVersion}"
OutputDir=.
SetupIconFile=chowkick.ico
UninstallDisplayIcon=chowkick.ico
UninstallFilesDir={commonpf32}\Chowdhury DSP\{#MyAppName}
Compression=lzma
SolidCompression=yes

[Types]
Name: "full"; Description: "Full installation"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: "VST3_32"; Description: "VST3 Plugin"; Types: full
Name: "VST_32"; Description: "VST Plugin"; Types: full
Name: "CLAP_32"; Description: "CLAP Plugin"; Types: full
Name: "Standalone"; Description: "Standalone Plugin"; Types: full
Name: Data; Description: "Data Files"; Types: full custom; Flags: fixed
; Name: "AAX"; Description: "AAX Plugin"; Types: full

[Files]
Source: "../../res/tuning_library/*"; DestDir: {commonappdata}\ChowKick\tuning_library; Components: Data; Flags: recursesubdirs; Excludes: "*.git,windows.wt,configuration.xml,paramdocumentation.xml";
Source: "../../bin/Win32/ChowKick.vst3"; Excludes: "*.clap"; DestDir: "{code:GetDir|VST3_32}"; Components: VST3_32; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "../../bin/Win32/ChowKick.dll"; Excludes: "*.vst3,*.clap"; DestDir: "{code:GetDir|VST_32}"; Components: VST_32; Flags: ignoreversion
Source: "../../bin/Win32/ChowKick.clap"; Excludes: "*.vst3"; DestDir: "{code:GetDir|CLAP_32}"; Components: CLAP_32; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "../../bin/Win32/ChowKick.exe"; Excludes: "*.vst3,*.clap"; DestDir: "{code:GetDir|Standalone}"; Components: Standalone; Flags: ignoreversion

[Icons]
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"

[Code]
var
  Vst3_32DirPage: TinputDirWizardPage;
  Vst_32DirPage: TinputDirWizardPage;
  StandaloneDirPage: TinputDirWizardPage;

procedure InitializeWizard;
begin
  Log('Initializing extra pages')
  //VST3 32-bit Dir Page
  Vst3_32DirPage := CreateInputDirPage(wpSelectComponents,
    'Select Install Location for VST3 32-bit', 'Where would you like to install the plugin?',
    'VST3 32-bit plugin will be installed in the following folder.'#13#10#13#10 +
    'To continue, click Next. If you would like to select a different folder, click Browse.',
    False, 'New Folder');

  Vst3_32DirPage.add('');
  Vst3_32DirPage.values[0] := ExpandConstant('{commoncf32}\VST3');

  //VST 32-bit Dir Page
  Vst_32DirPage := CreateInputDirPage(Vst3_32DirPage.ID,
    'Select Install Location for VST 32-bit', 'Where would you like to install the plugin?',
    'VST 32-bit plugin will be installed in the following folder.'#13#10#13#10 +
    'To continue, click Next. If you would like to select a different folder, click Browse.',
    False, 'New Folder');

  Vst_32DirPage.add('');
  Vst_32DirPage.values[0] := ExpandConstant('{commoncf32}\VST2');

  //Standalone Dir Page
  StandaloneDirPage := CreateInputDirPage(Vst_32DirPage.ID,
    'Select Install Location for Standalone', 'Where would you like to install the plugin?',
    'Standalone plugin will be installed in the following folder.'#13#10#13#10 +
    'To continue, click Next. If you would like to select a different folder, click Browse.',
    False, 'New Folder');
  StandaloneDirPage.add('');
  StandaloneDirPage.values[0] := ExpandConstant('{pf32}\Chowdhury DSP');
end;

function IsSelected(Param: String) : Boolean;
begin
  if not (Pos(Param, WizardSelectedComponents(False)) = 0) then // WizardSelectedComponents(False)) then
    Result := True
end;

function ShouldSkipPage(PageID: Integer): Boolean;
begin
  { Skip pages that shouldn't be shown }
  Result := False;

  if (PageID = Vst3_32DirPage.ID) then
  begin
      Result := True;
      Log('Selected 3: ' + WizardSelectedComponents(False));

      if IsSelected ('vst3_32') then
      begin
        Log('Not Skipping');
        Result := False;
      end
  end

  else if (PageID = Vst_32DirPage.ID) then
  begin
      Result := True;
      Log('Selected 5: ' + WizardSelectedComponents(False));

      if IsSelected ('vst_32') then
      begin
        Log('Not Skipping');
        Result := False;
      end
  end
  
  else if (PageID = StandaloneDirPage.ID) then
  begin
      Result := True;
      Log('Selected 6: ' + WizardSelectedComponents(False));
      if IsSelected ('standalone') then
      begin
        Log('Not Skipping');
        Result := False;
      end
  end
end;

function GetDir(Param: String) : String;
begin
  if (Param = 'VST3_32') then
    Result := Vst3_32DirPage.values[0]
  else if (Param = 'VST_32') then
    Result := Vst_32DirPage.values[0]
  else if (Param = 'CLAP_32') then
    Result := ExpandConstant('{commoncf32}\CLAP')
  else if (Param = 'Standalone') then
    Result := StandaloneDirPage.values[0]
end;

function UpdateReadyMemo(Space, NewLine, MemoUserInfoInfo, MemoDirInfo, MemoTypeInfo,
  MemoComponentsInfo, MemoGroupInfo, MemoTasksInfo: String): String;
var
  S: String;
begin
  { Fill the 'Ready Memo' with the normal settings and the custom settings }
  S := '';
  S := S + MemoTypeInfo + NewLine + NewLine;
  S := S + MemoComponentsInfo + NewLine + NewLine;
  S := S + 'Destination Location:' + NewLine;

  if IsSelected('vst3_32') then
    S := S + Space +  GetDir('VST3_32') + ' (VST3 32-bit)' + NewLine;

  if IsSelected('vst_32') then
    S := S + Space +  GetDir('VST_32') + ' (VST 32-bit)' + NewLine;

  if IsSelected('clap_32') then
    S := S + Space +  GetDir('CLAP_32') + ' (CLAP 32-bit)' + NewLine;

  if IsSelected('standalone') then
    S := S + Space +  GetDir('Standalone') + ' (Standalone)' + NewLine;

  Result := S;
end;
