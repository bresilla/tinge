set_languages("cxx11")
add_rules("mode.debug", "mode.release")

set_version("0.1.0", {build = "%Y%m%d%H%M"})

-- set_optimize("fastest")
-- add_cxflags("-fno-strict-aliasing", "-DDEBUG")

target("tinge")
    set_kind("static")
    add_includedirs("include")
    add_files("src/*.cpp")


-- package("tinge")
-- set_description("TINGE PACKAGE")
-- set_license("AMIT")

-- on_load(function (package)
--     package:set("installdir", path.join(os.scriptdir(), package:plat(), package:arch(), package:mode()))
-- end)


-- on_fetch(function (package)
--     local result = {}
--     result.links = "tinge"
--     result.linkdirs = package:installdir("lib")
--     result.includedirs = package:installdir("include")
--     return result
-- end)