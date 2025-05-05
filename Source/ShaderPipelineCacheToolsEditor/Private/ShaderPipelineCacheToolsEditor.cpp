// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShaderPipelineCacheToolsEditor.h"
#include "ShaderPipelineCacheToolsEditorStyle.h"
#include "ShaderPipelineCacheToolsEditorCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SShaderPipelineCacheToolsWidget.h"
#include "ToolMenus.h"

static const FName ShaderPipelineCacheToolsEditorTabName("ShaderPipelineCacheToolsEditor");

#define LOCTEXT_NAMESPACE "FShaderPipelineCacheToolsEditorModule"

void FShaderPipelineCacheToolsEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FShaderPipelineCacheToolsEditorStyle::Initialize();
	FShaderPipelineCacheToolsEditorStyle::ReloadTextures();

	FShaderPipelineCacheToolsEditorCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FShaderPipelineCacheToolsEditorCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FShaderPipelineCacheToolsEditorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FShaderPipelineCacheToolsEditorModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(ShaderPipelineCacheToolsEditorTabName, FOnSpawnTab::CreateRaw(this, &FShaderPipelineCacheToolsEditorModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FShaderPipelineCacheToolsEditorTabTitle", "ShaderPipelineCacheToolsEditor"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FShaderPipelineCacheToolsEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FShaderPipelineCacheToolsEditorStyle::Shutdown();

	FShaderPipelineCacheToolsEditorCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ShaderPipelineCacheToolsEditorTabName);
}

TSharedRef<SDockTab> FShaderPipelineCacheToolsEditorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Put your tab content here!
			SNew(SShaderPipelineCacheToolsWidget)
		];
}

void FShaderPipelineCacheToolsEditorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(ShaderPipelineCacheToolsEditorTabName);
}

void FShaderPipelineCacheToolsEditorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
		{
			FName SectionName = "Enrique's Tools";
			FToolMenuSection* Section = Menu->FindSection(SectionName);
			if (!Section)
			{
				Section = &Menu->AddSection(SectionName, FText::FromString(SectionName.ToString()));
			}
			Section->AddMenuEntryWithCommandList(FShaderPipelineCacheToolsEditorCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FShaderPipelineCacheToolsEditorCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FShaderPipelineCacheToolsEditorModule, ShaderPipelineCacheToolsEditor)