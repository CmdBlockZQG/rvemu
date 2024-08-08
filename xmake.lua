set_languages("cxx17")
set_warnings("all", "error")
set_optimize("fastest")

target("rvemu")
  set_kind("binary")
  set_targetdir("build")
  add_includedirs("include")
  add_files("src/**.cc")
