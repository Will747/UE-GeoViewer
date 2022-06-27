#include "SWeightMapImportDlg.h"

#include "DesktopPlatformModule.h"
#include "EditorDirectories.h"
#include "IDesktopPlatform.h"
#include "Misc/FileHelper.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/SRichTextBlock.h"

#define LOCTEXT_NAMESPACE "GeoViewerWeightMapDlg"

void SWeightMapImportDlg::Construct(
	const FArguments& InArgs,
	TSharedPtr<SWindow> InParentWindow,
	const FGeoBounds& InLandscapeBounds
	)
{
	ParentWindow = InParentWindow;
	LandscapeBounds = InLandscapeBounds;

	FilesListBox = SNew(SVerticalBox);
	RefreshFilesList();
	
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)

			+SVerticalBox::Slot()
			[
				SNew(SHorizontalBox)
				
				+SHorizontalBox::Slot() // Weight map section
				.Padding(5)
				[
					SNew(SVerticalBox)

					+SVerticalBox::Slot()
					.VAlign(VAlign_Center)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.Text(LOCTEXT("SelectWeightMapButton", "Select Weight Maps"))
						.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
						.OnClicked(this, &SWeightMapImportDlg::OnSelectWeightMapClicked)
					]

					+SVerticalBox::Slot()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("WeightMapListTitle", "Weight maps:"))
					]
						
					+SVerticalBox::Slot()
					.FillHeight(4.f)
					[
						SNew(SBorder)
						[
							SNew(SScrollBox)
							+SScrollBox::Slot()
							[
								FilesListBox.ToSharedRef()
							]
						]
					]
				]

				+SHorizontalBox::Slot() // Bounds section
				.Padding(5)
				[
					SNew(SVerticalBox)

					+SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SBorder)
						.Padding(5)
						[
							SNew(SVerticalBox)

							+SVerticalBox::Slot()
							[
								SNew(STextBlock)
								.Text(LOCTEXT("LandscapeBoundsText", "Landscape Bounds"))
							]

							+SVerticalBox::Slot()
							[
								SNew(STextBlock)
								.Text(
									FText::Format(LOCTEXT("LandscapeTopLeftText", "Top Left - {0}"),
										GetCoordinateText(LandscapeBounds.TopLeft)))
							]
							
							+SVerticalBox::Slot()
							[
								SNew(STextBlock)
								.Text(
									FText::Format(
										LOCTEXT("LandscapeBottomRightText", "Bottom Right - {0}"),
										GetCoordinateText(LandscapeBounds.BottomRight)))
							]
						]
					]
					
					+SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SButton)
						.Text(LOCTEXT("SaveGeoJsonButton", "Save landscape GeoJson file"))
						.OnClicked(this, &SWeightMapImportDlg::OnSaveGeoJsonClicked)
					]
				]
			]

			+SVerticalBox::Slot()
			.HAlign(HAlign_Right)
			.AutoHeight()
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.Text(this, &SWeightMapImportDlg::GetContinueText)
				.OnClicked(this, &SWeightMapImportDlg::OnContinueClicked)
			]
		]
	];
}

FText SWeightMapImportDlg::GetCoordinateText(FGeographicCoordinates Coordinates) const
{
	TArray<FText> Args;
	Args.Init(FText(), 2);
	FText Altitude;
	Coordinates.ToSeparateTexts(Args[0], Args[1], Altitude);
	
	return FText::Join(LOCTEXT("CoordinateComma", ", "), Args);
}

void SWeightMapImportDlg::RefreshFilesList()
{
	bool bFilesEmpty = false;
	if (Files.Num() == 0)
	{
		Files.Add(TEXT("None"));
		bFilesEmpty = true;
	}
	
	SVerticalBox* List = FilesListBox.Get();
	const int NumOfSlots = List->NumSlots();
	const int NumOfFiles = Files.Num();
	
	TArray<TSharedPtr<SWidget>> WidgetsToRemove;

	// Update existing slots with paths
	for (int i = 0; i < NumOfSlots; i++)
	{
		TSharedPtr<SWidget> SlotWidget = List->GetSlot(i).GetWidget();
		
		if (i >= NumOfFiles)
		{
			WidgetsToRemove.Add(SlotWidget);
		} else
		{
			TSharedPtr<STextBlock> TextWidget = StaticCastSharedPtr<STextBlock>(SlotWidget);

			if (TextWidget.IsValid())
			{
				TextWidget.Get()->SetText(FText::FromString(Files[i]));
			}
		}
	}

	if (WidgetsToRemove.Num() == 0)
	{
		// Add additional slots
		for (int i = NumOfSlots; i < NumOfFiles; i++)
		{
			TSharedRef<STextBlock> TextWidget = SNew(STextBlock)
				.Text(FText::FromString(Files[i]));

			List->AddSlot()
			[
				TextWidget
			];
		}
	} else
	{
		// Remove no longer needed slots
		for (TSharedPtr<SWidget> Widget : WidgetsToRemove)
		{
			List->RemoveSlot(Widget.ToSharedRef());
		}
	}

	if (bFilesEmpty)
	{
		Files.Empty();
	}
}

FText SWeightMapImportDlg::GetContinueText() const
{
	if (Files.Num() <= 0)
	{
		return LOCTEXT("ContinueButtonNoWeightMap", "Continue without weight maps");
	}

	return LOCTEXT("ContinueButton", "Continue");
}

FReply SWeightMapImportDlg::OnSelectWeightMapClicked()
{
	// Open file dialog
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		if (ParentWindow->GetNativeWindow().IsValid())
		{
			const void* ParentWindowHandle = ParentWindow->GetNativeWindow()->GetOSWindowHandle();

			const FString FileType = TEXT("Geotiff Weight Map|*.tif");

			Files.Empty();
			DesktopPlatform->OpenFileDialog(
				ParentWindowHandle,
				LOCTEXT("WeightMapOpenDialogTitle", "Select weightmaps").ToString(),
				*FEditorDirectories::Get().GetLastDirectory(ELastDirectory::UNR),
				TEXT(""),
				FileType,
				EFileDialogFlags::Multiple,
				Files);

			RefreshFilesList();
		}
	}
	
	return FReply::Handled();
}

FReply SWeightMapImportDlg::OnContinueClicked() const
{
	ParentWindow->RequestDestroyWindow();

	return FReply::Handled();
}

FReply SWeightMapImportDlg::OnSaveGeoJsonClicked() const
{
	// Open save file dialog
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		if (ParentWindow->GetNativeWindow().IsValid())
		{
			const void* ParentWindowHandle = ParentWindow->GetNativeWindow()->GetOSWindowHandle();

			const FString FileType = TEXT("GeoJSON|*.geojson");

			TArray<FString> GeoJsonFilePaths;
			DesktopPlatform->SaveFileDialog(
				ParentWindowHandle,
				LOCTEXT("GeoJsonSaveDialogTitle", "Save landscape bounds").ToString(),
				*FEditorDirectories::Get().GetLastDirectory(ELastDirectory::UNR),
				TEXT(""),
				FileType,
				EFileDialogFlags::None,
				GeoJsonFilePaths);

			if (GeoJsonFilePaths.Num() == 1)
			{
				const FString GeoJsonString = LandscapeBounds.GetGeoJson();
				FFileHelper::SaveStringToFile(GeoJsonString, *GeoJsonFilePaths[0]);
			}
		}
	}
	
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
