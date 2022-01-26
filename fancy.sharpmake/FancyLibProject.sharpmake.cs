using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using Sharpmake;

namespace Fancy
{
  [Sharpmake.Generate]
  public class FancyLibProject : Project
  {
    public string ExternalBasePath;

    public FancyLibProject()
    {
      AddTargets(new Target(Platform.win64,
        DevEnv.vs2019,
        Optimization.Debug | Optimization.Release,
        OutputType.Lib));

      SourceRootPath = @"[project.SharpmakeCsPath]";
      ExternalBasePath = Path.Combine(SourceRootPath, "/../external/");
    }
    public virtual void ConfigureAll(Configuration conf, Target target)
    {
      conf.Output = Configuration.OutputType.Lib;
      conf.ProjectPath = @"[project.SharpmakeCsPath]";
      conf.TargetLibraryPath = @"[project.SharpmakeCsPath]/../lib/[target.Platform]/[target.Optimization]/[project.Name]";
      conf.IntermediatePath = @"[project.SharpmakeCsPath]/../_build_temp/[target.Platform]/[target.Optimization]/[project.Name]";

      conf.IncludePaths.Add("[project.SourceRootPath]");
      conf.IncludePaths.Add(ExternalBasePath);
      conf.LibraryPaths.Add(ExternalBasePath);

      // DXC
      conf.IncludePaths.Add(ExternalBasePath + "dxc/inc/");
      
      // GLM
      conf.IncludePaths.Add(ExternalBasePath + "glm/");

      // SPIRV-Reflect
      conf.IncludePaths.Add(ExternalBasePath + "SPIRV-Reflect/");
    }
  }
}

