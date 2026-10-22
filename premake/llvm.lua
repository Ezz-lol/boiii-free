-- Patch premake5's clang module to use LLVM windres instead of the system default windres, ensuring we support COFF resource compilation

local p = premake
local clang = p.tools.clang
clang.tools["rc"] = "llvm-windres"
