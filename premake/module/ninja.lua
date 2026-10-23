-- Patch premake5's ninja module to emit resource compilation steps

local p = premake
local ninja = p.modules.ninja
if ninja ~= nil then
  function ninja.cpp.getResFlags(cfg, filecfg, toolset)
    toolset = toolset or ninja.gettoolset(cfg)

    local resflags =
      table.join(toolset.getdefines(cfg.resdefines), toolset.getincludedirs(cfg, cfg.resincludedirs), cfg.resoptions)
    return resflags
  end

  function ninja.cpp.configurationVariables(cfg)
    local toolset = ninja.gettoolset(cfg)

    local cflags = ninja.cpp.getCFlags(cfg, toolset)
    if #cflags > 0 then
      p.w("cflags_%s = %s", ninja.key(cfg), table.concat(cflags, " "))
    end

    local cxxflags = ninja.cpp.getCxxFlags(cfg, toolset)
    if #cxxflags > 0 then
      p.w("cxxflags_%s = %s", ninja.key(cfg), table.concat(cxxflags, " "))
    end

    local ldflags = ninja.cpp.getLdFlags(cfg, toolset)
    if #ldflags > 0 then
      p.w("ldflags_%s = %s", ninja.key(cfg), table.concat(ldflags, " "))
    end

    local links = toolset.getlinks(cfg)
    if #links > 0 then
      p.w("links_%s = %s", ninja.key(cfg), table.concat(links, " "))
    end

    local resflags = ninja.cpp.getResFlags(cfg, nil, toolset)
    if #resflags > 0 then
      p.w("resflags_%s = %s", ninja.key(cfg), table.concat(resflags, " "))
    end

    p.w("objdir_%s = %s", ninja.key(cfg), p.project.getrelative(cfg.project, cfg.objdir))
    p.w("targetdir_%s = %s", ninja.key(cfg), p.project.getrelative(cfg.project, cfg.buildtarget.directory))
    p.w("target_%s = %s", ninja.key(cfg), cfg.buildtarget.name)

    p.w("")
  end

  function ninja.cpp.objectFile(cfg, node, filecfg)
    local objdir = path.getrelative(cfg.workspace.location, cfg.objdir)
    local ext = path.getextension(node.abspath):lower()

    local shouldCompile = false

    if filecfg and (filecfg.buildaction == "Compile" or filecfg.buildaction == "Resource") then
      shouldCompile = true
    elseif filecfg and filecfg.compileas and filecfg.compileas ~= "Default" then
      if p.languages.isc(filecfg.compileas) or p.languages.iscpp(filecfg.compileas) then
        shouldCompile = true
      end
    else
      if path.iscppfile(node.abspath) or path.iscfile(node.abspath) or ext == ".rc" then
        shouldCompile = true
      end
    end

    if shouldCompile then
      local objname = filecfg.objname or path.getbasename(node.abspath)
      local toolset = ninja.gettoolset(cfg)
      local objext = toolset.gettooloutputext("cc")
      if ext == ".rc" then
        if toolset.gettooloutputext then
          objext = toolset.gettooloutputext("rc") or iif(toolset == p.tools.msc, ".res", ".o")
        else
          objext = iif(toolset == p.tools.msc, ".res", ".o")
        end
      end
      return objdir .. "/" .. objname .. objext
    end

    return nil
  end

  function ninja.cpp.hasPerFileConfiguration(cfg, filecfg, rule)
    if not filecfg then
      return false
    end

    local toolset = ninja.gettoolset(cfg)
    if not rule or rule == "cc" then
      local cflagsBase = table.concat(ninja.cpp.getCFlags(cfg, toolset), " ")
      local cflagsFile = table.concat(ninja.cpp.getFileCFlags(cfg, filecfg, toolset), " ")
      if cflagsBase ~= cflagsFile then
        return true
      end
    end

    if not rule or rule == "cxx" then
      local cxxflagsBase = table.concat(ninja.cpp.getCxxFlags(cfg, toolset), " ")
      local cxxflagsFile = table.concat(ninja.cpp.getFileCxxFlags(cfg, filecfg, toolset), " ")
      if cxxflagsBase ~= cxxflagsFile then
        return true
      end
    end

    if not rule or rule == "rc" then
      local resflagsBase = table.concat(ninja.cpp.getResFlags(cfg, nil, toolset), " ")
      local resflagsFile = table.concat(ninja.cpp.getResFlags(cfg, filecfg, toolset), " ")
      if resflagsBase ~= resflagsFile then
        return true
      end
    end

    return false
  end

  function ninja.cpp.buildFile(cfg, node, filecfg, objFile, pchFile, prebuildTarget)
    local ext = path.getextension(node.abspath):lower()
    local rule = nil
    local flags = ""
    local extraFlags = {}
    local toolset = ninja.gettoolset(cfg)
    local toolsetName = ninja.toolsetname(cfg)
    local assemblyFile
    if toolset.getassemblyflags then
      local assemblyCfg = filecfg.generateassembly and filecfg or cfg
      if toolset.getassemblyoutput then
        assemblyFile = toolset.getassemblyoutput(assemblyCfg, objFile)
      end
      table.insertflat(extraFlags, toolset.getassemblyflags(assemblyCfg, assemblyFile and p.quoted(assemblyFile)))
    end

    if filecfg and filecfg.compileas and filecfg.compileas ~= "Default" then
      if p.languages.isc(filecfg.compileas) then
        rule = "cc"
        flags = "cflags_" .. ninja.key(cfg)
      elseif p.languages.iscpp(filecfg.compileas) then
        rule = "cxx"
        flags = "cxxflags_" .. ninja.key(cfg)
      end
    elseif filecfg and filecfg.buildaction == "Compile" then
      if p.languages.isc(cfg.language) then
        rule = "cc"
        flags = "cflags_" .. ninja.key(cfg)
      else
        rule = "cxx"
        flags = "cxxflags_" .. ninja.key(cfg)
      end
    elseif (filecfg and filecfg.buildaction == "Resource") or path.getextension(node.abspath):lower() == ".rc" then
      rule = "rc"
      flags = "resflags_" .. ninja.key(cfg)
    else
      if path.iscfile(node.abspath) then
        rule = "cc"
        flags = "cflags_" .. ninja.key(cfg)
      elseif path.iscppfile(node.abspath) then
        rule = "cxx"
        flags = "cxxflags_" .. ninja.key(cfg)
      elseif path.getextension(node.abspath):lower() == ".rc" then
        rule = "rc"
        flags = "resflags_" .. ninja.key(cfg)
      end
    end

    if rule then
      local relPath = path.getrelative(cfg.workspace.location, node.abspath)
      local implicitDeps = ""

      local usePch = cfg.pchheader
        and cfg.enablepch ~= p.OFF
        and (not filecfg or filecfg.enablepch ~= p.OFF)
        and rule ~= "rc"
      if pchFile and usePch then
        implicitDeps = implicitDeps .. " | " .. pchFile
      end

      if prebuildTarget then
        if implicitDeps == "" then
          implicitDeps = " |"
        end
        implicitDeps = implicitDeps .. " " .. prebuildTarget
      end

      if cfg._customRuleOutputs and #cfg._customRuleOutputs > 0 then
        if implicitDeps == "" then
          implicitDeps = " |"
        end
        for _, output in ipairs(cfg._customRuleOutputs) do
          implicitDeps = implicitDeps .. " " .. output
        end
      end

      -- Check if there are any dependson targets and add them as implicit dependencies
      if cfg._dependsOnTargets then
        if implicitDeps == "" then
          implicitDeps = " |"
        end
        implicitDeps = implicitDeps .. " " .. table.concat(cfg._dependsOnTargets, " ")
      end

      local implicitOutput = ""
      if assemblyFile then
        implicitOutput = " | " .. assemblyFile
      end
      p.w("build %s%s: %s_%s %s%s", objFile, implicitOutput, rule, toolsetName, relPath, implicitDeps)

      if usePch then
        if toolset == p.tools.msc then
          table.insert(extraFlags, "/Yu" .. cfg.pchheader)
          local pchPath = ninja.cpp.getPchPath(cfg)
          if pchPath then
            local wksRelPchPath = path.getrelative(cfg.workspace.location, path.join(cfg.project.location, pchPath))
            table.insert(extraFlags, "/Fp" .. wksRelPchPath)
          end
        else
          local pch = toolset.getpch(cfg)
          if pch then
            local objdir = path.getrelative(cfg.workspace.location, cfg.objdir)
            local pchPlaceholder = objdir .. "/" .. path.getname(pch)
            -- Add the directory containing the PCH header to the search path so
            -- #include "pch.h" in source files can be resolved when building from
            -- the workspace root (where the project subdirectory is not in the path).
            local pchDir = path.getdirectory(pch)
            if pchDir and pchDir ~= "" and pchDir ~= "." then
              table.insert(extraFlags, "-I " .. pchDir)
            end
            table.insert(extraFlags, "-include " .. pchPlaceholder)
          end
        end
      end

      -- If the file has per-file configuration, generate flags directly rather than using variables
      local hasPerFileConfig = ninja.cpp.hasPerFileConfiguration(cfg, filecfg, rule)
      if hasPerFileConfig then
        local fileFlags = {}
        if rule == "cc" then
          fileFlags = ninja.cpp.getFileCFlags(cfg, filecfg, toolset)
        elseif rule == "cxx" then
          fileFlags = ninja.cpp.getFileCxxFlags(cfg, filecfg, toolset)
        elseif rule == "rc" then
          fileFlags = ninja.cpp.getResFlags(cfg, filecfg, toolset)
        end

        table.insertflat(fileFlags, extraFlags)

        if #fileFlags > 0 then
          if rule == "cc" then
            p.w("  cflags = %s", table.concat(fileFlags, " "))
          elseif rule == "cxx" then
            p.w("  cxxflags = %s", table.concat(fileFlags, " "))
          elseif rule == "rc" then
            p.w("  resflags = %s", table.concat(fileFlags, " "))
          end
        end
      else
        if rule == "cc" then
          if #extraFlags > 0 then
            p.w("  cflags = $%s %s", flags, table.concat(extraFlags, " "))
          else
            p.w("  cflags = $%s", flags)
          end
        elseif rule == "cxx" then
          if #extraFlags > 0 then
            p.w("  cxxflags = $%s %s", flags, table.concat(extraFlags, " "))
          else
            p.w("  cxxflags = $%s", flags)
          end
        elseif rule == "rc" then
          if #extraFlags > 0 then
            p.w("  resflags = $%s %s", flags, table.concat(extraFlags, " "))
          else
            p.w("  resflags = $%s", flags)
          end
        end
      end
    end
  end
end
