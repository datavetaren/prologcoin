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
   static private string theBoostDir = "";
   static private string thePlatformToolset;

   static private string MAIN_GUID = "7c9e667a-7425-30de-944b-f07fc1f90ae8";

   static private Guid CURRENT_GUID = new Guid(MAIN_GUID);

   static int[] byteOrder = { 15, 14, 13, 12, 11, 10, 9, 8, 6, 7, 4, 5, 0, 1, 2, 3 };
   static private Dictionary<string,string> theProjs = new Dictionary<string,string>();

   static Guid NextGuid()
   {
       var bytes = CURRENT_GUID.ToByteArray();
       var canIncrement = byteOrder.Any(i => ++bytes[i] != 0);
       var guid = new Guid(canIncrement ? bytes : new byte[16]);
       CURRENT_GUID = guid;
       return guid;
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

   internal static string getDir( string root, string targetType, string subdir )
   {
      string d = root + @"\" + targetType;
      if (subdir != null) d = d + @"\" + subdir;
      return d;
   }
   

   internal static void InitProject(Project project, string subdir, string what)
   {
       string root = theRootDir;

       string outDir = theOutDir;
       string binDir = theBinDir;

       var itemDefGroup = project.Xml.AddItemDefinitionGroup();
       var clDef = itemDefGroup.AddItemDefinition("ClCompile");
       string includeDirs = theBoostDir + ";" + "$(SolutionDir)" + MakeRelative(Path.GetFullPath(theSrcDir), theRootDir);

       clDef.AddMetadata("AdditionalIncludeDirectories", includeDirs);

       project.SetProperty("DefaultTargets", "Build");

       project.Xml.AddImport("$(VCTargetsPath)\\Microsoft.Cpp.Default.props");
       project.Xml.AddImport("$(VCTargetsPath)\\Microsoft.Cpp.props");
       project.Xml.AddImport("$(VCTargetsPath)\\Microsoft.Cpp.targets");

       var globalsGroup = project.Xml.AddPropertyGroup();
       globalsGroup.Label = "Globals";
       globalsGroup.AddProperty("ProjectGuid", NextGuid().ToString("B"));

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

       string name = subdir.Replace('\\', '_');

       var propDebug = project.Xml.AddPropertyGroup();
       propDebug.Condition = "'$(Configuration)|$(Platform)'=='Debug|Win32'";
       propDebug.Label = "Configuration";
       propDebug.SetProperty("ConfigurationType", what);
       propDebug.SetProperty("UseDebugLibraries", "true");
       propDebug.SetProperty("PlatformToolset", thePlatformToolset);
       propDebug.SetProperty("IntDir", @"$(SolutionDir)" + MakeRelative(Path.GetFullPath(getDir(outDir, "debug", subdir)), root) + @"\");
       propDebug.SetProperty("OutDir", @"$(SolutionDir)" + MakeRelative(Path.GetFullPath(getDir(binDir, "debug", null)), root) + @"\");
       propDebug.SetProperty("TargetName", name);

       if (what == "StaticLibrary") {
           propDebug.SetProperty("TargetExtension", "lib");
       } else {
           propDebug.SetProperty("TargetExtension", "exe");
       }
       

       // propDebug.SetProperty("CharacterSet", "MultiByte");

       var propRelease = project.Xml.AddPropertyGroup();
       propRelease.Condition = "'$(Configuration)|$(Platform)'=='Release|Win32'";
       propRelease.Label = "Configuration";
       propRelease.SetProperty("ConfigurationType", what);
       propRelease.SetProperty("UseDebugLibraries", "false");
       propRelease.SetProperty("PlatformToolset", thePlatformToolset);
       propRelease.SetProperty("IntDir", @"$(SolutionDir)" + MakeRelative(Path.GetFullPath(getDir(outDir, "release", subdir)), root) + @"\");
       propRelease.SetProperty("OutDir", @"$(SolutionDir)" + MakeRelative(Path.GetFullPath(getDir(binDir, "release", null)), root) + @"\");
       propRelease.SetProperty("TargetName", name);

       if (what == "StaticLibrary") {
           propRelease.SetProperty("TargetExtension", "lib");
       } else {
           propRelease.SetProperty("TargetExtension", "exe");
       }

       // propRelease.SetProperty("CharacterSet", "MultiByte");
   }

   public static void CreateLibProject(Project project, string subdir)
   {
       InitProject(project, subdir, "StaticLibrary");

       string srcDir = theSrcDir;
       string [] srcFiles = Directory.GetFiles(srcDir + @"\" + subdir);
       foreach (string srcFile in srcFiles) {
           var ext = Path.GetExtension(srcFile);
	   string filePath = Path.GetFullPath(srcFile);
	   if (ext == ".cpp") {
	       filePath = MakeRelative(filePath, theRootDir);
	       project.AddItem("ClCompile", @"$(SolutionDir)" + filePath);
	   } else if (ext == ".hpp" || ext == ".h") {
	       filePath = MakeRelative(filePath, theRootDir);
	       project.AddItem("ClInclude", @"$(SolutionDir)" + filePath);
	   }
       }
   }

   public static void CreateUnittestProject(Project project,
					    string subdir,
                                            string [] srcFiles,
   	  	      		            string libName)
   {
       InitProject(project, subdir, "Application");

       string name = subdir.Replace(@"\", "_");

       string mandatoryImportLib = @"$(OutDir)" + libName + ".lib";

       string unittestMainPre = "";
       string unittestMainBody = "";

       foreach (var srcFile in srcFiles) {
           string filePath = Path.GetFullPath(srcFile);

	   if (Path.GetExtension(srcFile) != ".cpp") {
	      continue;
 	   }

           string srcFilePath = @"$(SolutionDir)" + MakeRelative(filePath, theRootDir);
	   string mainFun = "main_" + Path.GetFileNameWithoutExtension(srcFile);

           ProjectItem item = project.AddItem("ClCompile", srcFilePath)[0];
	   item.Xml.AddMetadata("PreprocessorDefinitions", "main=" + mainFun);
	   unittestMainBody += "    " + mainFun + "(argc, argv);\n";
	   unittestMainPre += "int " + mainFun + "(int argc, char *argv[]);\n";
       }

       string unittestMain = unittestMainPre + "\n"
          + "int main(int argc, char *argv[])\n"
	  + "{\n"
	  + unittestMainBody
	  + "}\n";

       // Write this file and add it
       string unittestMainPath = Path.GetFullPath(theBinDir + @"\" + name + "_main.cpp");
       string unittestMainPath1 = @"$(SolutionDir)" + MakeRelative(unittestMainPath, theRootDir);
       var unittestMainFile = new StreamWriter(unittestMainPath);
       unittestMainFile.Write(unittestMain);
       unittestMainFile.Close();

       project.AddItem("ClCompile", unittestMainPath1);

       project.AddItem("Link", mandatoryImportLib);

       var itemDefGroup = project.Xml.AddItemDefinitionGroup();
       var clDef = itemDefGroup.AddItemDefinition("ClCompile");
       clDef.AddMetadata("DebugInformationFormat", "ProgramDatabase");
       clDef.AddMetadata("ProgramDataBaseFileName", @"$(OutDir)" + name + ".pdb");

       var linkDefGroup = project.Xml.AddItemDefinitionGroup();
       var linkDef = linkDefGroup.AddItemDefinition("Link");
       linkDef.Condition =  "'$(Configuration)|$(Platform)'=='Debug|Win32'";
       linkDef.AddMetadata("GenerateDebugInformation", "true");
       linkDef.AddMetadata("SubSystem", "Console");

       var propDebug = project.Xml.AddPropertyGroup();
       propDebug.Condition = "'$(Configuration)|$(Platform)'=='Debug|Win32'";
       propDebug.SetProperty("BuildLinkTargets", "");

       /*
       var custStepDep = project.Xml.AddPropertyGroup();
       custStepDep.SetProperty("CustomBuildAfterTargets", "Link");
       */
   }

   private static void processDirectory(string srcDir, string subdir, bool isTest)
   {
       Console.WriteLine("Process " + subdir);

       string name = subdir.Replace(@"\", "_");

       if (!isTest) {
          //
          // Library project
	  //
       	  Project libProject = new Project();
       	  CreateLibProject(libProject, subdir);
	  string libFile = name;
       	  Console.WriteLine("Saving " + (theBinDir + @"\" + libFile + ".vcxproj"));
       	  libProject.Save(theBinDir + @"\" + libFile + ".vcxproj", Encoding.ASCII);
          // projs.Add(libFile, NextGuid());
       } else {
          //
       	  // Unittest project
          // 
	  string importLibName = subdir.Substring(0, subdir.Length - 5);
          string [] srcFiles = Directory.GetFiles(srcDir + @"\" + subdir);
	  Project unittestProject = new Project();
          CreateUnittestProject(unittestProject, subdir,
	  			srcFiles, importLibName);
          var projFile = theBinDir + @"\" + name + ".vcxproj";
       	  Console.WriteLine("Saving " + projFile);
       	  unittestProject.Save(projFile, Encoding.ASCII);

	   /*
          foreach (var srcFile in srcFiles) {
	      string testName = subdir.Replace("\", "_") + "_" + Path.GetFileNameWithoutExtension(srcFile);
	      unittestGuid = NextGuid();
	      theProjs.Add(testName, unittestGuid.ToString("B"));
	   */
       }
   }

   public static void Main(string [] args)
   {
       string srcDir = "";
       string outDir = "";
       string binDir = "";
       string env = "";
       string boost = "";
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
	   if (x.StartsWith("boost=")) {
	      boost = x.Substring(6);
	   }
       }
       Console.WriteLine("srcDir=" + srcDir);
       Console.WriteLine("outDir=" + outDir);
       Console.WriteLine("binDir=" + binDir);
       Console.WriteLine("   env=" + env);
       Console.WriteLine("boost =" + boost);

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
       theBoostDir = boost;

       // Iterate through directories from src

       Stack<string> stack = new Stack<string>();
       stack.Push(srcDir);
       while (stack.Count() > 0) {
       	     string dir = stack.Pop();
             string [] cppFiles = Directory.GetFiles(dir, "*.cpp");
	     bool isTestDir = false;
	     if (cppFiles.Count() > 0) {
	         if (dir.EndsWith(@"\test")) {
		     isTestDir = true;
		 }
    	         string subdir = MakeRelative(dir, srcDir);
	         processDirectory(srcDir, subdir, isTestDir);
             }

	     string [] subdirs = Directory.GetDirectories(dir);
	     foreach (var subdir in subdirs) {
	         stack.Push(subdir);
	     }
       }
   }
}