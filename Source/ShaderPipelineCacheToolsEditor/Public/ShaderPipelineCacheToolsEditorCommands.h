// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"
#include "ShaderPipelineCacheToolsEditorStyle.h"

class FShaderPipelineCacheToolsEditorCommands : public TCommands<FShaderPipelineCacheToolsEditorCommands>
{
public:

	FShaderPipelineCacheToolsEditorCommands()
		: TCommands<FShaderPipelineCacheToolsEditorCommands>(TEXT("ShaderPipelineCacheToolsEditor"), NSLOCTEXT("Contexts", "ShaderPipelineCacheToolsEditor", "ShaderPipelineCacheToolsEditor Plugin"), NAME_None, FShaderPipelineCacheToolsEditorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};