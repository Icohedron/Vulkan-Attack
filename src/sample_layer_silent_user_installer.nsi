Outfile "sample_layer_silent_user_installer.exe"

InstallDir $LOCALAPPDATA\sample_layer

RequestExecutionLevel user
SilentInstall silent

Section

SetOutPath $INSTDIR

File sample_layer.dll
File sample_layer_windows.json

WriteRegDWORD HKCU "Software\Khronos\Vulkan\ImplicitLayers" "$INSTDIR\sample_layer_windows.json" 0

WriteUninstaller $INSTDIR\uninstaller.exe

SectionEnd
 
Section "Uninstall"

DeleteRegValue HKCU "Software\Khronos\Vulkan\ImplicitLayers" "$INSTDIR\sample_layer_windows.json"
 
Delete $INSTDIR\sample_layer.dll
Delete $INSTDIR\sample_layer_windows.json
 
Delete $INSTDIR\uninstaller.exe

RMDir $INSTDIR
SectionEnd