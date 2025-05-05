// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShaderPipelineCacheToolsEditorCommands.h"

#define LOCTEXT_NAMESPACE "FShaderPipelineCacheToolsEditorModule"

void FShaderPipelineCacheToolsEditorCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "ShaderPipelineCacheToolsEditor", "Bring up ShaderPipelineCacheToolsEditor window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
