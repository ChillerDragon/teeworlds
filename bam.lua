CheckVersion("0.5")

Import("configure.lua")

--- Setup Config -------
config = NewConfig()
config:Add(OptCCompiler("compiler"))
config:Add(OptTestCompileC("stackprotector", "int main(){return 0;}", "-fstack-protector -fstack-protector-all"))
config:Add(OptTestCompileC("minmacosxsdk", "int main(){return 0;}", "-mmacosx-version-min=10.7 -isysroot /Developer/SDKs/MacOSX10.7.sdk"))
config:Add(OptTestCompileC("buildwithoutsseflag", "#include <immintrin.h>\nint main(){_mm_pause();return 0;}", ""))
config:Finalize("config.lua")

generated_src_dir = "build/src"
builddir = "build/%(arch)s/%(conf)s"
content_src_dir = "datasrc/"

python_in_path = ExecuteSilent("python -V") == 0

-- data compiler
function Python(name)
	if family == "windows" then
		name = str_replace(name, "/", "\\")
		if not python_in_path then
			-- Python is usually registered for .py files in Windows
			return name
		end
	end
	return "python " .. name
end

function ContentCompile(action, output)
	output = PathJoin(generated_src_dir, Path(output))
	AddJob(
		output,
		action .. " > " .. output,
		Python("datasrc/compile.py") .. " " .. action .. " > " .. output
	)
	AddDependency(output, "datasrc/compile.py")
	AddDependency("datasrc/compile.py", "datasrc/content.py", "datasrc/network.py", "datasrc/datatypes.py")
	return output
end


function GenerateCommonSettings(settings, conf, arch, compiler)
	if compiler == "gcc" or compiler == "clang" then
		settings.cc.flags:Add("-Wall", "-fno-exceptions")
	end

	local md5 = Compile(settings, Collect("src/engine/external/md5/*.c"))
	local json = Compile(settings, Collect("src/engine/external/json-parser/*.c"))

	-- globally available libs
	libs = {md5=md5, json=json}
end

function GenerateMacOSXSettings(settings, conf, arch, compiler)
	if arch == "x86" then
		settings.cc.flags:Add("-arch i386")
		settings.link.flags:Add("-arch i386")
	elseif arch == "x86_64" then
		settings.cc.flags:Add("-arch x86_64")
		settings.link.flags:Add("-arch x86_64")
	elseif arch == "ppc" then
		settings.cc.flags:Add("-arch ppc")
		settings.link.flags:Add("-arch ppc")
	elseif arch == "ppc64" then
		settings.cc.flags:Add("-arch ppc64")
		settings.link.flags:Add("-arch ppc64")
	else
		print("Unknown Architecture '" .. arch .. "'. Supported: x86, x86_64, ppc, ppc64")
		os.exit(1)
	end

	-- c++ stdlib needed
	settings.cc.flags:Add("--stdlib=libc++")
	settings.link.flags:Add("--stdlib=libc++")
	-- this also needs the macOS min SDK version to be at least 10.7

	settings.cc.flags:Add("-mmacosx-version-min=10.7")
	settings.link.flags:Add("-mmacosx-version-min=10.7")

	if config.minmacosxsdk.value == 1 then
		settings.cc.flags:Add("-isysroot /Developer/SDKs/MacOSX10.7.sdk")
		settings.link.flags:Add("-isysroot /Developer/SDKs/MacOSX10.7.sdk")
	end

	settings.link.frameworks:Add("Carbon")
	settings.link.frameworks:Add("AppKit")

	GenerateCommonSettings(settings, conf, arch, compiler)

	-- Master server, version server and tools
	BuildEngineCommon(settings)

	-- Add requirements for Server & Client
	BuildGameCommon(settings)

	BuildServer(settings)

	BuildClient(settings)

	-- Content
	BuildContent(settings, arch, conf)
end

function GenerateLinuxSettings(settings, conf, arch, compiler)
	if arch == "x86" then
		if config.buildwithoutsseflag.value == false then
			settings.cc.flags:Add("-msse2") -- for the _mm_pause call
		end
		settings.cc.flags:Add("-m32")
		settings.link.flags:Add("-m32")
	elseif arch == "x86_64" then
		settings.cc.flags:Add("-m64")
		settings.link.flags:Add("-m64")
	elseif arch == "armv7l" then
		-- arm 32 bit
	else
		print("Unknown Architecture '" .. arch .. "'. Supported: x86, x86_64")
		os.exit(1)
	end
	settings.link.libs:Add("pthread")

	GenerateCommonSettings(settings, conf, arch, compiler)

	-- Master server, version server and tools
	BuildEngineCommon(settings)

	-- Add requirements for Server & Client
	BuildGameCommon(settings)

	-- Server
	BuildServer(settings)

	-- Client
	settings.link.libs:Add("X11")
	settings.link.libs:Add("GL")
	BuildClient(settings)

	-- Content
	BuildContent(settings, arch, conf)
end

function GenerateSolarisSettings(settings, conf, arch, compiler)
	settings.link.libs:Add("socket")
	settings.link.libs:Add("nsl")

	GenerateLinuxSettings(settings, conf, arch, compiler)
end

function GenerateWindowsSettings(settings, conf, target_arch, compiler)
	if compiler == "cl" then
		if (target_arch == "x86" and arch ~= "ia32") or
		   (target_arch == "x86_64" and arch ~= "ia64" and arch ~= "amd64") then
			print("Cross compiling is unsupported on Windows.")
			os.exit(1)
		end
		settings.cc.flags:Add("/wd4244", "/wd4577")
	elseif compiler == "gcc" or config.compiler.driver == "clang" then
		if target_arch ~= "x86" and target_arch ~= "x86_64" then
			print("Unknown Architecture '" .. arch .. "'. Supported: x86, x86_64")
			os.exit(1)
		end

		-- disable visibility attribute support for gcc on windows
		settings.cc.defines:Add("NO_VIZ")
		settings.cc.defines:Add("_WIN32_WINNT=0x0501")
	end

	-- Unicode support
	settings.cc.defines:Add("UNICODE") -- Windows headers
	settings.cc.defines:Add("_UNICODE") -- C-runtime

	-- Required libs
	settings.link.libs:Add("gdi32")
	settings.link.libs:Add("user32")
	settings.link.libs:Add("ws2_32")
	settings.link.libs:Add("ole32")
	settings.link.libs:Add("shell32")
	settings.link.libs:Add("advapi32")

	GenerateCommonSettings(settings, conf, target_arch, compiler)

	-- Master server, version server and tools
	BuildEngineCommon(settings)

	-- Add requirements for Server & Client
	BuildGameCommon(settings)

	-- Server
	local server_settings = settings:Copy()
	BuildServer(server_settings)

	-- Client
	settings.link.libs:Add("opengl32")
	settings.link.libs:Add("winmm")
	settings.link.libs:Add("imm32")
	BuildClient(settings)

	-- Content
	BuildContent(settings, target_arch, conf)
end

function SharedCommonFiles()
	-- Shared game files, generate only once

	if not shared_common_files then
		local network_source = ContentCompile("network_source", "generated/protocol.cpp")
		local network_header = ContentCompile("network_header", "generated/protocol.h")
		AddDependency(network_source, network_header, "src/engine/shared/protocol.h")

		shared_common_files = {network_source}
	end

	return shared_common_files
end

function SharedServerFiles()
	-- Shared server files, generate only once

	if not shared_server_files then
		local server_content_source = ContentCompile("server_content_source", "generated/server_data.cpp")
		local server_content_header = ContentCompile("server_content_header", "generated/server_data.h")
		AddDependency(server_content_source, server_content_header)
		shared_server_files = {server_content_source}
	end

	return shared_server_files
end

function SharedClientFiles()
	-- Shared client files, generate only once

	if not shared_client_files then
		local client_content_source = ContentCompile("client_content_source", "generated/client_data.cpp")
		local client_content_header = ContentCompile("client_content_header", "generated/client_data.h")
		AddDependency(client_content_source, client_content_header)
		shared_client_files = {client_content_source}
	end

	return shared_client_files
end

function BuildEngineCommon(settings)
	settings.link.extrafiles:Merge(Compile(settings, Collect("src/engine/shared/*.cpp", "src/base/*.c")))
end

function BuildGameCommon(settings)
	settings.link.extrafiles:Merge(Compile(settings, Collect("src/game/*.cpp"), SharedCommonFiles()))
end


function BuildClient(settings, family, platform)
	local client = Compile(settings, Collect("src/engine/client/*.cpp"))

	local game_client = Compile(settings, CollectRecursive("src/game/client/*.cpp"), SharedClientFiles())
	local game_editor = Compile(settings, Collect("src/game/editor/*.cpp"))

	Link(settings, "teeworlds", libs["md5"], libs["json"], client, game_client, game_editor)
end

function BuildServer(settings, family, platform)
	local server = Compile(settings, Collect("src/engine/server/*.cpp"))

	local game_server = Compile(settings, CollectRecursive("src/game/server/*.cpp"), SharedServerFiles())

	return Link(settings, "teeworlds_srv", libs["md5"], server, game_server)
end

function BuildContent(settings, arch, conf)
	local content = {}
	table.insert(content, CopyToDir(settings.link.Output(settings, "data"), CollectRecursive(content_src_dir .. "*.txt", content_src_dir .. "*.map", content_src_dir .. "*.rules", content_src_dir .. "*.json")))
	if family == "windows" then
		if arch == "x86_64" then
			_arch = "64"
		else
			_arch = "32"
		end
	end
	PseudoTarget(settings.link.Output(settings, "content") .. settings.link.extension, content)
end

-- create all targets for specified configuration & architecture
function GenerateSettings(conf, arch, builddir, compiler, headless)
	local settings = NewSettings()

	-- Set compiler if explicitly requested
	if compiler == "gcc" then
		SetDriversGCC(settings)
	elseif compiler == "clang" then
		SetDriversClang(settings)
	elseif compiler == "cl" then
		SetDriversCL(settings)
	else
		-- apply compiler settings
		config.compiler:Apply(settings)
		compiler = config.compiler.driver
	end

	if conf == "debug" then
		settings.debug = 1
		settings.optimize = 0
		settings.cc.defines:Add("CONF_DEBUG")
	else
		settings.debug = 0
		settings.optimize = 1
		settings.cc.defines:Add("CONF_RELEASE")
	end

	-- Generate object files in {builddir}/objs/
	settings.cc.Output = function (settings_, input)
		-- strip
		input = input:gsub("^src/", "")
		input = input:gsub("^" .. generated_src_dir .. "/", "")
		return PathJoin(PathJoin(builddir, "objs"), PathBase(input))
	end

	-- Build output files in {builddir}
	settings.link.Output = function (settings_, input)
		return PathJoin(builddir, PathBase(input) .. settings_.config_ext)
	end

	settings.cc.includes:Add("src")
	settings.cc.includes:Add(generated_src_dir)

	if family == "windows" then
		GenerateWindowsSettings(settings, conf, arch, compiler)
	elseif family == "unix" then
		if platform == "macosx" then
			GenerateMacOSXSettings(settings, conf, arch, compiler)
		elseif platform == "solaris" then
			GenerateSolarisSettings(settings, conf, arch, compiler)
		else -- Linux, BSD
			GenerateLinuxSettings(settings, conf, arch, compiler)
		end
	end

	return settings
end

-- String formatting with named parameters, by RiciLake http://lua-users.org/wiki/StringInterpolation
function interp(s, tab)
	return (s:gsub('%%%((%a%w*)%)([-0-9%.]*[cdeEfgGiouxXsq])',
			function(k, fmt)
				return tab[k] and ("%"..fmt):format(tab[k]) or '%('..k..')'..fmt
			end))
end

function CopyToDir(dst, ...)
	local output = {}
	for filename in TableWalk({...}) do
		table.insert(output, CopyFile(PathJoin(dst, string.sub(filename, string.len(content_src_dir)+1)), filename))
	end
	return output
end

function split(str, sep)
	local vals = {}
	str:gsub("([^,]+)", function(val) table.insert(vals, val) end)
	return vals
end

-- Supported archtitectures: x86, amd64, ppc, ppc64
if ScriptArgs['arch'] then
	archs = split(ScriptArgs['arch'])
else
	if arch == "ia32" then
		archs = {"x86"}
	elseif arch == "ia64" or arch == "amd64" then
		archs = {"x86_64"}
	else
		archs = {arch}
	end
end

if ScriptArgs['conf'] then
	confs = split(ScriptArgs['conf'])
else
	confs = {"debug"}
end

if ScriptArgs['compiler'] then
	compiler = ScriptArgs['compiler']
else
	compiler = nil
end

if ScriptArgs['builddir'] then
	builddir = ScriptArgs['builddir']
end

targets = {client="teeworlds", server="teeworlds_srv",
           content="content"}

subtargets = {}
for t, cur_target in pairs(targets) do
	subtargets[cur_target] = {}
end
for a, cur_arch in ipairs(archs) do
	for c, cur_conf in ipairs(confs) do
		cur_builddir = interp(builddir, {platform=family, arch=cur_arch, target=cur_target, conf=cur_conf, compiler=compiler})
		local settings = GenerateSettings(cur_conf, cur_arch, cur_builddir, compiler, headless)
		for t, cur_target in pairs(targets) do
			table.insert(subtargets[cur_target], PathJoin(cur_builddir, cur_target .. settings.link.extension))
		end
	end
end

for cur_name, cur_target in pairs(targets) do
	-- Supertarget for all configurations and architectures of that target
	PseudoTarget(cur_name, subtargets[cur_target])
end

PseudoTarget("game", "client", "server", "content")
DefaultTarget("game")
