"""Generate the Visual Studio 2010 solution and projects for QtClone.

Run from the repository root:

    python tools/gen_vs2010.py

The generator discovers framework sources automatically, so adding a .cpp under
src/core, src/gui or src/widgets only requires re-running it.
"""
import os

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

GUID = {
    'qtc': '{7A1F3C10-0001-4E8B-9C11-000000000001}',
    'qtcuic': '{7A1F3C10-0002-4E8B-9C11-000000000002}',
    'wfdesigner': '{7A1F3C10-0003-4E8B-9C11-000000000003}',
    'gallery': '{7A1F3C10-0004-4E8B-9C11-000000000004}',
    'wf_form': '{7A1F3C10-0005-4E8B-9C11-000000000005}',
}

LIBS = ('user32.lib;gdi32.lib;gdiplus.lib;comctl32.lib;comdlg32.lib;shell32.lib;'
        'ole32.lib;oleaut32.lib;shlwapi.lib;uxtheme.lib;imm32.lib;advapi32.lib')


def sources(*dirs):
    out = []
    for d in dirs:
        full = os.path.join(ROOT, d)
        for name in sorted(os.listdir(full)):
            if name.endswith('.cpp'):
                out.append(d.replace('/', '\\') + '\\' + name)
    return out


def vcxproj(name, kind, srcs, extra_items='', include_dirs='', subsystem='Windows',
            entry='mainCRTStartup', link_extra='', project_refs=None, console=False):
    out_dir = '$(SolutionDir)out\\$(Configuration)\\'
    libs = LIBS + ';' + link_extra + ';%(AdditionalDependencies)'

    cfg_props = []
    for cfg, debug in (('Debug', 'true'), ('Release', 'false')):
        cfg_props.append(
            '  <PropertyGroup Condition="\'$(Configuration)|$(Platform)\'==\'%s|Win32\'" Label="Configuration">\n'
            '    <ConfigurationType>%s</ConfigurationType>\n'
            '    <UseDebugLibraries>%s</UseDebugLibraries>\n'
            '    <PlatformToolset>v100</PlatformToolset>\n'
            '    <CharacterSet>Unicode</CharacterSet>\n'
            '  </PropertyGroup>\n' % (cfg, kind, debug))

    blocks = []
    for cfg, opt, rt in (('Debug', 'Disabled', 'MultiThreadedDebugDLL'),
                         ('Release', 'MaxSpeed', 'MultiThreadedDLL')):
        defines = ('WIN32;NDEBUG;UNICODE;_UNICODE;_CRT_SECURE_NO_WARNINGS' if cfg == 'Release'
                   else 'WIN32;_DEBUG;UNICODE;_UNICODE;_CRT_SECURE_NO_WARNINGS')
        entry_xml = ('      <EntryPointSymbol>%s</EntryPointSymbol>\n' % entry) if entry else ''
        blocks.append(
            '  <ItemDefinitionGroup Condition="\'$(Configuration)|$(Platform)\'==\'%s|Win32\'">\n'
            '    <ClCompile>\n'
            '      <PreprocessorDefinitions>%s;%%(PreprocessorDefinitions)</PreprocessorDefinitions>\n'
            '      <AdditionalIncludeDirectories>%s$(SolutionDir)include;$(SolutionDir)src\\gui;%%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>\n'
            '      <Optimization>%s</Optimization>\n'
            '      <RuntimeLibrary>%s</RuntimeLibrary>\n'
            '      <WarningLevel>Level3</WarningLevel>\n'
            '      <ExceptionHandling>Sync</ExceptionHandling>\n'
            '      <SDLCheck>false</SDLCheck>\n'
            '      <MultiProcessorCompilation>true</MultiProcessorCompilation>\n'
            '    </ClCompile>\n'
            '    <Link>\n'
            '      <SubSystem>%s</SubSystem>\n'
            '      <AdditionalDependencies>%s</AdditionalDependencies>\n'
            '      <GenerateDebugInformation>true</GenerateDebugInformation>\n'
            '%s    </Link>\n'
            '  </ItemDefinitionGroup>\n'
            % (cfg, defines, include_dirs, opt, rt, subsystem, libs, entry_xml))

    compiles = '\n'.join('    <ClCompile Include="%s" />' % s for s in srcs)
    refs = ''
    if project_refs:
        refs = '  <ItemGroup>\n' + '\n'.join(
            '    <ProjectReference Include="%s">\n'
            '      <Project>%s</Project>\n'
            '      <ReferenceOutputAssembly>false</ReferenceOutputAssembly>\n'
            '      <LinkLibraryDependencies>false</LinkLibraryDependencies>\n'
            '    </ProjectReference>' % (p, GUID[os.path.splitext(os.path.basename(p))[0]])
            for p in project_refs) + '\n  </ItemGroup>\n'

    xml = (
        '<?xml version="1.0" encoding="utf-8"?>\n'
        '<Project DefaultTargets="Build" ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">\n'
        '  <ItemGroup Label="ProjectConfigurations">\n'
        '    <ProjectConfiguration Include="Debug|Win32">\n'
        '      <Configuration>Debug</Configuration>\n'
        '      <Platform>Win32</Platform>\n'
        '    </ProjectConfiguration>\n'
        '    <ProjectConfiguration Include="Release|Win32">\n'
        '      <Configuration>Release</Configuration>\n'
        '      <Platform>Win32</Platform>\n'
        '    </ProjectConfiguration>\n'
        '  </ItemGroup>\n'
        '  <PropertyGroup Label="Globals">\n'
        '    <ProjectGuid>%s</ProjectGuid>\n'
        '    <RootNamespace>%s</RootNamespace>\n'
        '    <ProjectName>%s</ProjectName>\n'
        '  </PropertyGroup>\n'
        '  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.Default.props" />\n'
        '%s'
        '  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.props" />\n'
        '  <ImportGroup Label="ExtensionSettings" />\n'
        '  <ImportGroup Label="PropertySheets" Condition="\'$(Configuration)|$(Platform)\'==\'Debug|Win32\'">\n'
        '    <Import Project="$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props" Condition="exists(\'$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\')" Label="LocalAppDataPlatform" />\n'
        '  </ImportGroup>\n'
        '  <ImportGroup Label="PropertySheets" Condition="\'$(Configuration)|$(Platform)\'==\'Release|Win32\'">\n'
        '    <Import Project="$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props" Condition="exists(\'$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\')" Label="LocalAppDataPlatform" />\n'
        '  </ImportGroup>\n'
        '  <PropertyGroup Label="UserMacros" />\n'
        '  <PropertyGroup>\n'
        '    <OutDir>%s</OutDir>\n'
        '    <IntDir>$(SolutionDir)out\\obj\\$(ProjectName)\\$(Configuration)\\</IntDir>\n'
        '    <TargetName>%s</TargetName>\n'
        '  </PropertyGroup>\n'
        '%s'
        '  <ItemGroup>\n'
        '%s\n'
        '%s'
        '  </ItemGroup>\n'
        '%s'
        '  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.targets" />\n'
        '  <ImportGroup Label="ExtensionTargets" />\n'
        '</Project>\n'
        % (GUID[name], name, name, ''.join(cfg_props), out_dir, name, ''.join(blocks),
           compiles, extra_items, refs))
    return xml


def main():
    projects = {}
    projects['qtc'] = vcxproj('qtc', 'StaticLibrary',
                              sources('src\\core', 'src\\gui', 'src\\widgets'))

    lib_extra = '$(SolutionDir)out\\$(Configuration)\\qtc.lib'
    projects['qtcuic'] = vcxproj('qtcuic', 'Application',
                                 ['tools\\qtcuic\\qtcuic.cpp', 'tools\\uicgen\\uicgen.cpp'],
                                 include_dirs='$(SolutionDir)tools\\uicgen;',
                                 subsystem='Console', entry='', link_extra=lib_extra,
                                 project_refs=['qtc.vcxproj'])
    projects['wfdesigner'] = vcxproj('wfdesigner', 'Application',
                                     ['designer\\designer.cpp', 'tools\\uicgen\\uicgen.cpp'],
                                     link_extra=lib_extra, project_refs=['qtc.vcxproj'])
    projects['gallery'] = vcxproj('gallery', 'Application',
                                  ['examples\\gallery\\gallery.cpp'],
                                  link_extra=lib_extra, project_refs=['qtc.vcxproj'])

    custom_build = (
        '    <CustomBuild Include="examples\\form\\Form1.ui">\n'
        '      <FileType>Document</FileType>\n'
        '      <Message>qtcuic %(Identity)</Message>\n'
        '      <Command>if not exist "$(IntDir)" mkdir "$(IntDir)"\n'
        '"$(SolutionDir)out\\$(Configuration)\\qtcuic.exe" "%(FullPath)" -o "$(IntDir)ui_form1.h" -q</Command>\n'
        '      <Outputs>$(IntDir)ui_form1.h;%(Outputs)</Outputs>\n'
        '      <AdditionalInputs>$(SolutionDir)out\\$(Configuration)\\qtcuic.exe;%(AdditionalInputs)</AdditionalInputs>\n'
        '    </CustomBuild>\n')
    projects['wf_form'] = vcxproj('wf_form', 'Application',
                                  ['examples\\form\\main.cpp'],
                                  extra_items=custom_build,
                                  include_dirs='$(IntDir);',
                                  link_extra=lib_extra,
                                  project_refs=['qtc.vcxproj', 'qtcuic.vcxproj'])

    for name, text in projects.items():
        with open(os.path.join(ROOT, name + '.vcxproj'), 'w', encoding='utf-8-sig', newline='\r\n') as f:
            f.write(text)
        print('wrote', name + '.vcxproj')

    sln = ['', 'Microsoft Visual Studio Solution File, Format Version 11.00', '# Visual Studio 2010']
    order = ['qtc', 'qtcuic', 'wfdesigner', 'gallery', 'wf_form']
    for name in order:
        sln.append('Project("%s") = "%s", "%s.vcxproj", "%s"' % (GUID[name], name, name, GUID[name]))
        sln.append('EndProject')
    sln.append('Global')
    sln.append('\tGlobalSection(SolutionConfigurationPlatforms) = preSolution')
    sln.append('\t\tDebug|Win32 = Debug|Win32')
    sln.append('\t\tRelease|Win32 = Release|Win32')
    sln.append('\tEndGlobalSection')
    sln.append('\tGlobalSection(ProjectConfigurationPlatforms) = postSolution')
    for name in order:
        for cfg in ('Debug', 'Release'):
            sln.append('\t\t%s.%s|Win32.ActiveCfg = %s|Win32' % (GUID[name], cfg, cfg))
            sln.append('\t\t%s.%s|Win32.Build.0 = %s|Win32' % (GUID[name], cfg, cfg))
    sln.append('\tEndGlobalSection')
    sln.append('\tGlobalSection(SolutionProperties) = preSolution')
    sln.append('\t\tHideSolutionNode = FALSE')
    sln.append('\tEndGlobalSection')
    sln.append('EndGlobal')
    with open(os.path.join(ROOT, 'QtClone.sln'), 'w', encoding='utf-8-sig', newline='\r\n') as f:
        f.write('\n'.join(sln) + '\n')
    print('wrote QtClone.sln')


if __name__ == '__main__':
    main()
