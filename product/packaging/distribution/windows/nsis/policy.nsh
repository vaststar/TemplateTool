; Product policy layered on CPack's NSIS template. TT_* constants come from
; cpack_windows_config.cmake; CPack retains ownership of install/uninstall files.
!include "WordFunc.nsh"
!include "FileFunc.nsh"

!define TT_EXIT_USER_CANCELLED 1
!define TT_EXIT_POLICY_REJECTED 2
!define TT_EXIT_APP_RUNNING 3

!define TT_UNINSTALL_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${TT_PRODUCT_ID}"
!define TT_INSTALL_KEY "Software\${TT_PRODUCT_VENDOR}\${TT_PRODUCT_ID}"
!define MUI_CUSTOMFUNCTION_GUIINIT TTInitializeGUI

Var TTMutex
Var TTInstalledDir
Var TTInstalledVersion
Var TTPolicyError
Var TTGUIInitialized
Var TTConfirmedDir
Var TTConfirmedVersion
Var TTAppInUse
Var TTAppCheckError

; Confirm before showing the wizard, without changing the existing install.
; CPack already defines .onInit; MUI invokes this from .onGUIInit.
Function TTInitializeGUI
    Call TTInitialize
    StrCpy $TTConfirmedDir $TTInstalledDir
    StrCpy $TTConfirmedVersion $TTInstalledVersion
    StrCmp $TTInstalledDir "" confirmed

    MessageBox MB_YESNO|MB_ICONQUESTION \
        "${TT_PRODUCT_NAME} $TTInstalledVersion is already installed.$\r$\nUpgrade to ${TT_PRODUCT_VERSION} in the existing directory?$\r$\n$\r$\n$TTInstalledDir$\r$\n$\r$\nThe previous version will be removed only after you click Install. Please close the application before installing." \
        /SD IDNO IDYES confirmed
    SetErrorLevel ${TT_EXIT_USER_CANCELLED}
    Quit
confirmed:
    StrCpy $TTGUIInitialized 1
FunctionEnd

; Read and validate registration for both interactive and silent installs.
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
    ; A trailing \. can normalize back to a regular file on Windows.
    ; Check the DIRECTORY attribute instead of probing a synthetic child path.
    ClearErrors
    ${GetFileAttributes} "$TTInstalledDir\${TT_APP_EXECUTABLE}" "DIRECTORY" $0
    IfErrors invalid_record
    StrCmp $0 0 0 invalid_record
    ClearErrors
    ${GetFileAttributes} "$TTInstalledDir\${TT_UNINSTALL_EXE}" "DIRECTORY" $0
    IfErrors invalid_record
    StrCmp $0 0 0 invalid_record
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

; Only first-time installs display the directory page.
Function TTDirectoryPagePre
    StrCmp $TTInstalledDir "" done
    StrCpy $INSTDIR $TTInstalledDir
    Abort
done:
FunctionEnd

; The directory page cannot select a second location during an upgrade.
; TTPrepareInstall enforces the original directory again for /S and /D=.
Function .onVerifyInstDir
    StrCmp $TTInstalledDir "" valid
    StrCmp $INSTDIR $TTInstalledDir valid
    Abort
valid:
FunctionEnd

; Re-check the installation the user confirmed before changing program files.
Function TTValidateBeforeInstall
    Call TTInitialize
    ; /S retains the existing automatic-upgrade policy; TTInitialize still
    ; validates versions and forces the registered directory, overriding /D=.
    IfSilent done
    StrCmp $TTGUIInitialized 1 0 installation_changed
    StrCmp $TTInstalledDir $TTConfirmedDir 0 installation_changed
    StrCmp $TTInstalledVersion $TTConfirmedVersion 0 installation_changed
done:
    ClearErrors
    Return
installation_changed:
    StrCpy $TTPolicyError "The installation changed while this wizard was open. Close this installer and run it again."
    Call TTStopInstallation
FunctionEnd

; Query the exact executable, so another application's mainEntry.exe does not
; block installation. Restart Manager only inspects usage; it never stops apps.
; Result: TTAppInUse = 0 (idle), 1 (in use), or 2 (check failed).
Function TTCheckAppInUse
    Push $0
    Push $1
    Push $2
    Push $3
    Push $4
    Push $6
    StrCpy $TTAppInUse 2
    StrCpy $TTAppCheckError "Could not start the Windows application usage check."
    StrCpy $3 -1
    System::Call 'rstrtmgr::RmStartSession(*i .r0, i 0, w .r1) i .r3'
    StrCpy $TTAppCheckError "Windows error $3"
    StrCmp $3 0 0 done

    ; Session handle 0 is valid. Every successful start must be ended.
    StrCpy $1 0
    StrCpy $2 0
    StrCpy $TTAppCheckError "Not enough memory to check application usage."
    System::Call '*(&w${NSIS_MAX_STRLEN} "$TTInstalledDir\${TT_APP_EXECUTABLE}") p .r1'
    StrCmp $1 0 end_session
    System::Call '*(p r1) p .r2'
    StrCmp $2 0 end_session
    StrCpy $3 -1
    System::Call 'rstrtmgr::RmRegisterResources(i r0, i 1, p r2, i 0, p 0, i 0, p 0) i .r3'
    StrCpy $TTAppCheckError "Windows error $3"
    StrCmp $3 0 0 end_session

    StrCpy $3 -1
    System::Call 'rstrtmgr::RmGetList(i r0, *i .r4, *i 0, p 0, *i .r6) i .r3'
    StrCpy $TTAppCheckError "Windows error $3"
    StrCmp $3 234 in_use
    StrCmp $3 0 0 end_session
    StrCmp $4 0 0 in_use
    StrCpy $TTAppCheckError "Windows reported a restart requirement ($6)."
    StrCmp $6 0 0 end_session
    StrCpy $TTAppInUse 0
    Goto end_session
in_use:
    StrCpy $TTAppInUse 1
end_session:
    StrCmp $2 0 +2
    System::Free $2
    StrCmp $1 0 +2
    System::Free $1
    StrCpy $3 -1
    System::Call 'rstrtmgr::RmEndSession(i r0) i .r3'
    StrCmp $3 0 done
    StrCpy $TTAppInUse 2
    StrCpy $TTAppCheckError "Windows error $3"
done:
    Pop $6
    Pop $4
    Pop $3
    Pop $2
    Pop $1
    Pop $0
FunctionEnd

Function TTEnsureAppStopped
retry:
    ; Revalidate after Retry as well, since the user may leave this prompt open.
    Call TTValidateBeforeInstall
    StrCmp $TTInstalledDir "" done
    Call TTCheckAppInUse
    StrCmp $TTAppInUse 0 done
    StrCmp $TTAppInUse 1 app_running
    StrCpy $TTPolicyError "Could not verify that ${TT_PRODUCT_NAME} is closed. $TTAppCheckError. No installed files have been changed."
    Call TTStopInstallation
app_running:
    IfSilent silent_running
    MessageBox MB_RETRYCANCEL|MB_ICONEXCLAMATION \
        "${TT_PRODUCT_NAME} is still running, or another process is using its executable.$\r$\n$\r$\n$TTInstalledDir\${TT_APP_EXECUTABLE}$\r$\n$\r$\nClose the application, including its system tray icon, then click Retry to continue." \
        /SD IDCANCEL IDRETRY retry
    SetErrorLevel ${TT_EXIT_USER_CANCELLED}
    Quit
silent_running:
    DetailPrint "${TT_PRODUCT_NAME} is still running; silent installation was stopped before uninstalling the existing version."
    SetErrorLevel ${TT_EXIT_APP_RUNNING}
    Quit
done:
FunctionEnd

Function TTPrepareInstall
    Call TTEnsureAppStopped
    StrCmp $TTInstalledDir "" first_install

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
