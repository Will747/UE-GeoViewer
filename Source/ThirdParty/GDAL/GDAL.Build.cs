/*
	THIS FILE WAS GENERATED AUTOMATICALLY BY CONAN-UE4CLI VERSION 0.0.38.
	THIS BOILERPLATE CODE IS INTENDED FOR USE WITH UNREAL ENGINE VERSION 4.27 but modified to work
	without CONAN-UE4CLI on UE5.
*/
using System;
using System.IO;
using UnrealBuildTool;
using System.Collections.Generic;

public class GDAL : ModuleRules
{
	//Returns the identifier string for the given target, which includes its platform, architecture (if specified), and debug CRT status
	private string TargetIdentifier(ReadOnlyTargetRules target)
	{
		//Append the target's architecture to its platform name if an architecture was specified
		string id = (target.Architecture != null && target.Architecture.Length > 0) ?
			String.Format("{0}-{1}", target.Platform.ToString(), target.Architecture) :
			target.Platform.ToString();
		
		//Append a debug suffix for Windows debug targets that actually use the debug CRT
		bool isDebug = (target.Configuration == UnrealTargetConfiguration.Debug || target.Configuration == UnrealTargetConfiguration.DebugGame);
		if (isDebug && target.bDebugBuildsActuallyUseDebugCRT) {
			id += "-Debug";
		}
		
		return id;
	}
	
	//Determines if a target's platform is a Windows target platform
	private bool IsWindows(ReadOnlyTargetRules target) {
		return target.IsInPlatformGroup(UnrealPlatformGroup.Windows);
	}
	
	//Determines if we have precomputed dependency data for the specified target and Engine version, and processes it if we do
	private bool ProcessPrecomputedData(ReadOnlyTargetRules target, string stagingDir)
	{
		//Resolve the paths to the files and directories that will exist if we have precomputed data for the target
		string targetDir = Path.Combine(ModuleDirectory, "precomputed", this.TargetIdentifier(target));
		string flagsFile = Path.Combine(targetDir, "flags.json");
		string includeDir = Path.Combine(targetDir, "include");
		string libDir = Path.Combine(targetDir, "lib");
		string binDir = Path.Combine(targetDir, "bin");
		string dataDir = Path.Combine(targetDir, "data");
		
		//If any of the required files or directories do not exist then we do not have precomputed data
		if (!File.Exists(flagsFile) || !Directory.Exists(includeDir) || !Directory.Exists(libDir) || !Directory.Exists(binDir) || !Directory.Exists(dataDir)) {
			return false;
		}
		
		//Add the precomputed include directory to our search paths
		PublicIncludePaths.Add(includeDir);
		
		//Link against all static library files (and import libraries for DLLs under Windows) in the lib directory
		string libExtension = ((this.IsWindows(target)) ? ".lib" : ".a");
		string[] libs = Directory.GetFiles(libDir, "*" + libExtension);
		foreach(string lib in libs) {
			PublicAdditionalLibraries.Add(lib);
		}
		
		//Under non-Windows platforms, link against all shared library files in the lib directory
		if (this.IsWindows(target) == false)
		{
			List<string> sharedLibs = new List<string>();
			sharedLibs.AddRange(Directory.GetFiles(libDir, "*.dylib"));
			sharedLibs.AddRange(Directory.GetFiles(libDir, "*.so"));
			foreach(string lib in sharedLibs) {
				PublicAdditionalLibraries.Add(lib);
			}
		}
		
		//Ensure any shared libraries are staged alongside the binaries for the plugin
		string[] searchDirs = new string[]{ binDir, libDir };
		foreach (string dir in searchDirs)
		{
			List<string> binaries = new List<string>();
			binaries.AddRange(Directory.GetFiles(dir, "*.dll"));
			binaries.AddRange(Directory.GetFiles(dir, "*.dylib"));
			binaries.AddRange(Directory.GetFiles(dir, "*.so"));
			foreach (string binary in binaries) {
				RuntimeDependencies.Add(Path.Combine("$(BinaryOutputDir)", Path.GetFileName(binary)), binary, StagedFileType.NonUFS);
			}
		}
		
		//Link against any Unreal Engine modules for bundled third-party libraries
		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"libcurl",
        		"UElibPNG",
                "zlib"
			}
		);
		
		//Copy any data files needed by the package into our staging directory
		string[] files = Directory.GetFiles(dataDir, "*", SearchOption.AllDirectories);
		foreach(string file in files) {
			RuntimeDependencies.Add(Path.Combine(stagingDir, Path.GetFileName(file)), file, StagedFileType.NonUFS);
		}
		
		return true;
	}
	
	public GDAL(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		
		//Ensure our staging directory exists prior to copying any dependency data files into it
		string stagingDir = Path.Combine("$(ProjectDir)", "Binaries", "Data", "GDAL");
		if (!Directory.Exists(stagingDir)) {
			Directory.CreateDirectory(stagingDir);
		}
		
		//Determine if we have precomputed dependency data for the target that is being built
		if (ProcessPrecomputedData(Target, stagingDir) == false)
		{
			throw new Exception("Missing precomputed data!");
		}
	}
}
