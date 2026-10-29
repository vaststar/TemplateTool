; Product policy layered on CPack's NSIS template. TT_* constants come from
; cpack_windows_config.cmake; CPack retains ownership of install/uninstall files.
!include "WordFunc.nsh"
!include "FileFunc.nsh"

!define TT_EXIT_USER_CANCELLED 1
!define TT_EXIT_POLICY_REJECTED 2

!define TT_UNINSTALL_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${TT_PRODUCT_ID}"
!define TT_INSTALL_KEY "Software\${TT_PRODUCT_VENDOR}\${TT_PRODUCT_ID}"
!define MUI_CUSTOMFUNCTION_GUIINIT TTInitialize

Var TTMutex
Var TTInstalledDir
Var TTInstalledVersion
Var TTPolicyError

; CPack already defines .onInit; MUI calls this before showing its first page.
; Silent installs call the same policy through TTPrepareInstall instead.
Function TTInitialize
    SetRegView 32
    SetShellVarContext all
    StrCpy $SV_ALLUSERS "AllUsers"

    StrCmp $TTMutex "" 0 read_installation
    System::Call 'kernel32::CreateMutexW(p 0, i 0, w "Global\${TT_PRODUCT_ID}.Installer") p .r0 ?e'
    Pop $1
    StrCmp $0 0 mutex_failed
    StrCmp $1 183 mutex_exists
    StrCpy $TTMutex $0

read_installation:
    ; Use the same registry view and product keys as CPack's own uninstaller.
    ReadRegStr $TTInstalledDir HKLM "${TT_INSTALL_KEY}" ""
    ReadRegStr $TTInstalledVersion HKLM "${TT_UNINSTALL_KEY}" "DisplayVersion"
    StrCmp $TTInstalledDir "" 0 installed
    StrCmp $TTInstalledVersion "" done invalid_record

installed:
    StrCmp $TTInstalledVersion "" invalid_record
    GetFullPathName $0 "$TTInstalledDir"
    StrCmp $0 $TTInstalledDir 0 invalid_record
    GetFullPathName $0 "$TTInstalledDir\.."
    StrCmp $0 $TTInstalledDir invalid_record
    IfFileExists "$TTInstalledDir\${TT_APP_EXECUTABLE}" 0 invalid_record
    IfFileExists "$TTInstalledDir\${TT_APP_EXECUTABLE}\." invalid_record
    IfFileExists "$TTInstalledDir\${TT_UNINSTALL_EXE}" 0 invalid_record
    IfFileExists "$TTInstalledDir\${TT_UNINSTALL_EXE}\." invalid_record
    ReadRegStr $0 HKLM "${TT_UNINSTALL_KEY}" "UninstallString"
    StrCmp $0 '$\"$TTInstalledDir\${TT_UNINSTALL_EXE}$\"' 0 invalid_record

    ${VersionCompare} "$TTInstalledVersion" "${TT_PRODUCT_VERSION}" $0
    StrCmp $0 0 same_version
    StrCmp $0 1 newer_version
    StrCpy $INSTDIR $TTInstalledDir
done:
    ClearErrors
    Return

mutex_failed:
    StrCpy $TTPolicyError "The installer could not create its installation lock."
    Goto fail
mutex_exists:
    System::Call 'kernel32::CloseHandle(p r0)'
    StrCpy $TTPolicyError "Another ${TT_PRODUCT_NAME} installer is already running."
    Goto fail
invalid_record:
    StrCpy $TTPolicyError "The existing installation record is incomplete. Uninstall ${TT_PRODUCT_NAME} from Windows Settings before installing again."
    Goto fail
same_version:
    StrCpy $TTPolicyError "${TT_PRODUCT_NAME} ${TT_PRODUCT_VERSION} is already installed."
    Goto fail
newer_version:
    StrCpy $TTPolicyError "A newer ${TT_PRODUCT_NAME} version ($TTInstalledVersion) is already installed. Downgrades are not allowed."
fail:
    Call TTStopInstallation
FunctionEnd

; The directory page cannot select a second location during an upgrade.
; TTPrepareInstall enforces the original directory again for /S and /D=.
Function .onVerifyInstDir
    StrCmp $TTInstalledDir "" valid
    StrCmp $INSTDIR $TTInstalledDir valid
    Abort
valid:
FunctionEnd

Function TTPrepareInstall
    ; Re-read registration immediately before changing any installed files.
    Call TTInitialize
    StrCmp $TTInstalledDir "" first_install

    MessageBox MB_YESNO|MB_ICONQUESTION \
        "Upgrade ${TT_PRODUCT_NAME} $TTInstalledVersion to ${TT_PRODUCT_VERSION}?$\r$\n$\r$\nInstallation directory: $TTInstalledDir$\r$\nPlease close the application before continuing." \
        /SD IDYES IDYES upgrade
    SetErrorLevel ${TT_EXIT_USER_CANCELLED}
    Quit

upgrade:
    ; Only execute the expected uninstaller in the registered application root.
    ; Run a private copy so the old uninstaller can delete its installed binary.
    ; _?= must be the final, unquoted argument so ExecWait waits for the actual
    ; uninstaller instead of its temporary relauncher, including paths with spaces.
    InitPluginsDir
    SetOutPath "$TEMP"
    ClearErrors
    CopyFiles /SILENT "$TTInstalledDir\${TT_UNINSTALL_EXE}" "$PLUGINSDIR\old-uninstall.exe"
    IfErrors uninstall_failed
    ClearErrors
    ExecWait '$\"$PLUGINSDIR\old-uninstall.exe$\" /S _?=$TTInstalledDir' $0
    IfErrors uninstall_failed
    StrCmp $0 0 0 uninstall_failed

    ; CPack's generated uninstaller does not turn every Delete error into a
    ; nonzero exit code. Check registration and refuse any remaining old files.
    ReadRegStr $0 HKLM "${TT_UNINSTALL_KEY}" "DisplayVersion"
    StrCmp $0 "" 0 uninstall_failed
    ReadRegStr $0 HKLM "${TT_INSTALL_KEY}" ""
    StrCmp $0 "" 0 uninstall_failed

    StrCpy $INSTDIR $TTInstalledDir
    IfFileExists "$TTInstalledDir\." 0 ready
    ${DirState} "$TTInstalledDir" $0
    StrCmp $0 0 ready uninstall_failed

first_install:
    ; CPack already created the selected directory before entering this hook.
    ; Normalize it before CPack registers it, and require a dedicated empty root.
    ClearErrors
    GetFullPathName $INSTDIR "$INSTDIR"
    IfErrors directory_failed
    GetFullPathName $0 "$INSTDIR\.."
    StrCmp $0 $INSTDIR directory_failed
    ${DirState} "$INSTDIR" $0
    StrCmp $0 0 ready directory_not_empty
ready:
    ClearErrors
    SetOutPath "$INSTDIR"
    IfErrors directory_failed
    Return

directory_not_empty:
    StrCpy $TTPolicyError "Choose an empty application directory for the first installation."
    Goto fail
directory_failed:
    StrCpy $TTPolicyError "The installation directory could not be prepared."
    Goto fail
uninstall_failed:
    StrCpy $TTPolicyError "The previous version could not be removed completely and may be partially uninstalled. Close the application, resolve any remaining files in the original installation directory, then run this installer again."
fail:
    Call TTStopInstallation
FunctionEnd

Function TTStopInstallation
    DetailPrint "$TTPolicyError"
    MessageBox MB_OK|MB_ICONSTOP "$TTPolicyError" /SD IDOK
    SetErrorLevel ${TT_EXIT_POLICY_REJECTED}
    Quit
FunctionEnd
