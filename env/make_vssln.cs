using System.IO;
using System.Text;
using System;
using System.Linq;
using System.Collections.Generic;
using System.Xml;
using Microsoft.Build.Construction;
using Microsoft.Build.Evaluation;

public class make_vssln
{
   static private string PROJECT = "prologcoin";
   static private string theRootDir;
   static private string theBinDir;

   class FileComp : IComparer<string>
   {
       public int Compare(string x, string y)
       {
           if (x.Contains("_test") && !y.Contains("_test")) {
	       return -1;
	   }
	   if (!x.Contains("_test") && y.Contains("_test")) {
	       return 1;
	   }
	   return x.CompareTo(y);
       }
   }

   public static void Main(string [] args)
   {
       string binDir = "";
       foreach (string x in args) {
	   if (x.StartsWith("bin=")) {
	       binDir = x.Substring(4);
	   }
       }
       Console.WriteLine("binDir=" + binDir);
       string root = Path.GetFullPath(binDir);
       theRootDir = root;
       theBinDir = binDir;

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
       var projs = new SortedDictionary<string,string>(new FileComp());
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
	   if (projFile.EndsWith("_test.vcxproj")) {
	       // Extract the non-unittest lib project
	        var depLib = projFile.Substring(0, projFile.Length - "_test.vcxproj".Length);
		if (names.ContainsKey(depLib)) {
		    string depGuid = names[depLib];
		    sw.WriteLine("\tProjectSection(ProjectDependencies) = postProject");

                    sw.WriteLine("\t\t" + depGuid + " = " + depGuid);

                    sw.WriteLine("\tEndProjectSection");
	        }
   	   }

           sw.WriteLine("EndProject");
       }

       sw.WriteLine("Global");

       sw.WriteLine("\tGlobalSection(SolutionConfigurationPlatforms) = preSolution");
       sw.WriteLine("\t\tDebug|x86 = Debug|x86");
       sw.WriteLine("\t\tRelease|x86 = Release|x86");
       sw.WriteLine("\tEndGlobalSection");

       sw.WriteLine("\tGlobalSection(ProjectConfigurationPlatforms) = postSolution");
       foreach (var entry in projs) {
           string projGuid = entry.Value;

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
