import os

Env = Environment()
Env['ENV']['PATH'] = os.environ['PATH']
Env['ENV']['LIB'] = os.environ['LIB']
Env['ENV']['INCLUDE'] = os.environ['INCLUDE']

##################
# Please change this path according your configuration

BOOSTPath = Dir('C:\\boost\\boost_1_39_0')

MayaIncPath = Dir('C:\\Program Files\\Autodesk\\Maya2009\\include')
MayaLibPath = Dir('C:\\Program Files\\Autodesk\\Maya2009\\lib')

ZLIBIncPath = Dir('E:\\SDK\\Inc')
ZLIBLibPath = Dir('E:\\SDK\\Lib\\X64')

##################

IncPath = []
IncPath.append(BOOSTPath)
IncPath.append(MayaIncPath)
IncPath.append(ZLIBIncPath)

LibPath = []
LibPath.append(MayaLibPath)
LibPath.append(ZLIBLibPath);

##################

PluginDefines = '/DWIN32 /D_WIN32 /DUNICODE /D_UNICODE /DNT_PLUGIN /DREQUIRE_IOSTREAM'
CCDefines = ' /EHsc'

Libs = ['zlib1', 'Foundation', 'OpenMaya', 'OpenMayaFX']

SourceFiles = ['M2K.cpp','PRT.cpp']

Env.SharedLibrary(target='M2K', SHLIBSUFFIX='.mll', source=SourceFiles, CPPPATH = IncPath, LIBPATH = LibPath, LIBS = Libs, CCFLAGS = PluginDefines + CCDefines)
