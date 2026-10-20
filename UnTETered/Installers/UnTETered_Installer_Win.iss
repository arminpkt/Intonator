; =============================================================================
;  UnTETered — Windows installer (Inno Setup 6.3+)
;
;  One page: VST3 folder + optional "Install Ableton Extension" with its own path.
;  Everything project-specific lives in the #defines, so this file works as a
;  template for future plugins.
;
;  Expected location: <repo>\UnTETered\installer\UnTETered.iss
; =============================================================================

#define AppName        "UnTETered"
#define AppVersion     "0.0.1"
#define Publisher      "UnTETered"
; Generate once and NEVER change it — Windows uses it to recognise updates.
#define AppGUID        "{{8C671952-7699-45EB-B588-5EEBF2030F4A}"

; --- Build artefacts (relative to this .iss file) ----------------------------
#define BuildDir       "..\..\cmake-build-release\UnTETered\UnTETered_artefacts\Release"
#define VST3Bundle     BuildDir + "\VST3\UnTETered.vst3"

; --- Ableton Remote Script ----------------------------------------------------
#define RemoteScriptSrc    "..\scripts\ableton\UnTETered"
; Must match the folder name used in CMake (_remote_script_dest)
#define RemoteScriptFolder "UnTETered"


[Setup]
AppId={#AppGUID}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#Publisher}
; Only used for the uninstaller files — the real targets come from the custom page
DefaultDirName={autopf}\{#Publisher}\{#AppName}
OutputDir=Output
OutputBaseFilename={#AppName}-{#AppVersion}-Windows-Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
UsedUserAreasWarning=no
UninstallDisplayName={#AppName}
; Skip all standard pages -> our custom page is the first thing the user sees
DisableWelcomePage=yes
DisableDirPage=yes
DisableProgramGroupPage=yes
DisableReadyPage=yes


[Files]
Source: "{#VST3Bundle}\*"; DestDir: "{code:GetVST3Dir}\{#AppName}.vst3"; \
    Flags: ignoreversion recursesubdirs createallsubdirs

Source: "{#RemoteScriptSrc}\*.py"; DestDir: "{code:GetAbletonDir}\Remote Scripts\{#RemoteScriptFolder}"; \
    Check: InstallAbleton; Flags: ignoreversion


[UninstallDelete]
; Live writes __pycache__ into the script folder; without this the folder stays behind
Type: filesandordirs; Name: "{code:GetAbletonDir}\Remote Scripts\{#RemoteScriptFolder}"; Check: InstallAbleton


[Messages]
FinishedLabel=Setup has finished installing [name].%n%nIf you installed the Ableton extension: restart Live, then choose "UnTETered" as a Control Surface under Preferences → Link, Tempo & MIDI.


[Code]
var
  MainPage: TWizardPage;
  VST3Edit, AbletonEdit: TNewEdit;
  VST3Browse, AbletonBrowse: TNewButton;
  AbletonCheck: TNewCheckBox;
  AbletonLabel, AbletonHint: TNewStaticText;

// ---- show/hide the Ableton path depending on the checkbox --------------------
procedure UpdateAbletonVisibility;
begin
  AbletonLabel.Visible  := AbletonCheck.Checked;
  AbletonEdit.Visible   := AbletonCheck.Checked;
  AbletonBrowse.Visible := AbletonCheck.Checked;
  AbletonHint.Visible   := AbletonCheck.Checked;
end;

procedure AbletonCheckClick(Sender: TObject);
begin
  UpdateAbletonVisibility;
end;

// ---- browse buttons ----------------------------------------------------------
procedure VST3BrowseClick(Sender: TObject);
var
  Dir: String;
begin
  Dir := VST3Edit.Text;
  if BrowseForFolder('Select your VST3 plugin folder:', Dir, True) then
    VST3Edit.Text := Dir;
end;

procedure AbletonBrowseClick(Sender: TObject);
var
  Dir: String;
begin
  Dir := AbletonEdit.Text;
  if BrowseForFolder('Select your Ableton User Library folder:', Dir, True) then
    AbletonEdit.Text := Dir;
end;

// ---- original user's folders -------------------------------------------------
// Setup runs elevated. If the user approved UAC with a DIFFERENT admin account,
// {userdocs}/{userappdata} point to that admin's profile, not to the person who
// actually uses Live. So we ask Windows as the original (non-elevated) user.
procedure GetOriginalUserFolders(var Docs, AppData: String);
var
  TmpFile: String;
  ResultCode: Integer;
  Lines: TArrayOfString;
begin
  // Fallback: correct whenever the elevated account is the same user (the usual case)
  Docs    := ExpandConstant('{userdocs}');
  AppData := ExpandConstant('{userappdata}');

  // C:\ProgramData: both accounts can write/read there ({tmp} may belong to the admin)
  TmpFile := ExpandConstant('{commonappdata}\{#AppName}-setup-folders.txt');
  DeleteFile(TmpFile);

  if ExecAsOriginalUser('powershell.exe',
       '-NoProfile -NonInteractive -ExecutionPolicy Bypass -Command "' +
       '[Environment]::GetFolderPath(''MyDocuments''), [Environment]::GetFolderPath(''ApplicationData'') ' +
       '| Out-File -Encoding utf8 -FilePath ''' + TmpFile + '''"',
       '', SW_HIDE, ewWaitUntilTerminated, ResultCode)
     and (ResultCode = 0)
     and LoadStringsFromFile(TmpFile, Lines)
     and (GetArrayLength(Lines) >= 2) then
  begin
    Docs    := Trim(Lines[0]);
    AppData := Trim(Lines[1]);
  end;

  DeleteFile(TmpFile);
end;

// Live writes %APPDATA%\Ableton\Live <version>\ on first launch
function AbletonLiveInstalled(AppData: String): Boolean;
var
  FindRec: TFindRec;
begin
  Result := FindFirst(AppData + '\Ableton\Live *', FindRec);
  if Result then
    FindClose(FindRec);
end;

// ---- build the single page ---------------------------------------------------
procedure InitializeWizard;
var
  DefaultAbleton, OrigDocs, OrigAppData: String;
  VST3Label: TNewStaticText;
begin
  GetOriginalUserFolders(OrigDocs, OrigAppData);
  MainPage := CreateCustomPage(wpWelcome, 'Install {#AppName}',
    'Choose where {#AppName} should be installed.');

  // VST3 folder
  VST3Label := TNewStaticText.Create(MainPage);
  VST3Label.Parent  := MainPage.Surface;
  VST3Label.Caption := 'VST3 plugin folder:';
  VST3Label.Top     := 0;

  VST3Edit := TNewEdit.Create(MainPage);
  VST3Edit.Parent := MainPage.Surface;
  VST3Edit.Top    := VST3Label.Top + ScaleY(18);
  VST3Edit.Width  := MainPage.SurfaceWidth - ScaleX(85);
  VST3Edit.Text   := GetPreviousData('VST3Dir', ExpandConstant('{commoncf64}\VST3'));

  VST3Browse := TNewButton.Create(MainPage);
  VST3Browse.Parent  := MainPage.Surface;
  VST3Browse.Caption := 'Browse...';
  VST3Browse.Left    := MainPage.SurfaceWidth - ScaleX(75);
  VST3Browse.Top     := VST3Edit.Top - ScaleY(1);
  VST3Browse.Width   := ScaleX(75);
  VST3Browse.Height  := ScaleY(23);
  VST3Browse.OnClick := @VST3BrowseClick;

  // Checkbox — pre-ticked if a default Ableton User Library exists on this machine
  DefaultAbleton := OrigDocs + '\Ableton\User Library';

  AbletonCheck := TNewCheckBox.Create(MainPage);
  AbletonCheck.Parent  := MainPage.Surface;
  AbletonCheck.Caption := 'Install Ableton Extension';
  AbletonCheck.Top     := VST3Edit.Top + ScaleY(44);
  AbletonCheck.Width   := MainPage.SurfaceWidth;
  AbletonCheck.Height  := ScaleY(17);
if AbletonLiveInstalled(OrigAppData) or DirExists(DefaultAbleton) then
    AbletonCheck.Checked := GetPreviousData('InstallAbleton', '1') = '1'
  else
    AbletonCheck.Checked := GetPreviousData('InstallAbleton', '0') = '1';
  AbletonCheck.OnClick := @AbletonCheckClick;

  // Ableton User Library folder
  AbletonLabel := TNewStaticText.Create(MainPage);
  AbletonLabel.Parent  := MainPage.Surface;
  AbletonLabel.Caption := 'Ableton User Library folder:';
  AbletonLabel.Top     := AbletonCheck.Top + ScaleY(30);

  AbletonEdit := TNewEdit.Create(MainPage);
  AbletonEdit.Parent := MainPage.Surface;
  AbletonEdit.Top    := AbletonLabel.Top + ScaleY(18);
  AbletonEdit.Width  := MainPage.SurfaceWidth - ScaleX(85);
  AbletonEdit.Text   := GetPreviousData('AbletonUserLibrary', DefaultAbleton);

  AbletonBrowse := TNewButton.Create(MainPage);
  AbletonBrowse.Parent  := MainPage.Surface;
  AbletonBrowse.Caption := 'Browse...';
  AbletonBrowse.Left    := MainPage.SurfaceWidth - ScaleX(75);
  AbletonBrowse.Top     := AbletonEdit.Top - ScaleY(1);
  AbletonBrowse.Width   := ScaleX(75);
  AbletonBrowse.Height  := ScaleY(23);
  AbletonBrowse.OnClick := @AbletonBrowseClick;

  AbletonHint := TNewStaticText.Create(MainPage);
  AbletonHint.Parent   := MainPage.Surface;
  AbletonHint.Top      := AbletonEdit.Top + ScaleY(30);
  AbletonHint.Width    := MainPage.SurfaceWidth;
  AbletonHint.WordWrap := True;
  AbletonHint.Caption  := 'If you moved your User Library in Live (Preferences → Library), select that folder instead.';

  UpdateAbletonVisibility;
end;

// Our page is the last one before installing -> button says "Install"
procedure CurPageChanged(CurPageID: Integer);
begin
  if CurPageID = MainPage.ID then
    WizardForm.NextButton.Caption := SetupMessage(msgButtonInstall);
end;

// ---- validation ---------------------------------------------------------------
function NextButtonClick(CurPageID: Integer): Boolean;
begin
  Result := True;
  if CurPageID <> MainPage.ID then
    Exit;

  if Trim(VST3Edit.Text) = '' then
  begin
    MsgBox('Please choose a VST3 folder.', mbError, MB_OK);
    Result := False;
    Exit;
  end;

  if AbletonCheck.Checked and not DirExists(AbletonEdit.Text) then
    Result := MsgBox('This folder does not exist:' + #13#10 + AbletonEdit.Text + #13#10 + #13#10 +
                     'Live may not be installed, or your User Library is somewhere else. Install there anyway?',
                     mbConfirmation, MB_YESNO) = IDYES;
end;

// ---- remember choices for future updates ---------------------------------------
procedure RegisterPreviousData(PreviousDataKey: Integer);
begin
  SetPreviousData(PreviousDataKey, 'VST3Dir', VST3Edit.Text);
  SetPreviousData(PreviousDataKey, 'AbletonUserLibrary', AbletonEdit.Text);
  if AbletonCheck.Checked then
    SetPreviousData(PreviousDataKey, 'InstallAbleton', '1')
  else
    SetPreviousData(PreviousDataKey, 'InstallAbleton', '0');
end;

// ---- getters used in [Files] / [UninstallDelete] -------------------------------
function GetVST3Dir(Param: String): String;
begin
  Result := VST3Edit.Text;
end;

function GetAbletonDir(Param: String): String;
begin
  Result := AbletonEdit.Text;
end;

function InstallAbleton: Boolean;
begin
  Result := AbletonCheck.Checked;
end;