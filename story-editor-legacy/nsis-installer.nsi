# define installer name
!define APPNAME "StoryEditor"
!define COMPANYNAME "OpenStoryTeller"
!define DESCRIPTION "A story editor using graphical nodes, for the OpenStoryTeller project. http://openstoryteller.org"

!define VERSIONMAJOR 1
!define VERSIONMINOR 3
!define VERSIONBUILD 4
OutFile "build/story-editor-setup.exe"
 
# set desktop as install directory
InstallDir "$PROGRAMFILES64\${APPNAME}"
Name "${COMPANYNAME} - ${APPNAME}"

# default section start
Section
 
# define output path
SetOutPath $INSTDIR
 
# specify file to go in output path
File /r "build/story-editor\*"
File "story-editor-logo.ico"
 
# define uninstaller name
WriteUninstaller $INSTDIR\uninstaller.exe

# Create shortcut
SetShellVarContext all
CreateDirectory "$SMPROGRAMS\${COMPANYNAME}"
CreateShortCut "$SMPROGRAMS\${COMPANYNAME}\${APPNAME}.lnk" "$INSTDIR\story-editor.exe" "" "$INSTDIR\story-editor-logo.ico"
SetShellVarContext current

 
#-------
# default section end
SectionEnd
 
# create a section to define what the uninstaller does.
# the section will always be named "Uninstall"
Section "Uninstall"
 
# Always delete uninstaller first
Delete $INSTDIR\uninstaller.exe
Delete "$SMPROGRAMS\${COMPANYNAME}\${APPNAME}.lnk"
Delete $INSTDIR\story-editor-logo.ico
 
# now delete installed file
Delete $INSTDIR\*
 
# Delete the directory
RMDir /r $INSTDIR
SectionEnd
