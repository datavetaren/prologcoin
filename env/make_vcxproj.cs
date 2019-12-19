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
   static private string theBit = "32";

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
   

   internal static void InitProject(Project project, string moddir, string subdir, string name, string what)
   {
       string slnRoot = Path.GetFullPath(theBinDir);

       string outDir = theOutDir;
       string binDir = theBinDir;

       var itemDefGroup = project.Xml.AddItemDefinitionGroup();
       var clDef = itemDefGroup.AddItemDefinition("ClCompile");

       var includeDirs = "";
       List<string> additionalIncDirs = GetAdditionalIncludeDirs(moddir);
       foreach (var incDir in additionalIncDirs) {
       	   if (includeDirs.Length != 0) includeDirs += ";";
           includeDirs += incDir;
       }
       if (includeDirs.Length != 0) includeDirs += ";";
       includeDirs += theBoostDir + ";" + "$(SolutionDir)" + MakeRelative(Path.GetFullPath(theSrcDir), slnRoot);

       clDef.AddMetadata("AdditionalIncludeDirectories", includeDirs);

       clDef.AddMetadata("PreprocessorDefinitions", "_WIN32_WINNT=0x0501");

       string ccExtra = GetWinCCExtra(subdir);
       if (ccExtra != null) {
           clDef.AddMetadata("AdditionalOptions", ccExtra);
       }

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

       var platform = (theBit == "64") ? "x64" : "Win32";

       var debugConfig = projConfigs.AddItem("ProjectConfiguration",
					     "Debug|" + platform);
       debugConfig.AddMetadata("Configuration", "Debug");
       debugConfig.AddMetadata("Platform", platform);


       var releaseConfig = projConfigs.AddItem("ProjectConfiguration",
					       "Release|" + platform);
       releaseConfig.AddMetadata("Configuration", "Release");
       releaseConfig.AddMetadata("Platform", platform);

       //
       // Configurations (debug | release) applications
       //

       var propDebug = project.Xml.AddPropertyGroup();
       propDebug.Condition = "'$(Configuration)|$(Platform)'=='Debug|" + platform + "'";
       propDebug.Label = "Configuration";
       propDebug.SetProperty("ConfigurationType", what);
       propDebug.SetProperty("UseDebugLibraries", "true");
       propDebug.SetProperty("PlatformToolset", thePlatformToolset);
       propDebug.SetProperty("IntDir", @"$(SolutionDir)" + MakeRelative(Path.GetFullPath(getDir(outDir, "debug", subdir)), slnRoot) + @"\");
       propDebug.SetProperty("OutDir", @"$(SolutionDir)" + MakeRelative(Path.GetFullPath(getDir(binDir, "debug", null)), slnRoot) + @"\");
       propDebug.SetProperty("TargetName", name);

       if (what == "StaticLibrary") {
           propDebug.SetProperty("TargetExtension", "lib");
       } else {
           propDebug.SetProperty("TargetExtension", "exe");
       }
       

       // propDebug.SetProperty("CharacterSet", "MultiByte");

       var propRelease = project.Xml.AddPropertyGroup();
       propRelease.Condition = "'$(Configuration)|$(Platform)'=='Release|" + platform + "'";
       propRelease.Label = "Configuration";
       propRelease.SetProperty("ConfigurationType", what);
       propRelease.SetProperty("UseDebugLibraries", "false");
       propRelease.SetProperty("PlatformToolset", thePlatformToolset);
       propRelease.SetProperty("IntDir", @"$(SolutionDir)" + MakeRelative(Path.GetFullPath(getDir(outDir, "release", subdir)), slnRoot) + @"\");
       propRelease.SetProperty("OutDir", @"$(SolutionDir)" + MakeRelative(Path.GetFullPath(getDir(binDir, "release", null)), slnRoot) + @"\");
       propRelease.SetProperty("TargetName", name);

       if (what == "StaticLibrary") {
           propRelease.SetProperty("TargetExtension", "lib");
       } else {
           propRelease.SetProperty("TargetExtension", "exe");
       }

       // propRelease.SetProperty("CharacterSet", "MultiByte");
   }

   public static void AddSourceFiles(Project project, string subdir)
   {
       string slnRoot = Path.GetFullPath(theBinDir);
       string srcDir = theSrcDir;
       string [] srcFiles = Directory.GetFiles(srcDir + @"\" + subdir);
       foreach (string srcFile in srcFiles) {
           var ext = Path.GetExtension(srcFile);
	   string filePath = Path.GetFullPath(srcFile);
	   if (ext == ".cpp") {
	       filePath = MakeRelative(filePath, slnRoot);
	       project.AddItem("ClCompile", @"$(SolutionDir)" + filePath);
	   } else if (ext == ".hpp" || ext == ".h") {
	       filePath = MakeRelative(filePath, slnRoot);
	       project.AddItem("ClInclude", @"$(SolutionDir)" + filePath);
	   }
       }
   }

   public static void CreateLibProject(Project project, string subdir)
   {
       InitProject(project, subdir, subdir, subdir, "StaticLibrary");
       AddSourceFiles(project, subdir);
   }

   public static void CreateExeProject(Project project, string subdir, string name)
   {
       InitProject(project, subdir, subdir, name, "Application");
       AddSourceFiles(project, subdir);

       // Get depending libs from parent Makefile.env
       List<string> importLibs = GetDependingLibs(subdir);
       SetupApplication(project, name, importLibs);

       string args = GetDefaultArgs(subdir);
       if (args != null) {
           var platform = (theBit == "64") ? "x64" : "Win32";
           var propDebug = project.Xml.AddPropertyGroup();
           propDebug.Condition = "'$(Configuration)|$(Platform)'=='Debug|" + platform + "'";
           propDebug.SetProperty("LocalDebuggerCommandArguments", args);
           var propRelease = project.Xml.AddPropertyGroup();
           propRelease.Condition = "'$(Configuration)|$(Platform)'=='Release|" + platform + "'";
           propRelease.SetProperty("LocalDebuggerCommandArguments", args);
       }
   }

   public static List<string> GetDependingLibs(string subdir)
   {
       string srcDir = theSrcDir;
       string makeEnvFile = srcDir + @"\" + subdir + @"\Makefile.env";
       StreamReader fs = new StreamReader(makeEnvFile);
       string line = null;
       List<string> deps = new List<string>();
       while((line = fs.ReadLine()) != null) {
          if (line.StartsWith("DEPENDS :=")) {
	      string [] strDeps = line.Substring(10).Trim().Split(' ');
	      foreach (var dep in strDeps) {
	          if (dep.Trim().Length != 0) {
	              deps.Add(dep);
		  }
	      }
	  }
       }
       fs.Close();
       return deps;
   }

   public static List<string> GetAdditionalIncludeDirs(string subdir)
   {
       List<string> incDirs = new List<string>();
       string slnRoot = Path.GetFullPath(theBinDir);
       string srcDir = theSrcDir;
       string makeEnvFile = srcDir + @"\" + subdir + @"\Makefile.env";
       if (!File.Exists(makeEnvFile)) {
           return incDirs;
       }
       StreamReader fs = new StreamReader(makeEnvFile);
       string line = null;
       while((line = fs.ReadLine()) != null) {
          if (line.StartsWith("INCDIR :=")) {
	      string [] strIncDirs = line.Substring(10).Trim().Split(' ');
	      foreach (var dir in strIncDirs) {
	          if (dir.Trim().Length != 0) {

		      string dir1 = dir;
		      if (dir.StartsWith("$(ROOT)")) {
		          var incPath = Path.GetFullPath(Path.Combine(theRootDir, dir.Substring("$(ROOT)/".Length).Replace("/", "\\")));
		          string newDir = "$(SolutionDir)" + MakeRelative(incPath, slnRoot);
			  dir1 = newDir;
		      }

	              incDirs.Add(dir1);
		  }
	      }
	  }
       }
       fs.Close();
       return incDirs;
   }

   public static string GetExeName(string subdir)
   {
       string srcDir = theSrcDir;
       string makeEnvFile = srcDir + @"\" + subdir + @"\Makefile.env";
       StreamReader fs = new StreamReader(makeEnvFile);
       string exeName = null;
       string line = null;
       while((line = fs.ReadLine()) != null) {
          if (line.StartsWith("EXE := ")) {
	      exeName = line.Substring(7).Trim();
	      break;
	  }
       }
       fs.Close();
       return exeName;
   }

   public static string GetDefaultArgs(string subdir)
   {
       string srcDir = theSrcDir;
       string makeEnvFile = srcDir + @"\" + subdir + @"\Makefile.env";
       StreamReader fs = new StreamReader(makeEnvFile);
       string exeName = null;
       string line = null;
       while((line = fs.ReadLine()) != null) {
          if (line.StartsWith("DEFAULT_ARGS := ")) {
	      exeName = line.Substring(16).Trim();
	      break;
	  }
       }
       fs.Close();
       return exeName;
   }

   public static string GetWinCCExtra(string subdir)
   {
       if (subdir.EndsWith(@"\test")) {
       	  subdir = subdir + @"\..";
       }
       string srcDir = theSrcDir;
       string makeEnvFile = srcDir + @"\" + subdir + @"\Makefile.env";
       if (!File.Exists(makeEnvFile)) {
           return null;
       }

       StreamReader fs = new StreamReader(makeEnvFile);
       string exeName = null;
       string line = null;
       while((line = fs.ReadLine()) != null) {
          if (line.StartsWith("WIN_CC_EXTRA := ")) {
	      exeName = line.Substring(16).Trim();
	      break;
	  }
       }
       fs.Close();
       return exeName;
   }

   public static void SetupApplication(Project project, string name, List<string> importLibs)
   {
       foreach (var lib in importLibs) {
           string libFile = @"$(OutDir)" + lib + ".lib";
           project.AddItem("Link", libFile);
       }

       var itemDefGroup = project.Xml.AddItemDefinitionGroup();
       var clDef = itemDefGroup.AddItemDefinition("ClCompile");
       clDef.AddMetadata("DebugInformationFormat", "ProgramDatabase");
       clDef.AddMetadata("ProgramDataBaseFileName", @"$(OutDir)" + name + ".pdb");

       var platform = (theBit == "64") ? "x64" : "Win32";

       var linkDefGroup = project.Xml.AddItemDefinitionGroup();
       var linkDef = linkDefGroup.AddItemDefinition("Link");
       linkDef.Condition =  "'$(Configuration)|$(Platform)'=='Debug|" + platform + "'";
       linkDef.AddMetadata("GenerateDebugInformation", "true");
       linkDef.AddMetadata("SubSystem", "Console");

       // Extract numbers from thePlatformToolset
       string vcVer = "";
       foreach (char c in thePlatformToolset) {
           if (c >= '0' && c <= '9') vcVer += c;
       }
       vcVer = vcVer.Substring(0, vcVer.Length-1) + "." + vcVer[vcVer.Length-1];
       string boostLibDir = theBoostDir + @"\" + "lib" + theBit + "-msvc-" + vcVer;
       // Console.WriteLine("BoostLibDir: " + boostLibDir);
       linkDef.AddMetadata("AdditionalLibraryDirectories", boostLibDir);

       var propDebug = project.Xml.AddPropertyGroup();
       propDebug.Condition = "'$(Configuration)|$(Platform)'=='Debug|" + platform + "'";
       propDebug.SetProperty("BuildLinkTargets", "");
   }

   public static void CreateExeAndRunProject(Project project,
					     string subdir,
                                             string [] srcFiles,
   	  	      		             string libName,
					     bool isScript)
   {
       string slnRoot = Path.GetFullPath(theBinDir);
       string name = subdir.Replace(@"\", "_");

       InitProject(project, subdir + @"\..\", subdir, name, "Application");

       string mandatoryImportLib = libName;

       // Get depending libs from parent Makefile.env
       List<string> importLibs = GetDependingLibs(subdir + @"\..\");

       string mainPre = "";
       string mainBody = "";

       foreach (var srcFile in srcFiles) {
           string filePath = Path.GetFullPath(srcFile);

	   if (Path.GetExtension(srcFile) != ".cpp") {
	      continue;
 	   }

           string srcFilePath = @"$(SolutionDir)" + MakeRelative(filePath, slnRoot);
	   string mainFun = "main_" + Path.GetFileNameWithoutExtension(srcFile);

           ProjectItem item = project.AddItem("ClCompile", srcFilePath)[0];
	   item.Xml.AddMetadata("PreprocessorDefinitions", "main=" + mainFun);
	   mainBody += "    " + mainFun + "(argc, argv);\n";
	   mainPre += "int " + mainFun + "(int argc, char *argv[]);\n";
       }

       string allMain = mainPre + "\n"
          + "int main(int argc, char *argv[])\n"
	  + "{\n"
	  + mainBody
	  + "}\n";

       // Write this file and add it
       string mainPath = Path.GetFullPath(theBinDir + @"\" + name + "_main.cpp");
       string mainPath1 = @"$(SolutionDir)" + MakeRelative(mainPath, slnRoot);
       var mainFile = new StreamWriter(mainPath);
       mainFile.Write(allMain);
       mainFile.Close();

       if (mandatoryImportLib != null) {
           importLibs.Add(mandatoryImportLib);
       }

       project.AddItem("ClCompile", mainPath1);

       SetupApplication(project, name, importLibs);

       if (isScript) {
           var postEvent = project.Xml.AddItemGroup();
	   var events = postEvent.AddItem("PostBuildEvent", "Command");
	   events.AddMetadata("Command", "\"$(OutDir)" + name + ".exe\"" + " " + "\"$(SolutionDir)$(Configuration)\"");
       }
   }


   private static void processDirectory(string srcDir, string subdir, bool isRunProject, bool isScript)
   {
       // Console.WriteLine("Process " + subdir);

       string name = subdir.Replace(@"\", "_");

       if (!isRunProject) {

          string exeName = GetExeName(subdir);
	  bool isLib = exeName == null;

	  if (isLib) {
              //
              // Library project
	      //
       	      Project libProject = new Project();
       	      CreateLibProject(libProject, subdir);
	      string libFile = name;
       	      Console.WriteLine("Saving " + (theBinDir + @"\" + libFile + ".vcxproj"));
       	      libProject.Save(theBinDir + @"\" + libFile + ".vcxproj", Encoding.ASCII);
	  } else {
	      //
	      // Exe project
	      //
	      Project exeProject = new Project();
	      CreateExeProject(exeProject, subdir, exeName);
	      

	      Console.WriteLine("Saving " + (theBinDir + @"\" + exeName + ".vcxproj"));
	      exeProject.Save(theBinDir + @"\" + exeName + ".vcxproj", Encoding.ASCII);
	  }
	  
          // projs.Add(libFile, NextGuid());
       } else {
          //
       	  // Make executable and run - project
          //
	  string importLibName = null;
	  if (GetExeName(subdir + @"\..\") == null) {
	      importLibName = subdir;
	      importLibName = subdir.Substring(0, subdir.IndexOf(@"\"));
	  }
          string [] srcFiles = Directory.GetFiles(srcDir + @"\" + subdir);
	  Project runProject = new Project();
          CreateExeAndRunProject(runProject, subdir,
	  			 srcFiles, importLibName, isScript);
          var projFile = theBinDir + @"\" + name + ".vcxproj";
       	  Console.WriteLine("Saving " + projFile);
       	  runProject.Save(projFile, Encoding.ASCII);
       }
   }

   public static void Main(string [] args)
   {
       string rootDir = "";
       string srcDir = "";
       string outDir = "";
       string binDir = "";
       string env = "";
       string boost = "";
       string bit = "";
       Console.WriteLine("make_vcproj");
       foreach (string x in args) {
           if (x.StartsWith("root=")) {
	       rootDir = x.Substring(5);
           }
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
	   if (x.StartsWith("bit=")) {
	      bit = x.Substring(4);
	   }
	   if (x.StartsWith("boost=")) {
	      boost = x.Substring(6);
	   }
       }

       Console.WriteLine("rootDir=" + rootDir);
       Console.WriteLine("srcDir=" + srcDir);
       Console.WriteLine("outDir=" + outDir);
       Console.WriteLine("binDir=" + binDir);
       Console.WriteLine("   env=" + env);
       Console.WriteLine("   bit=" + bit);
       Console.WriteLine("boost =" + boost);
       Directory.CreateDirectory(binDir);
       Directory.CreateDirectory(binDir + @"\test");

       thePlatformToolset = GetPlatformToolset(env);
       if (thePlatformToolset.Length == 0) {
           Console.WriteLine("Could not determine Visual Studio Version");
	   return;
       }

       theRootDir = rootDir;
       theSrcDir = srcDir;
       theOutDir = outDir;
       theBinDir = binDir;
       theBoostDir = boost;
       theBit = bit;

       // Iterate through directories from src

       Stack<string> stack = new Stack<string>();
       stack.Push(srcDir);
       while (stack.Count() > 0) {
       	     string dir = stack.Pop();
             string [] cppFiles = Directory.GetFiles(dir, "*.cpp");

	     bool isRun = false;
	     bool isScript = false;
	     if (cppFiles.Count() > 0) {
	         if (dir.EndsWith(@"\test") || dir.EndsWith(@"\script")) {
		     isRun = true;
		     isScript = dir.EndsWith(@"\script");
		 }
    	         string subdir = MakeRelative(dir, srcDir);
	         processDirectory(srcDir, subdir, isRun, isScript);
             }

	     string [] subdirs = Directory.GetDirectories(dir);
	     foreach (var subdir in subdirs) {
	         stack.Push(subdir);
	     }
       }
   }
}