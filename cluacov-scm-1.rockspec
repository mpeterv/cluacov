package = "cluacov"
version = "scm-1"
source = {
   url = "git+https://github.com/mpeterv/cluacov.git"
}
description = {
   homepage = "https://github.com/mpeterv/cluacov",
   license = "MIT/X11"
}
dependencies = {
   "lua >= 5.1, < 5.4",
   "luacov" -- TODO: depend on exact compatible minor version.
}
build = {
   type = "builtin",
   modules = {
      ["cluacov.deepactivelines"] = "src/cluacov/deepactivelines.c",
      ["cluacov.hook"] = "src/cluacov/hook.c",
      ["cluacov.version"] = "src/cluacov/version.lua"
   }
}
