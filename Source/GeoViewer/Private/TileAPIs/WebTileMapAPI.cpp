#include "TileAPIs/WebTileMapAPI.h"
#include "GDALWarp.h"

FWebMapTileAPI::FWebMapTileAPI(TWeakObjectPtr<UGeoViewerEdModeConfig> InEdModeConfig,
                               AWorldReferenceSystem* ReferencingSystem, FGeoBounds TileBounds) :
	FGeoTileAPI(InEdModeConfig, ReferencingSystem, TileBounds), ZoomLevel(0), TileResolution(0)
{
	SegmentNum = -1;
}

void FWebMapTileAPI::LoadTile()
{
	if (TileReferenceSystem)
	{
		//Get the bounds in projected coordinates
		FVector TopCornerProj;
		TileReferenceSystem->GeographicToProjectedWithEPSG(Bounds.TopLeft, TopCornerProj, EPSG);

		FVector BottomCornerProj;
		TileReferenceSystem->GeographicToProjectedWithEPSG(Bounds.BottomRight, BottomCornerProj, EPSG);

		if (TopCornerProj.Y <= BottomCornerProj.Y)
		{
			const double TempY = TopCornerProj.Y;
			TopCornerProj.Y = BottomCornerProj.Y;
			BottomCornerProj.Y = TempY;
		}

		if (TopCornerProj.X >= BottomCornerProj.X)
		{
			const double TempX = TopCornerProj.X;
			TopCornerProj.X = BottomCornerProj.X;
			BottomCornerProj.X = TempX;
		}
		
		//Download all segments needed till the 'CurrentPosition' is beyond the bottom corner
		FVector CurrentPosition = TopCornerProj;
		
		FVector2D PositionIndex = FVector2D(0, 0); //Position of a segment in relation to the other segments.
		while (CurrentPosition.Y > BottomCornerProj.Y)
		{
			PositionIndex.X = 0;
			FCartesianCoordinates ProjectedSegmentSize;
			while (CurrentPosition.X < BottomCornerProj.X)
			{
				FGeographicCoordinates SegmentCenter;
				TileReferenceSystem->ProjectedToGeographicWithEPSG(CurrentPosition, SegmentCenter, EPSG);

				//Calculate top corner position of tile
				const float HalfTileSize = CalculateTileSize(SegmentCenter.Latitude) / 2;
				
				FGeographicCoordinates SegmentTopCornerGeo;
				AGeoViewerReferenceSystem::GetGeographicalCoordinatesAtOffset(
					SegmentCenter,
					FVector2D(-HalfTileSize, HalfTileSize),
					SegmentTopCornerGeo
					);
				
				FVector SegmentTopCornerProj;
				TileReferenceSystem->GeographicToProjectedWithEPSG(SegmentTopCornerGeo, SegmentTopCornerProj, EPSG);

				// height in meters of the tile / number of vertical pixels
				//const double PixelSize =  TileSize / EdModeConfigPtr.Get()->TileSize;
				ProjectedSegmentSize.X = FMath::Abs(CurrentPosition.X - SegmentTopCornerProj.X) * 2;
				ProjectedSegmentSize.Y = FMath::Abs(CurrentPosition.Y - SegmentTopCornerProj.Y) * 2;

				FVector2D PixelSize;
				PixelSize.X = ProjectedSegmentSize.X / TileResolution;
				PixelSize.Y = ProjectedSegmentSize.Y / TileResolution;

				//Get URL and filename
				const FString URL = GetTileURL(SegmentCenter);
				const FString FileName = GetFileName(SegmentCenter);

				FString CacheFolder = GetCacheFolderPath();
				const bool bTileExists = FPaths::FileExists(CacheFolder + FileName + ".tif");

				if (bTileExists)
				{
					GDALDataset* Dataset = (GDALDataset*)GDALOpen( TCHAR_TO_UTF8(*(CacheFolder + FileName + ".tif")), GA_ReadOnly);
					DatasetsToMerge.Add(Dataset);
				} else
				{
					//Setup tile downloader
					const TSharedRef<FTileDownloader> Segment = MakeShared<FTileDownloader>();
					Segment->OnDownloaded.BindRaw(this, &FWebMapTileAPI::OnSegmentCompleted);
					SegmentsDownloaders.Add(Segment);
				
					//Update the dataset with new bounds
					Segment->SetMetaData(SegmentTopCornerProj, PixelSize, 3857);
					Segment->BeginDownload(URL, FileName);
				}
				
				
				//Move position along
				CurrentPosition.X += ProjectedSegmentSize.X;
				PositionIndex.X++;
			}
			
			//After each row reset X and increment Y
			CurrentPosition.X = TopCornerProj.X;
			CurrentPosition.Y -= ProjectedSegmentSize.Y;
			
			PositionIndex.Y++;
		}

		SegmentNum = PositionIndex.X * PositionIndex.Y;
	}

	CheckComplete();
}

void FWebMapTileAPI::CheckComplete()
{
	// See if all tiles have downloaded
	if (SegmentNum > 0 && DatasetsToMerge.Num() == SegmentNum)
	{
		// As the download has complete none of the downloaders are needed
		SegmentsDownloaders.Empty();
		
		// At this point all segments have been downloaded so the final dataset can be created.
		const int MergedDatasetIdx = CachedDatasets.Add(GDALDatasetRef(MergeDatasets()));
		
		const FString CurrentCRS = AGeoViewerReferenceSystem::EPSGToString(EPSG);
		const FString FinalCRS = TileReferenceSystem->ProjectedCRS;
		GDALDatasetRef WarpedDataset = FGDALWarp::WarpDataset(CachedDatasets[MergedDatasetIdx], CurrentCRS, FinalCRS);
		TriggerOnCompleted(WarpedDataset.Release());
	}
}

void FWebMapTileAPI::OnSegmentCompleted(const FTileDownloader* TileDownloader)
{
	DatasetsToMerge.Add(TileDownloader->FinalDataset);
	CheckComplete();
}

int FWebMapTileAPI::CalculateTileSize(double Latitude) const
{
	// https://docs.microsoft.com/en-us/bingmaps/articles/understanding-scale-and-resolution
	// Meters/Pixel
	const float PixelSize = (FMath::Cos(Latitude * PI / 180) * 156543.03392) / FMath::Pow(2.0f, ZoomLevel);
	
	return PixelSize * TileResolution;
}
