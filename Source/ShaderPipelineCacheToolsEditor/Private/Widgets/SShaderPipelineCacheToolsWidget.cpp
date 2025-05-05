// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/SShaderPipelineCacheToolsWidget.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Settings/ProjectPackagingSettings.h"
#include "SlateOptMacros.h"
#include "IDesktopPlatform.h"
#include "DesktopPlatformModule.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SShaderPipelineCacheToolsWidget::Construct(const FArguments& InArgs)
{
	Platforms = GetAllPlatforms(); 
	bPSOBundledSettingsOk = AreBundledPSOSettingsOk();

	const FMargin padding = FMargin(4,2);
	ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.VAlign(EVerticalAlignment::VAlign_Top)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Right)
			//.AutoWidth()
			.Padding(padding)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Bundled PSO settings status: "))
			]
			+ SHorizontalBox::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Left)
			//.AutoWidth()
			.Padding(padding)
			[
				SNew(STextBlock)
				.Text_Lambda([this]() {return FText::FromString(bPSOBundledSettingsOk ? "Ok" : "Needs fix"); })
			]			
		]
		+ SVerticalBox::Slot()
		.VAlign(EVerticalAlignment::VAlign_Top)
		.AutoHeight()
		[
			SNew(SButton)
			.Text(FText::FromString("Fix Bundled PSO Settings"))
			.IsEnabled_Lambda([this]() {return !bPSOBundledSettingsOk; })
			.OnClicked(this, &SShaderPipelineCacheToolsWidget::FixBundledPSOSettings)
			.HAlign(EHorizontalAlignment::HAlign_Center)
		]
		+SVerticalBox::Slot()
		.VAlign(EVerticalAlignment::VAlign_Top)
		.AutoHeight()
		.Padding(padding)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Right)
			.Padding(padding)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Platform: "))
			]
			+ SHorizontalBox::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Left)
			.Padding(padding)
			[
				SAssignNew(PlatformComboBox, SComboBox<FName>)
				.OptionsSource(&Platforms)
				.OnGenerateWidget_Lambda([](FName InName) {
					return SNew(STextBlock).Text(FText::FromName(InName)); 
					})
				[
					SNew(STextBlock)
						.Text_Lambda([this]() { return FText::FromName(PlatformComboBox->GetSelectedItem()); })
				]
			]
		]
		+ SVerticalBox::Slot()
		.VAlign(EVerticalAlignment::VAlign_Top)
		.AutoHeight()
		.Padding(padding)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Right)
			.Padding(padding)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Prefix:"))
			]
			+ SHorizontalBox::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Left)
			.Padding(padding)
			[
				SNew(SEditableTextBox)
				.HintText(FText::FromString("Prefix for the shaderCache ([YourPrefix][ProjectName][ShaderFormatName].spc)"))
				.Text(FText::FromString(Prefix))
				.OnTextCommitted_Lambda([this](const FText& Val, ETextCommit::Type TextCommitType)
				{
					Prefix = Val.ToString();
				})
			]
		]
		+ SVerticalBox::Slot()
		.VAlign(EVerticalAlignment::VAlign_Top)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Right)
			.AutoWidth()
			.Padding(padding)
			[
				SNew(SButton)
				.Text(FText::FromString("Select shaderKeys"))
				.OnClicked(this,&SShaderPipelineCacheToolsWidget::HandleShaderKeySelection)
				.VAlign(EVerticalAlignment::VAlign_Center)
			]
			+ SHorizontalBox::Slot()
			[
				SAssignNew(PipelineKeysListbox, SListView<FName>)
				.ListItemsSource(&ShaderKeyFiles)
				.OnGenerateRow(this, &SShaderPipelineCacheToolsWidget::GenerateRow)
				.OnContextMenuOpening_Lambda([this]() {return OnOpenContextMenu(PipelineKeysListbox, ShaderKeyFiles); })
			]
		]
		+ SVerticalBox::Slot()
		.VAlign(EVerticalAlignment::VAlign_Top)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Left)
			.AutoWidth()
			.Padding(padding)
			[
				SNew(SButton)
				.Text(FText::FromString("Select pipeline caches"))
				.OnClicked(this, &SShaderPipelineCacheToolsWidget::HandlePipelineCacheSelection)
				.VAlign(EVerticalAlignment::VAlign_Center)
			]
			+ SHorizontalBox::Slot()
			[
				SAssignNew(PipelineCachesListbox,SListView<FName>)
				.ListItemsSource(&PipelineCachesFiles)
				.OnGenerateRow(this,&SShaderPipelineCacheToolsWidget::GenerateRow)
				.OnContextMenuOpening_Lambda([this]() {return OnOpenContextMenu(PipelineCachesListbox, PipelineCachesFiles); })
			]
		]
		+ SVerticalBox::Slot()
		.VAlign(EVerticalAlignment::VAlign_Top)
		.AutoHeight()
		[
			SNew(SButton)
			.Text(FText::FromString("Combine PSO Caches"))
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.OnClicked(this, &SShaderPipelineCacheToolsWidget::CombinePSOFiles)
		]
	];
}

bool SShaderPipelineCacheToolsWidget::AreBundledPSOSettingsOk() const
{
	bool bAllPlatformsNeedStableKeys = false; 
	const FString EngineIniFile = GetDefaultEngineIniName(); 
	GConfig->GetBool(TEXT("DevOptions.Shaders"), TEXT("NeedsShaderStableKeys"), bAllPlatformsNeedStableKeys, EngineIniFile);
	const UProjectPackagingSettings* packagingSettings = GetDefault<UProjectPackagingSettings>();
	return bAllPlatformsNeedStableKeys && packagingSettings->bShareMaterialShaderCode && packagingSettings->bSharedMaterialNativeLibraries;
}

FReply SShaderPipelineCacheToolsWidget::FixBundledPSOSettings()
{
	UProjectPackagingSettings* packagingSettings = GetMutableDefault<UProjectPackagingSettings>();
	packagingSettings->bSharedMaterialNativeLibraries = true; 
	packagingSettings->bShareMaterialShaderCode = true;
	FProperty* sharedMaterialNativeLibsProperty = packagingSettings->GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UProjectPackagingSettings, bSharedMaterialNativeLibraries));
	FProperty* sharedMaterialShaderCodeProperty = packagingSettings->GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UProjectPackagingSettings, bShareMaterialShaderCode));
	packagingSettings->UpdateSinglePropertyInConfigFile(sharedMaterialNativeLibsProperty, packagingSettings->GetDefaultConfigFilename());
	packagingSettings->UpdateSinglePropertyInConfigFile(sharedMaterialShaderCodeProperty, packagingSettings->GetDefaultConfigFilename());

	const bool bAllPlatformsNeedStableKeys = true;
	const FString EngineIniFile = GetDefaultEngineIniName(); 
	FConfigFile* configFile = GConfig->FindConfigFile(EngineIniFile); 
	GConfig->SetBool(TEXT("DevOptions.Shaders"), TEXT("NeedsShaderStableKeys"), bAllPlatformsNeedStableKeys, EngineIniFile);
	configFile->UpdateSinglePropertyInSection(*EngineIniFile, TEXT("NeedsShaderStableKeys"), TEXT("DevOptions.Shaders"));
	bPSOBundledSettingsOk = AreBundledPSOSettingsOk();
	return FReply::Handled(); 
}

FReply SShaderPipelineCacheToolsWidget::HandlePipelineCacheSelection()
{	
	OpenFileDialog("select shader caches", "", "pipeline cache|*.rec.upipelinecache", PipelineCachesFiles, PipelineCachesListbox);
	return FReply::Handled();
}

FReply SShaderPipelineCacheToolsWidget::HandleShaderKeySelection()
{
	const FString DefaultOutputPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Cooked"), PlatformComboBox->GetSelectedItem().ToString(), FApp::GetProjectName(), TEXT("Metadata"), TEXT("PipelineCaches"));
	OpenFileDialog("select shader keys", DefaultOutputPath, "shader keys|*.shk", ShaderKeyFiles, PipelineKeysListbox);
	return FReply::Handled();
}

void SShaderPipelineCacheToolsWidget::OpenFileDialog(const FString& DialogTitle, const FString& DefaultPath, const FString& FileTypes, TArray<FName>& SourceItems, const TSharedPtr<SListView<FName>>& ListView)
{
	TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(AsShared());
	const void* ParentWindowHandle = nullptr;
	if (ParentWindow.IsValid() && ParentWindow->GetNativeWindow().IsValid())
	{
		ParentWindowHandle = ParentWindow->GetNativeWindow()->GetOSWindowHandle();
	}
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
	{
		return;
	}
	TArray<FString> OutNames;
	if (!DesktopPlatform->OpenFileDialog(ParentWindowHandle, DialogTitle, DefaultPath, FString(""), FileTypes, EFileDialogFlags::Multiple, OutNames))
	{
		return;
	}
	for (const FString& File : OutNames)
	{
		SourceItems.AddUnique(FName(File));
	}
	if (ListView.IsValid())
	{
		ListView->RequestListRefresh();
	}
}

TSharedRef<ITableRow> SShaderPipelineCacheToolsWidget::GenerateRow(FName RowName, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<FName>, OwnerTable)
		[
			SNew(STextBlock).Text(FText::FromName(RowName))
			.AutoWrapText(true)
		];
}

TSharedPtr<SWidget> SShaderPipelineCacheToolsWidget::OnOpenContextMenu(TSharedPtr<SListView<FName>> listBox, TArray<FName>& ItemsSource)
{
	const TArray<FName> SelectedItems = listBox->GetSelectedItems();
	UToolMenus* ToolMenus = UToolMenus::Get();
	static const FName MenuName = "ShaderPipelineCacheToolsEditor.ContextMenu";
	if (!ToolMenus->IsMenuRegistered(MenuName))
	{
		UToolMenu* RegisteredMenu = ToolMenus->RegisterMenu(MenuName);
		// Add section so it can be used as insert position for menu extensions
		RegisteredMenu->AddSection("ShaderPipelineCacheToolsEditor");
	}
	FToolMenuContext Context;
	UToolMenu* Menu = ToolMenus->GenerateMenu(MenuName, Context);
	FToolMenuSection& Section = *Menu->FindSection("ShaderPipelineCacheToolsEditor");
	Section.AddMenuEntry(TEXT("Delete Item(s)"),
		FText::FromString("Delete Item(s)"),
		FText::FromString("Delete Item(s)"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([&ItemsSource, SelectedItems, listBox]()
				{
					for (const FName& Name : SelectedItems)
					{
						ItemsSource.Remove(Name);
					}
					listBox->RequestListRefresh();
				}),
			FCanExecuteAction()
		));
	return ToolMenus->GenerateWidget(Menu);
}

FReply SShaderPipelineCacheToolsWidget::CombinePSOFiles()
{
	// Handle error handeling 
	if (PlatformComboBox->GetSelectedItem().IsNone())
	{
		AddErrorMessage("Selected platform was not set"); 
		return FReply::Handled();
	}
	if (ShaderKeyFiles.IsEmpty())
	{
		AddErrorMessage("No shader key files selected");
		return FReply::Handled();
	}
	if (PipelineCachesFiles.IsEmpty())
	{
		AddErrorMessage("No pipeline cache files selected");
		return FReply::Handled();
	}
	if (Prefix.IsEmpty())
	{
		AddErrorMessage("Prefix was empty");
		return FReply::Handled();
	}
	TArray<FString> Errors; 
	const FString ShaderFormat = VerifyFiles(Errors);
	if (Errors.Num() != 0)
	{
		AddErrorMessage(FString::Join(Errors, TEXT("\n"))); 
		return FReply::Handled();
	}

	// Copying files to the temp directory 
	const FString TempDirectory = FPaths::Combine(FPaths::ProjectSavedDir(), "ShaderPipelineCacheTools");
	IFileManager::Get().DeleteDirectory(*TempDirectory,false,true);
	IFileManager::Get().MakeDirectory(*TempDirectory);
	for (const FName& File : ShaderKeyFiles)
	{
		CopyFile(File, TempDirectory);
	}
	for (const FName& File : PipelineCachesFiles)
	{
		CopyFile(File, TempDirectory);
	}
	// Commandlet to run for combining the PSO caches
	FProcHandle ProcessHandle;
	bool bCancelled = false;
	
	const FString OutputFile = FString::Printf(TEXT("%s_%s_%s.spc"), *Prefix, FApp::GetProjectName(), *ShaderFormat);
	const FString OutputPath = FPaths::Combine(FPaths::ProjectDir(), "Build", PlatformComboBox->GetSelectedItem().ToString(), "PipelineCaches", OutputFile);

	// Creating the output directory if it doesn't exist
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(OutputPath),true);

	// Copied from FEditorBuildUtils::RunWorldPartitionBuilder
	// Task scope
	{
		FScopedSlowTask SlowTask(0, FText::FromString("Combining PSO Caches"));
		SlowTask.MakeDialog(true);

		const FString EnginePath = FPaths::Combine(IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::EngineDir()), "Binaries", "Win64", "UnrealEditor-Cmd.exe");
		// Full Commandline From the documentation: 
		// Engine\Binaries\Win64\UnrealEditor-Cmd.exe [ProjectName] -run=ShaderPipelineCacheTools expand C:\PSOCache\*.rec.upipelinecache C:\PSOCache\*.shk C:\PSOCache\[YourPrefix][ProjectName][ShaderFormatName].spc

		const FString UpropertyFile = FPaths::GetProjectFilePath();
		const FString CommandLineArguments = FString::Printf(TEXT("\"%s\" -run=ShaderPipelineCacheTools expand \"%s/*.rec.upipelinecache\" \"%s/*.shk\" \"%s\""), *UpropertyFile, *TempDirectory, *TempDirectory, *OutputPath);

		void* PipeRead = nullptr;
		void* PipeWrite = nullptr;
		verify(FPlatformProcess::CreatePipe(PipeRead, PipeWrite));


		uint32 ProcessID;
		ProcessHandle = FPlatformProcess::CreateProc(
			*EnginePath,
			*CommandLineArguments,
			/*bLaunchedDetached*/true,
			/*bLaunchedHidden*/false,
			/*bLaunchedReallyHidden*/false,
			&ProcessID,
			/*PriorityModifier*/0,
			/*OptionalWorkingDirectory*/nullptr,
			/*PipeWriteChild*/PipeWrite);

		while (FPlatformProcess::IsProcRunning(ProcessHandle))
		{
			if (SlowTask.ShouldCancel())
			{
				bCancelled = true;
				FPlatformProcess::TerminateProc(ProcessHandle);
				break;
			}
			const FString Output = FPlatformProcess::ReadPipe(PipeRead);
			TArray<FString> SplitOutput;
			Output.ParseIntoArrayLines(SplitOutput);
			for (const FString& OutputLine : SplitOutput)
			{
				UE_LOG(LogTemp, Display, TEXT("%s"), *OutputLine);
			}

			SlowTask.EnterProgressFrame(0);
			FPlatformProcess::Sleep(0.1);
		}
		const FString Output = FPlatformProcess::ReadPipe(PipeRead); 
		TArray<FString> SplitOutput;
		Output.ParseIntoArrayLines(SplitOutput);
		for (const FString& OutputLine : SplitOutput)
		{
			UE_LOG(LogTemp, Display, TEXT("%s"), *OutputLine);
		}
		FPlatformProcess::ClosePipe(PipeRead, PipeWrite);
	}

	if (bCancelled)
	{
		FMessageDialog::Open(EAppMsgCategory::Info, EAppMsgType::Ok, FText::FromString("PSO Combining Cancelled"));
		return FReply::Handled();
	}
	int32 Result = 0; 
	if (FPlatformProcess::GetProcReturnCode(ProcessHandle, &Result) && Result == 0)
	{
		FMessageDialog::Open(EAppMsgCategory::Info, EAppMsgType::Ok, FText::FromString("PSO CacheFile Saved at: " + OutputPath));
		return FReply::Handled();
	}
	AddErrorMessage("PSO Combining Failed, Check logs for details");
	return FReply::Handled();
}

void SShaderPipelineCacheToolsWidget::AddErrorMessage(const FString& ErrorText)
{
	FNotificationInfo Info(FText::FromString(ErrorText));
	Info.ExpireDuration = 5.0f;
	Info.bUseSuccessFailIcons = true;
	Info.Image = FAppStyle::GetBrush(TEXT("MessageLog.Error"));
	FSlateNotificationManager::Get().AddNotification(Info);
	FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, FText::FromString(ErrorText));
}

void SShaderPipelineCacheToolsWidget::CopyFile(const FName& File, const FString& ToDirectory)
{
	FString CurrentFile = File.ToString();
	const FString CleanFileName = FPaths::GetCleanFilename(CurrentFile);
	const FString Directory = FPaths::GetPath(CurrentFile);
	CopyFile(CleanFileName, Directory, ToDirectory);
}

void SShaderPipelineCacheToolsWidget::CopyFile(const FString& BaseFileName, const FString& FromDirectory, const FString& ToDirectory, int Attempt)
{
	const FString Extension = FPaths::GetExtension(BaseFileName, true);
	const FString FileName = FPaths::GetBaseFilename(BaseFileName);
	FString FullPath = FPaths::Combine(ToDirectory, FileName + (Attempt == 0 ? "" : "_" + FString::FromInt(Attempt)) + Extension);
	if (IFileManager::Get().FileExists(*FullPath))
	{
		CopyFile(BaseFileName, FromDirectory, ToDirectory, Attempt + 1);
		return;
	}
	FString OriginalPath = FPaths::Combine(FromDirectory, BaseFileName);
	IFileManager::Get().Copy(*FullPath, *OriginalPath);
}

const FString SShaderPipelineCacheToolsWidget::VerifyFiles(TArray<FString>& Errors) const
{
	// Check if every shader key file has the same ID
	// Normally the id of the shaderpipelinecache ends with -ShaderFormatName.shk
	// ShaderStableInfo-Global-PCD3D_SM5.shk
	const FString ShaderFormat = GetShaderFormatFromShaderKeyFile(ShaderKeyFiles[0]);
	if (ShaderFormat.IsEmpty())
	{
		Errors.Add("Shader Format not found on file: " + ShaderKeyFiles[0].ToString());
		return "";
	}
	const int32 Size = ShaderKeyFiles.Num();
	for (int i = 1; i < Size; i++)
	{
		const FString CurrentShaderFormat = GetShaderFormatFromShaderKeyFile(ShaderKeyFiles[i]);
		if (CurrentShaderFormat.IsEmpty())
		{
			Errors.Add("Shader Format not found on file: " + ShaderKeyFiles[i].ToString());
			continue;
		}
		if (ShaderFormat != CurrentShaderFormat)
		{
			Errors.Add("Current shader format:" + CurrentShaderFormat + " not equal to first shader format found: " + ShaderFormat + " File: " + ShaderKeyFiles[i].ToString());
			continue;
		}
	}

	for (const FName& CurrentFile : PipelineCachesFiles)
	{
		FString FoundShaderFormat = GetShaderFormatFromPipelinecacheFile(CurrentFile); 
		if (FoundShaderFormat.IsEmpty())
		{
			Errors.Add("Shader Format not found on file: " + CurrentFile.ToString());
			continue;
		}
		if (ShaderFormat != FoundShaderFormat)
		{
			Errors.Add("Current shader format:" + FoundShaderFormat + " not equal to first shader format found: " + ShaderFormat + " File: " + CurrentFile.ToString());
			continue;
		}
	}

	return ShaderFormat;
}

FString SShaderPipelineCacheToolsWidget::GetShaderFormatFromShaderKeyFile(FName StableShaderFile)const
{
	// Normally the id of the shaderpipelinecache ends with -ShaderFormatName.shk
	// ShaderStableInfo-Global-PCD3D_SM5.shk
	FString File = StableShaderFile.ToString();
	int Index = INDEX_NONE;
	File.FindLastChar('-', Index);
	if (Index == INDEX_NONE)
	{
		return FString();
	}
	FString ShaderFormat = File.RightChop(Index + 1);
	ShaderFormat.ReplaceInline(TEXT(".shk"), TEXT(""));
	return ShaderFormat;
}

FString SShaderPipelineCacheToolsWidget::GetShaderFormatFromPipelinecacheFile(FName PipelineCacheFile)const
{
	//++UE5+Release-5.5-CL-40574608-PluginCreator_PCD3D_SM5_CB1A39874D0D96EDC8F2F28BD0E9834C.rec.upipelinecache
	const FRegexPattern Pattern = FRegexPattern(FString::Printf(TEXT(".*-%s_(.*)?_.*"), FApp::GetProjectName()), ERegexPatternFlags::None);
	FRegexMatcher Matcher = FRegexMatcher(Pattern, PipelineCacheFile.ToString());
	if (Matcher.FindNext())
	{
		const FString ShaderGroup = Matcher.GetCaptureGroup(1);
		return ShaderGroup;
	}
	return FString();
}

const FString SShaderPipelineCacheToolsWidget::GetDefaultEngineIniName() const
{
	return FConfigCacheIni::NormalizeConfigIniPath(FString::Printf(TEXT("%sDefault%s.ini"), *FPaths::SourceConfigDir(), TEXT("Engine")));;
}

const TArray<FName> SShaderPipelineCacheToolsWidget::GetAllPlatforms() const
{
	TArray<FName> returnValue; 
	for (const auto& Pair : FDataDrivenPlatformInfoRegistry::GetAllPlatformInfos())
	{
		if (Pair.Value.bIsFakePlatform || Pair.Value.bEnabledForUse == false)
		{
			continue;
		}

		FName PlatformName = Pair.Key;
		const FDataDrivenPlatformInfo& Info = Pair.Value;
		if (FDataDrivenPlatformInfoRegistry::IsPlatformHiddenFromUI(PlatformName))
		{
			continue;
		}
		returnValue.Add(PlatformName);
	}
	return returnValue;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
