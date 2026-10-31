local p = premake
local clang = p.tools.clang

if clang ~= nil then
  if clang.tools ~= nil then
    -- Patch premake5's clang module to use LLVM windres instead of the system default windres,
    -- ensuring we support COFF resource compilation
    clang.tools["rc"] = "llvm-windres"
  end

  -- use _host_ OS for host OS-dependent clang args instead of _target_ OS
  function clang.getstructuredimplicitincludedirs(cfg, toolname, language)
    local cmd = nil

    if language == "C" then
      cmd = toolname .. " -v -E -x c " .. iif(os.ishost("windows"), "nul", "/dev/null")
    elseif language == "C++" then
      cmd = toolname .. " -v -E -x c++ " .. iif(os.ishost("windows"), "nul", "/dev/null")
    else
      p.warn("Unsupported language '%s' for implicit include directories", language)
      return {}
    end

    local stdout, stderr = os.outputof(cmd, "both")
    if not stdout or not stderr then
      p.warn("Failed to execute command '%s' to get implicit include directories", cmd)
      return {}
    end

    -- Lines between "#include <...> search starts here:" and "End of search list." are the implicit include directories
    local result = {}
    local inSearchList = false
    for line in stdout:gmatch("[^\r\n]+") do
      if line:find("#include <...> search starts here:") then
        inSearchList = true
      elseif line:find("End of search list.") then
        inSearchList = false
      elseif inSearchList then
        local trimmed = line:match("^%s*(.-)%s*$") -- trim whitespace

        -- If on windows, replace backslashes with forward slashes to normalize the paths
        if os.istarget("windows") then
          trimmed = trimmed:gsub("\\", "/")
        end

        table.insert(result, {
          flag = "-isystem",
          value = trimmed,
        })
      end
    end

    return result
  end
end
