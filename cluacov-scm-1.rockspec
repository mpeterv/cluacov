package = "cluacov"
version = "scm-1"
source = {
   url = "git+https://github.com/mpeterv/cluacov.git"
}
description = {
   summary = "C extensions for LuaCov",
   detailed = [[
      Optional C extensions for LuaCov, improving performance and reducing
      number of lines incorrectly marked as missed. Depends on LuaCov.
   ]],
   homepage = "https://github.com/mpeterv/cluacov",
   license = "MIT/X11"
}
dependencies = {
   "lua >= 5.1, < 5.4",
   "luacov >= 0.12.0, < 0.13.0"
}
build = {
   type = "builtin",
   modules = {
      ["cluacov.deepactivelines"] = "src/cluacov/deepactivelines.c",
      ["cluacov.hook"] = "src/cluacov/hook.c",
      ["cluacov.version"] = "src/cluacov/version.lua"
   }
}
