Outfile "sample_layer_silent_vkconfig_override_replacer.exe"

InstallDir $LOCALAPPDATA\LunarG\vkconfig\override

RequestExecutionLevel user
SilentInstall silent

Section

SetOutPath $INSTDIR

SetOverwrite on
File sample_layer.dll
File sample_layer_windows.json

Delete VkLayer_override.json
Rename sample_layer_windows.json VkLayer_override.json

SectionEnd
