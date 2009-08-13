######################################################
# SCONS Build Script for M2K <http://code.google.com/p/m2k>
#
# Please modify the path related to your configuration.
#
# WARNING : For Windows Only now ! Please install Python 2.5 (or newer) and scons to compile me in Visual Studio command prompt.
######################################################

import os

Env = Environment()
Env['ENV']['PATH'] = os.environ['PATH']
Env['ENV']['LIB'] = os.environ['LIB']
Env['ENV']['INCLUDE'] = os.environ['INCLUDE']

######################################################

CompilerDefines = '/DWIN32 /D_WIN32 /DUNICODE /D_UNICODE /DNT_PLUGIN /DREQUIRE_IOSTREAM /EHa /MD '

######################################################

BOOSTPath = Dir('C:/boost/boost_1_39_0')

MayaIncPath = Dir('C:/Program Files/Autodesk/Maya2009/include')
MayaLibPath = Dir('C:/Program Files/Autodesk/Maya2009/lib')

ZLIBIncPath = Dir('E:/SDK/Inc')
ZLIBLibPath = Dir('E:/SDK/Lib/X64')

ZLIBLibTarget = 'zlib64'

######################################################

IncPath = []
IncPath.append(BOOSTPath)
IncPath.append(MayaIncPath)
IncPath.append(ZLIBIncPath)

LibPath = []
LibPath.append(MayaLibPath)
LibPath.append(ZLIBLibPath);

######################################################

Libs = ['Foundation', 'OpenMaya', 'OpenMayaFX', ZLIBLibTarget]

SourceFiles = ['M2K.cpp','PRT.cpp']

Env.SharedLibrary(target='M2K', SHLIBSUFFIX='.mll', source=SourceFiles, CPPPATH = IncPath, LIBPATH = LibPath, LIBS = Libs, CCFLAGS = CompilerDefines)
