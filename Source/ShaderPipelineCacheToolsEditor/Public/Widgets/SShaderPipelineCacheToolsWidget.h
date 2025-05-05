// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */

class SHADERPIPELINECACHETOOLSEDITOR_API SShaderPipelineCacheToolsWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SShaderPipelineCacheToolsWidget)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	bool AreBundledPSOSettingsOk()const; 

protected: 
	UFUNCTION()
	FReply FixBundledPSOSettings();
	UFUNCTION()
	FReply HandlePipelineCacheSelection(); 

	UFUNCTION()
	FReply HandleShaderKeySelection(); 
	UFUNCTION()
	TSharedRef<ITableRow> GenerateRow(FName RowName,const TSharedRef<STableViewBase>& OwnerTable);
	UFUNCTION()
	TSharedPtr<SWidget> OnOpenContextMenu(TSharedPtr<SListView<FName>> listBox, TArray<FName>& ItemsSource);

	UFUNCTION()
	FReply CombinePSOFiles(); 

	void AddErrorMessage(const FString& ErrorText);
	virtual const FString VerifyFiles(TArray<FString>& Errors)const; 
	FString GetShaderFormatFromShaderKeyFile(FName StableShaderFile)const; 
	FString GetShaderFormatFromPipelinecacheFile(FName PipelineCacheFile)const;

	void CopyFile(const FString& BaseFileName, const FString& FromDirectory, const FString& ToDirectory, int Attempt = 0);
	void CopyFile(const FName& File, const FString& ToDirectory);

	void OpenFileDialog(const FString& DialogTitle, const FString& DefaultPath, const FString& FileTypes, TArray<FName>& SourceItems, const TSharedPtr<SListView<FName>>& ListView);
private: 
	const FString GetDefaultEngineIniName()const; 
	const TArray<FName> GetAllPlatforms()const;
	bool bPSOBundledSettingsOk; 
	FString Prefix;
	FText PSOBundledSettingsStatus;
	TArray<FName> Platforms;
	TArray<FName> ShaderKeyFiles;
	TArray<FName> PipelineCachesFiles;

	TSharedPtr<SComboBox<FName>> PlatformComboBox;
	TSharedPtr<SListView<FName>> PipelineCachesListbox;
	TSharedPtr<SListView<FName>> PipelineKeysListbox;
};
