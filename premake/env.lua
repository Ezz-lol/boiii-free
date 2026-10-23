-- Automatically populates MSVC, Windows SDK, ATL/MFC, and LLVM 23 environment variables on Windows.

local M = {}

function M.init()
  if os.is("windows") then
    print("[Env][Setup] Checking and configuring Windows build environment...")

    -- 1. Configure MSVC and Windows SDK environment if not already present
    if not (os.getenv("INCLUDE") and os.getenv("LIB") and os.getenv("VCToolsInstallDir")) then
      local program_files_x86 = os.getenv("ProgramFiles(x86)") or "C:\\Program Files (x86)"
      local vswhere_path = program_files_x86 .. "\\Microsoft Visual Studio\\Installer\\vswhere.exe"

      if os.isfile(vswhere_path) then
        -- Query vswhere for the latest VS installation with C++ tools
        local cmd = string.format(
          '"%s" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath',
          vswhere_path
        )
        local handle = io.popen(cmd)
        if handle then
          local vs_path = handle:read("*a")
          handle:close()

          vs_path = vs_path:gsub("^%s+", ""):gsub("%s+$", "")
          if vs_path ~= "" then
            local vs_dev_cmd = vs_path .. "\\Common7\\Tools\\VsDevCmd.bat"
            if os.isfile(vs_dev_cmd) then
              print("[Env][Setup] Initializing MSVC environment from: " .. vs_dev_cmd)

              local temp_file = (os.getenv("TEMP") or "C:\\Windows\\Temp") .. "\\premake_env_" .. os.time() .. ".txt"
              local run_cmd = string.format('cmd.exe /c "call "%s" -arch=x64 && set > "%s""', vs_dev_cmd, temp_file)
              os.execute(run_cmd)

              local file = io.open(temp_file, "r")
              if file then
                local content = file:read("*a")
                file:close()
                os.remove(temp_file)

                for line in content:gmatch("[^\r\n]+") do
                  local eq = line:find("=")
                  if eq then
                    local key = line:sub(1, eq - 1)
                    local val = line:sub(eq + 1)
                    os.setenv(key, val)
                  end
                end
                print("[Env][Setup] Successfully loaded MSVC, Windows SDK, ATL, and MFC environment variables.")
              else
                print("[Env][Setup] Error: Failed to capture environment output from VsDevCmd.bat.")
              end
            else
              print("[Env][Setup] Warning: VsDevCmd.bat not found at expected path: " .. vs_dev_cmd)
            end
          else
            print("[Env][Setup] Warning: No Visual Studio installation with C++ build tools found via vswhere.")
          end
        end
      else
        print("[Env][Setup] Warning: vswhere.exe not found at " .. vswhere_path)
      end
    else
      print("[Env][Setup] MSVC environment variables already present; skipping initialization.")
    end

    -- 2. Ensure LLVM 23 is on the PATH
    local current_path = os.getenv("PATH") or ""

    local function check_llvm_version(bin_path)
      local clang_exe = bin_path .. "\\clang.exe"
      if os.isfile(clang_exe) then
        local handle = io.popen('"' .. clang_exe .. '" --version')
        if handle then
          local output = handle:read("*a")
          handle:close()
          if output:find("version 23") or output:find("LLVM version 23") then
            return true
          end
        end
      end
      return false
    end

    -- Check if LLVM 23 is already active via PATH
    local already_llvm_23 = false
    local handle = io.popen("clang --version")
    if handle then
      local output = handle:read("*a")
      handle:close()
      if output:find("version 23") or output:find("LLVM version 23") then
        already_llvm_23 = true
      end
    end

    if already_llvm_23 then
      print("[Env][Setup] LLVM 23 toolchain is already active in PATH.")
    else
      local candidate_paths = {
        "C:\\Program Files\\LLVM\\bin",
        "C:\\Program Files\\LLVM 23\\bin",
        "C:\\Program Files\\LLVM\\23\\bin",
        "C:\\LLVM\\bin",
        "C:\\LLVM 23\\bin",
      }

      local found_llvm_bin = nil
      for _, path in ipairs(candidate_paths) do
        if os.isdir(path) and check_llvm_version(path) then
          found_llvm_bin = path
          break
        end
      end

      -- If not in standard static locations, scan Program Files
      if not found_llvm_bin then
        local pf = os.getenv("ProgramFiles") or "C:\\Program Files"
        local dir_handle = io.popen('dir /b "' .. pf .. '"')
        if dir_handle then
          for dir in dir_handle:lines() do
            if dir:lower():find("llvm") then
              local bin_path = pf .. "\\" .. dir .. "\\bin"
              if os.isdir(bin_path) and check_llvm_version(bin_path) then
                found_llvm_bin = bin_path
                break
              end
            end
          end
          dir_handle:close()
        end
      end

      if found_llvm_bin then
        if not current_path:find(found_llvm_bin, 1, true) then
          os.setenv("PATH", found_llvm_bin .. ";" .. current_path)
          print("[Env][Setup] Added LLVM 23 toolchain to PATH: " .. found_llvm_bin)
        else
          print("[Env][Setup] LLVM 23 bin directory already present in PATH.")
        end
      else
        print("[Env][Setup] Warning: LLVM 23 installation directory could not be located automatically.")
      end
    end
  end
end

return M
