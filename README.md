# UE-GeoViewer
### This plugin is still being worked on and not complete.
A plugin for Unreal Engine that overlays real world maps into the world making it easier to precisely place items in the world and imports real world terrain. 

![Placing A tree with the overlay active](docs/UE5Overlay.png)

Using the built in GeoReferencing plugin means the maps can be used on many different projections depending on the place being created in the world and the maps are reprojected to the projected CRS using GDAL.

## Features
- Supports both Google and Bing maps.
- Caches downloaded images to prevent unnecessary API requests.
- Supports any EPSG projection.
- Contains many options such as overlay type, resolution, max number of tiles/decals, decal size.
- Stores settings and API keys in the Editor.ini file.
- Displays current geographical coordinates.
- Imports terrain data in the STRM HGT format.

## Requirements
- UE5 with the GeoReferencing plugin enabled.

## Installation
1. Download or Clone this repository.
2. Copy the folder 'GeoViewer' into the plugins folder of the project.
3. Launch the project, before it starts it should come up with a warning about missing modules and the question, "Would you like to rebuild them now?". Select 'Yes' to this and the plugin should get compiled. This may take a few minutes.
4. Afterwards the editor should launch as normal, and the plugins should be installed.

## Usage
To get started open the 'Geo Viewer' editor mode. This will add two new actors to the world. The first is named 'WorldReferenceSystem', this is used to convert between engine coordinates and geographical coordinates. The second is named 'MapOverlayActor' this will hold the decals that get added when the overlay is active.

To setup the world origin and projection select the 'WorldReferenceSystem' actor in the details panel some options should appear:
- Planet Shape - At the moment the overlay only supports 'Flat Planet'
- Projected CRS - This can be changed to a suitable projection for the area the world represents to minimise distortion. [EPSG.io](https://epsg.io/) is a useful website when looking for a CRS to use.
- Geographic CRS - This should be left as 'EPSG:4326'.
- Origin Location in Projected - Should be set to false, the origin must be in Longitude and Latitude.
- Origin Latitude and Origin Longitude - This can be set based on the location the map is based.

![World Overlay System Details Panel](docs/WorldReferenceSystem.png)

### Overlay
Then the API keys need adding for either Google or Bing maps, these appear in the Geo Viewer editor mode panel when 'Show API Key' is set to true. The rest of the settings should be fine left with the default values.
'Overlay System' can be changed to select which API to use.

To activate the overlay, press the 'Activate Overlay' button at the top of the panel. This button acts as a toggle so pressing it again will deactivate the overlay.

![Bing Maps in 'Canvas Dark' mode](docs/BingCanvasDarkMode.png)

### Terrain
Currently elevation data in the HGT STRM format can be imported. All '.hgt' files should be placed in `Resources\Terrain\HGT`. Then in the editor there is a 'Load Terrain' button at the top of the GeoViewer editor panel. This will create landscape tiles based on the current geographic position in the viewport. Landscape tiles can be added gradually as the world gets created, but the '.hgt' files must keep their original filename which relates to their geographic position e.g. N00W000.hgt
