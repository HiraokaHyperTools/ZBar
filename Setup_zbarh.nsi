; example2.nsi
;
; This script is based on example1.nsi, but it remember the directory, 
; has uninstall support and (optionally) installs start menu shortcuts.
;
; It will install example2.nsi into a directory that the user selects,

;--------------------------------

Unicode true

!define APP "zbarh"

!define ZBAR_KEY "Software\ZBar"

!system 'MySign "release\zbarimg.exe"'
!finalize 'MySign "%1"'

!system 'DefineAsmVer.exe "release\zbarimg.exe" "!define VER ""[SFVER]"" " > Tmpver.nsh'
!include "Tmpver.nsh"

; The name of the installer
Name "${APP} ${VER}"

; The file to write
OutFile "Setup_${APP}.exe"

; The default installation directory
InstallDir "$PROGRAMFILES\ZBar"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM ${ZBAR_KEY} "InstallDir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

XPStyle on

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "Update zbarimg.exe"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath "$INSTDIR\bin"
  
  ; Put file there
  File "release\zbarimg.exe"
  
SectionEnd

;--------------------------------
