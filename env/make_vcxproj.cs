using System.IO;
using System.Text;
using System;
using System.Linq;
using System.Collections.Generic;
using Microsoft.Build.Construction;
using Microsoft.Build.Evaluation;

public class make_vcproj
{
   static private string theRootDir;
   static private string theSrcDir;
   static private string theOutDir;
   static private string theBinDir;
   static private string thePlatformToolset;

   static private string PROJECT = "omegal";
   static private string MAIN_GUID = "7c9e6679-7425-30de-944b-f07fc1f90ae8";
   static private string UNITTEST_GUID = "7c9e6679-7425-30de-944b-f07fc1f90ae9";

   static int[] byteOrder = { 15, 14, 13, 12, 11, 10, 9, 8, 6, 7, 4, 5, 0, 1, 2, 3 };

   static Guid NextGuid(Guid guid)
   {
       var bytes = guid.ToByteArray();
       var canIncrement = byteOrder.Any(i => ++bytes[i] != 0);
       return new Guid(canIncrement ? bytes : new byte[16]);
   }

   internal static string MakeRelative(string filePath, string referencePath)
   {
       var filePathParts = filePath.Split('\\');
       var referencePathParts = referencePath.Split('\\');

       int i;       
       for (i = 0; i < filePathParts.Length-1 &&
       	           i < referencePathParts.Length &&
		   filePathParts[i] == referencePathParts[i]; i++) { 
       }

       // Number of walk ups
       string relPath = "";
       for (var j = i; j < referencePathParts.Length; j++) {
           relPath += @"..\";
       }

       for (var j = i; j < filePathParts.Length; j++) {
           if (j > i) relPath += @"\";
       	   relPath += filePathParts[j];
       }

       return relPath;
   }

   static readonly Guid VC_CPP_PROJECT_GUID = Guid.Parse("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}");

   internal static string GetPlatformToolset(string str)
   {
       if (!str.StartsWith("VS")) {
           return "";
       }
       // Extract the following digits
       string digits = "";
       for (var i = 2; i < str.Length; i++) {
           if (Char.IsNumber(str[i])) {
	       digits += str[i];
	   }
       }
       str = "v" + digits;
       return str;
   }

   internal static void InitProject(Project project, string subdir)
   {
       string root = theRootDir;

       string outDir = theOutDir;
       if (subdir != null) outDir += @"\" + subdir;
       string binDir = theBinDir;
       if (subdir != null) binDir += @"\" + subdir;

       project.SetProperty("DefaultTargets", "Build");

       project.Xml.AddImport("$(VCTargetsPath)\\Microsoft.Cpp.Default.props");
       project.Xml.AddImport("$(VCTargetsPath)\\Microsoft.Cpp.props");
       project.Xml.AddImport("$(VCTargetsPath)\\Microsoft.Cpp.targets");

       var globalsGroup = project.Xml.AddPropertyGroup();
       globalsGroup.Label = "Globals";
       globalsGroup.AddProperty("ProjectGuid", VC_CPP_PROJECT_GUID.ToString("B"));

       //
       // Project configurations (debug | release)
       //

       var projConfigs = project.Xml.AddItemGroup();
       projConfigs.Label = "ProjectConfigurations";

       var debugConfig = projConfigs.AddItem("ProjectConfiguration",
					     "Debug|Win32");
       debugConfig.AddMetadata("Configuration", "Debug");
       debugConfig.AddMetadata("Platform", "Win32");

       var releaseConfig = projConfigs.AddItem("ProjectConfiguration",
					       "Release|Win32");
       releaseConfig.AddMetadata("Configuration", "Release");
       releaseConfig.AddMetadata("Platform", "Win32");

       //
       // Configurations (debug | release) applications
       //

       var confType = (subdir != null) ? "Application" : "StaticLibrary";

       var propDebug = project.Xml.AddPropertyGroup();
       propDebug.Condition = "'$(Configuration)|$(Platform)'=='Debug|Win32'";
       propDebug.Label = "Configuration";
       propDebug.SetProperty("ConfigurationType", confType);
       propDebug.SetProperty("UseDebugLibraries", "true");
       propDebug.SetProperty("PlatformToolset", thePlatformToolset);
       propDebug.SetProperty("IntDir", @"$(SolutionDir)" + MakeRelative(Path.GetFullPath(outDir) + @"\debug\", root));
       propDebug.SetProperty("OutDir", @"$(SolutionDir)" + MakeRelative(Path.GetFullPath(binDir) + @"\debug\", root));
       propDebug.SetProperty("MainOutDir", @"$(SolutionDir)" + MakeRelative(Path.GetFullPath(theBinDir) + @"\debug\", root));
       // propDebug.SetProperty("CharacterSet", "MultiByte");

       var propRelease = project.Xml.AddPropertyGroup();
       propRelease.Condition = "'$(Configuration)|$(Platform)'=='Release|Win32'";
       propRelease.Label = "Configuration";
       propDebug.SetProperty("ConfigurationType", confType);
       propRelease.SetProperty("UseDebugLibraries", "false");
       propRelease.SetProperty("PlatformToolset", thePlatformToolset);
       propRelease.SetProperty("IntDir", @"$(SolutionDir)" + MakeRelative(Path.GetFullPath(outDir) + @"\release\", root));
       propRelease.SetProperty("OutDir", @"$(SolutionDir)" + MakeRelative(Path.GetFullPath(binDir) + @"\release\", root));
       propRelease.SetProperty("MainOutDir", @"$(SolutionDir)" + MakeRelative(Path.GetFullPath(theBinDir) + @"\release\", root));

       // propRelease.SetProperty("CharacterSet", "MultiByte");
   }

   public static void CreateMainProject(Project project)
   {
       InitProject(project, null);

       string srcDir = theSrcDir;
       string [] srcFiles = Directory.GetFiles(srcDir);
       foreach (string srcFile in srcFiles) {
           var ext = Path.GetExtension(srcFile);
	   string filePath = Path.GetFullPath(srcFile);
	   if (ext == ".cpp") {
	       filePath = MakeRelative(filePath, theRootDir);
	       project.AddItem("ClCompile", @"$(SolutionDir)\" + filePath);
	   } else if (ext == ".hpp" || ext == ".h") {
	       filePath = MakeRelative(filePath, theRootDir);
	       project.AddItem("ClInclude", @"$(SolutionDir)\" + filePath);
	   }
       }
   }

   public static void CreateUnittestProject(Project project, string srcFile)
   {
       InitProject(project, Path.GetFileNameWithoutExtension(srcFile));

       string libFile = @"$(MainOutDir)" + PROJECT + ".lib";

       string filePath = Path.GetFullPath(srcFile);
       string srcFilePath = MakeRelative(filePath, theRootDir);
       string objFilePath = @"$(IntDir)\" + Path.GetFileNameWithoutExtension(srcFile) + ".obj";
       string exeFilePath = @"$(OutDir)\" + Path.GetFileNameWithoutExtension(srcFile) + ".exe";

       project.AddItem("ClCompile", @"$(SolutionDir)\" + srcFilePath);
       project.AddItem("Link", libFile);

       var custBuild = project.Xml.AddItemDefinitionGroup();
       var custBuild1 = custBuild.AddItemDefinition("CustomBuildStep");
       custBuild1.AddMetadata("Command", exeFilePath + " >" + exeFilePath + ".log & echo ok >" + exeFilePath + ".ok");
       custBuild1.AddMetadata("Outputs", exeFilePath + ".ok");

       var itemDefGroup = project.Xml.AddItemDefinitionGroup();
       var clDef = itemDefGroup.AddItemDefinition("ClCompile");
       clDef.AddMetadata("AdditionalIncludeDirectories",
		@"$(SolutionDir)\" + MakeRelative(Path.GetFullPath(theSrcDir), theRootDir));

       clDef.AddMetadata("DebugInformationFormat", "ProgramDatabase");
       clDef.AddMetadata("ProgramDataBaseFileName", @"$(OutDir)\" + Path.GetFileNameWithoutExtension(srcFile) + ".pdb");

       var linkDefGroup = project.Xml.AddItemDefinitionGroup();
       var linkDef = linkDefGroup.AddItemDefinition("Link");
       linkDef.Condition =  "'$(Configuration)|$(Platform)'=='Debug|Win32'";
       linkDef.AddMetadata("GenerateDebugInformation", "true");

       var propDebug = project.Xml.AddPropertyGroup();
       propDebug.Condition = "'$(Configuration)|$(Platform)'=='Debug|Win32'";
       propDebug.SetProperty("BuildLinkTargets", "");

       var custStepDep = project.Xml.AddPropertyGroup();
       custStepDep.SetProperty("CustomBuildAfterTargets", "Link");
   }

   public static void Main(string [] args)
   {
       string srcDir = "";
       string outDir = "";
       string binDir = "";
       string env = "";
       Console.WriteLine("make_vcproj");
       foreach (string x in args) {
           if (x.StartsWith("src=")) {
	       srcDir = x.Substring(4);
	   }
	   if (x.StartsWith("out=")) {
	       outDir = x.Substring(4);
	   }
	   if (x.StartsWith("bin=")) {
	       binDir = x.Substring(4);
	   }
	   if (x.StartsWith("env=")) {
	      env = x.Substring(4);
	   }
       }
       Console.WriteLine("srcDir=" + srcDir);
       Console.WriteLine("outDir=" + outDir);
       Console.WriteLine("binDir=" + binDir);
       Console.WriteLine("   env=" + env);

       Directory.CreateDirectory(binDir);
       Directory.CreateDirectory(binDir + @"\test");

       thePlatformToolset = GetPlatformToolset(env);
       if (thePlatformToolset.Length == 0) {
           Console.WriteLine("Could not determine Visual Studio Version");
	   return;
       }
       String root = Path.GetFullPath(binDir);

       theRootDir = root;
       theSrcDir = srcDir;
       theOutDir = outDir;
       theBinDir = binDir;

       //
       // Standard project
       //
       Project mainProject = new Project();
       CreateMainProject(mainProject);
       Console.WriteLine("Saving " + (binDir + @"\" + PROJECT + ".vcxproj"));
       mainProject.Save(binDir + @"\" + PROJECT + ".vcxproj", Encoding.ASCII);

       //
       // Unittest project
       //
       Guid unittestGuid = new Guid(UNITTEST_GUID);
       string [] srcFiles = Directory.GetFiles(srcDir + @"\test\");
       var unittestGuids = new Dictionary<string,string>();
       foreach (var srcFile in srcFiles) {
           string testName = Path.GetFileNameWithoutExtension(srcFile);
	   unittestGuid = NextGuid(unittestGuid);
	   unittestGuids.Add(testName, unittestGuid.ToString("B"));
           Project unittestProject = new Project();
           CreateUnittestProject(unittestProject, srcFile);
	   var projFile = binDir + @"\test\" + testName + ".vcxproj";
       	   Console.WriteLine("Saving " + projFile);
       	   unittestProject.Save(projFile, Encoding.ASCII);
       }

       //
       // Solution file
       //

       Console.WriteLine("Saving " + binDir + @"\" + PROJECT + ".sln");
       var sw = new StreamWriter(binDir + @"\" + PROJECT + ".sln");
       sw.WriteLine("Microsoft Visual Studio Solution File, Format Version 12.00");
       sw.WriteLine("# Visual Studio 2013");
       sw.WriteLine("VisualStudioVersion = 12.0.31101.0");
       sw.WriteLine("MinimumVisualStudioVersion = 10.0.40219.1");

       sw.WriteLine("Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"" + PROJECT + "\", \"" + PROJECT + ".vcxproj\", \"{" + MAIN_GUID + "}\"");
       sw.WriteLine("EndProject");

       foreach (var entry in unittestGuids) {
           string testName = entry.Key;
           string testGuid = entry.Value;
	   var projFile = @"test\" + testName + ".vcxproj";
           sw.WriteLine("Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"" + PROJECT+"_unittest_"+testName+"\", \"" + projFile + "\", \"" + testGuid + "\"");
           sw.WriteLine("    ProjectSection(ProjectDependencies) = postProject");
           sw.WriteLine("        {" + MAIN_GUID + "} = {" + MAIN_GUID + "}");
           sw.WriteLine("    EndProjectSection");
           sw.WriteLine("EndProject");
       }
       

       sw.WriteLine("");

       sw.WriteLine("Global");

       sw.WriteLine("GlobalSection(SolutionConfigurationPlatforms) = preSolution");
       sw.WriteLine("Debug|Win32 = Debug|Win32");
       sw.WriteLine("Release|Win32 = Release|Win32");
       sw.WriteLine("EndGlobalSection");

       sw.WriteLine("");

       sw.WriteLine("GlobalSection(SolutionProperties) = preSolution");
       sw.WriteLine("    HideSolutionNode = FALSE");
       sw.WriteLine("EndGlobalSection");
       
       sw.WriteLine("");

       sw.WriteLine("EndGlobal");

       sw.Close();

   }
}