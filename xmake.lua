set_languages("cxx11")
add_rules("mode.debug", "mode.release")

set_version("0.1.0", {build = "%Y%m%d%H%M"})

-- set_optimize("fastest")
-- add_cxflags("-fno-strict-aliasing", "-DDEBUG")

target("tinge")
    set_kind("static")
    add_includedirs("include")
    add_files("src/*.cpp")
