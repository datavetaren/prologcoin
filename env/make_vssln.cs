using System.IO;
using System.Text;
using System;
using System.Linq;
using System.Collections.Generic;
using System.Xml;
using System.Diagnostics;
using Microsoft.Build.Construction;
using Microsoft.Build.Evaluation;

public class make_vssln
{
   static private string PROJECT = "prologcoin";
   static private string theRootDir;
   static private string theBinDir;
   static private string theSrcDir;
   static private Dictionary<string, string []> theDeps
   	  = new Dictionary<string, string []>();

   class FileComp : IComparer<string>
   {
       public FileComp(string main)
       {
	   thisMain = main;
       }
       
       public int Compare(string x, string y)
       {
	   if (x == y) {
	       return 0;
	   }
	   if (x == thisMain) {
	       return -1;
	   }
           if (x.Contains("_test") && !y.Contains("_test")) {
	       return -1;
	   }
	   if (!x.Contains("_test") && y.Contains("_test")) {
	       return 1;
	   }
	   return x.CompareTo(y);
       }

private string thisMain;
   }

   public static string [] GetDependingLibs(string srcDir, string subdir)
   {
       string makeEnvFile = srcDir + @"\" + subdir + @"\Makefile.env";
       StreamReader fs = new StreamReader(makeEnvFile);
       string line = null;
       string [] deps = null;
       while((line = fs.ReadLine()) != null) {
          if (line.StartsWith("DEPENDS :=")) {
	      deps = line.Substring(11).Trim().Split(' ');
	  }
       }
       fs.Close();
       if (deps == null) {
           deps = new string[0];
       }
       return deps;
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

   private static void ComputeDepsDir(string srcDir, string subdir, bool isTest, string exeName)
   {
       string name = exeName != null ? exeName : subdir.Replace(@"\", "_");

       if (!isTest) {
           string [] deps = GetDependingLibs(srcDir, subdir);
	   theDeps[name] = deps;
	   /*
	   Console.WriteLine("Add dependency: " + name + " --> ");
	   foreach (var n in deps) {
	       Console.WriteLine("    " + n);
	   }
	   */
       }
   }

   public static string GetExeName(string dir)
   {
       string makeEnvFile = dir + @"\Makefile.env";
       try {
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
       } catch {
              return null;
       }
   }
   
   public static void ComputeDeps(string srcDir)
   {	  
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
		 string exeName = GetExeName(dir);
    	         string subdir = MakeRelative(dir, srcDir);
	         ComputeDepsDir(srcDir, subdir, isTestDir, exeName);
             }

	     string [] subdirs = Directory.GetDirectories(dir);
	     foreach (var subdir in subdirs) {
	         stack.Push(subdir);
	     }
       }
   }

   private static void AddDep(Dictionary<string,string> names, string depLib, StreamWriter sw)
   {
   	if (names.ContainsKey(depLib)) {
	    string depGuid = names[depLib];
	    sw.WriteLine("\tProjectSection(ProjectDependencies) = postProject");
            sw.WriteLine("\t\t" + depGuid + " = " + depGuid);
            sw.WriteLine("\tEndProjectSection");
       }
   }

   public static bool IsLibrary(string vcxPath)
   {
       StreamReader fs = new StreamReader(vcxPath);
       bool isLib = false;
       string line;
       while((line = fs.ReadLine()) != null) {
          if (line.Contains("StaticLibrary")) {
	      isLib = true;
	      break;
	  }
       }
       fs.Close();
       return isLib;
   }

   public static string FindMainProj(string [] projs)
   {
	foreach (string proj in projs) {
	    if (IsLibrary(proj)) {
	         continue;
	    }
	    if (proj.EndsWith("_test.vcxproj")) {
	         continue;
	    }
	    return proj;
        }
        return null;
   }

   public static void Main(string [] args)
   {
       string binDir = "";
       string srcDir = "";
       foreach (string x in args) {
	   if (x.StartsWith("bin=")) {
	       binDir = x.Substring(4);
	   }
	   if (x.StartsWith("src=")) {
	       srcDir = x.Substring(4);
	   }
       }
       Console.WriteLine("srcDir=" + srcDir);
       Console.WriteLine("binDir=" + binDir);
       string root = Path.GetFullPath(binDir);
       theRootDir = root;
       theBinDir = binDir;
       theSrcDir = srcDir;


       //
       // Get all dependencies
       //

       ComputeDeps(srcDir);

       //
       // Solution file
       //

       Console.WriteLine("Saving " + binDir + @"\" + PROJECT + ".sln");
       var sw = new StreamWriter(binDir + @"\" + PROJECT + ".sln");
       sw.WriteLine("Microsoft Visual Studio Solution File, Format Version 12.00");
       sw.WriteLine("# Visual Studio 14");
       sw.WriteLine("VisualStudioVersion = 14.0.25420.1"); // "12.0.31101.0");
       sw.WriteLine("MinimumVisualStudioVersion = 10.0.40219.1");

       //
       // Open all vcxproj files and find GUIDs.

       string [] projFiles = Directory.GetFiles(binDir, "*.vcxproj");
       //
       // Find the main vcxporj file
       //
       var mainProj = FindMainProj(projFiles);

       Console.WriteLine("Main project is: " + mainProj);

       var projs = new SortedDictionary<string,string>(new FileComp(mainProj));
       var names = new Dictionary<string,string>();
       foreach (var projFile in projFiles) {
           XmlDocument xml = new XmlDocument();
	   xml.Load(projFile);
	   var guid = xml.SelectSingleNode("/*[local-name() = 'Project']/*[local-name() = 'PropertyGroup']/*[local-name() = 'ProjectGuid']").InnerText.ToUpper();
	   projs.Add(projFile, guid);
	   names.Add(Path.GetFileNameWithoutExtension(projFile), guid);
       }

       foreach (var entry in projs) {
           string projFile = Path.GetFileName(entry.Key);
           string projGuid = entry.Value;

	   Console.WriteLine( projFile );
           sw.WriteLine("Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"" + Path.GetFileNameWithoutExtension(projFile) + "\", \"" + projFile + "\", \"" + projGuid + "\"");

	   // Is this a unittest?
	   if (!IsLibrary(binDir + @"\" + projFile)) {
	        var depLib = projFile.Substring(0, projFile.Length);
	        // Extract the non-unittest lib project
	        if (projFile.EndsWith("_test.vcxproj")) {
	           depLib = projFile.Substring(0, projFile.Length
		   	            - "_test.vcxproj".Length);
	        } else if (projFile.EndsWith(".vcxproj")) {
		   depLib = projFile.Substring(0, projFile.Length
		   	            - ".vcxproj".Length);
		} else {	    
		   Debug.Assert(false, "Unknown project vcxproj file" );
		}
		AddDep(names, depLib, sw);

		// Add all dependent libs
		var deps = theDeps[depLib];
		foreach (var dep in deps) {
		    AddDep(names, dep, sw);
		}
           }

           sw.WriteLine("EndProject");
       }

       sw.WriteLine("Global");

       sw.WriteLine("\tGlobalSection(SolutionConfigurationPlatforms) = preSolution");
       sw.WriteLine("\t\tDebug|x64 = Debug|x64");
       sw.WriteLine("\t\tRelease|x64 = Release|x64");
       sw.WriteLine("\t\tDebug|x86 = Debug|x86");
       sw.WriteLine("\t\tRelease|x86 = Release|x86");
       sw.WriteLine("\tEndGlobalSection");

       sw.WriteLine("\tGlobalSection(ProjectConfigurationPlatforms) = postSolution");
       foreach (var entry in projs) {
           string projGuid = entry.Value;
	   sw.WriteLine("\t\t" + projGuid + ".Debug|x64.ActiveCfg = Debug|x64");
	   sw.WriteLine("\t\t" + projGuid + ".Debug|x64.Build.0 = Debug|x64");
	   sw.WriteLine("\t\t" + projGuid + ".Release|x64.ActiveCfg = Release|x64");
	   sw.WriteLine("\t\t" + projGuid + ".Release|x64.Build.0 = Release|x64");

	   sw.WriteLine("\t\t" + projGuid + ".Debug|x86.ActiveCfg = Debug|Win32");
	   sw.WriteLine("\t\t" + projGuid + ".Debug|x86.Build.0 = Debug|Win32");
	   sw.WriteLine("\t\t" + projGuid + ".Release|x86.ActiveCfg = Release|Win32");
	   sw.WriteLine("\t\t" + projGuid + ".Release|x86.Build.0 = Release|Win32");
       }
       sw.WriteLine("\tEndGlobalSection");

       sw.WriteLine("\tGlobalSection(SolutionProperties) = preSolution");
       sw.WriteLine("\t\tHideSolutionNode = FALSE");
       sw.WriteLine("\tEndGlobalSection");

       sw.WriteLine("EndGlobal");

       sw.Close();
   }
}
