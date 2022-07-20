using System.IO;
using UnrealBuildTool;

public class GDAL : ModuleRules
{
	public GDAL(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		
		var baseDir = Path.Combine(ModuleDirectory, "vcpkg-installed", "x64-windows-static-md");
		var includeDir = Path.Combine(baseDir, "include");
		var libDir = Path.Combine(baseDir, "lib");
		var shareDir = Path.Combine(baseDir, "share");

		//Add the precomputed include directory to our search paths
		PublicIncludePaths.Add(includeDir);
		
		var libs = Directory.GetFiles(libDir, "*.lib");
		foreach(var lib in libs) {
			PublicAdditionalLibraries.Add(lib);
		}

		//Link against any Unreal Engine modules for bundled third-party libraries
		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"libcurl",
				"zlib"
			}
		);
		
		//Copy any data files needed by the package into our staging directory
		string stagingDir = Path.Combine("$(ProjectDir)", "Binaries", "Data", "GDAL");
		if (!Directory.Exists(stagingDir)) {
			Directory.CreateDirectory(stagingDir);
		}
		
		var files = Directory.GetFiles(Path.Combine(shareDir, "gdal"), "*", SearchOption.AllDirectories);
		foreach(var file in files) {
			RuntimeDependencies.Add(Path.Combine(stagingDir, Path.GetFileName(file)), file, StagedFileType.NonUFS);
		}
	}
}
