######################################################
# SCONS Build Script for M2K <http://code.google.com/p/m2k>
#
# Please modify the path related to your configuration.
#
# WARNING : For Windows Only now ! Please install Python 2.5 (or newer) and SCons 
# to compile me in Visual Studio command prompt.
######################################################

print "M2K SCons Build Script\nWARNING : You should run this under Visual Studio prompt\n\n"

import os

Env = Environment()
Env['ENV']['PATH'] = os.environ['PATH']
Env['ENV']['LIB'] = os.environ['LIB']
Env['ENV']['INCLUDE'] = os.environ['INCLUDE']

######################################################
# Compiler parameters for different OS and different Maya

W32M32CP = '/DWIN32 /D_WIN32 /DUNICODE /D_UNICODE /DNT_PLUGIN /DREQUIRE_IOSTREAM /EHa /MD /Wp64 /nologo'
W64M32CP = '/DWIN64 /D_WIN64 /DUNICODE /D_UNICODE /DNT_PLUGIN /DREQUIRE_IOSTREAM /EHa /MD /Wp64 /nologo'

######################################################

BoostPath = os.environ['BOOST_ROOT']
if( BoostPath == '' ) :
	print "ERROR : Can't found system variable BOOST_ROOT, you must setup it."

MayaIncPath = Dir('C:/Program Files (x86)/Autodesk/Maya2008/include')
MayaLibPath = Dir('C:/Program Files (x86)/Autodesk/Maya2008/lib')

ZLIBIncPath = Dir('E:/SDK/Inc')
ZLIBLibPath = Dir('E:/SDK/Lib/X86')

ZLIBLibTarget = 'libzlib'

######################################################

IncPath = []
IncPath.append(BoostPath)
IncPath.append(MayaIncPath)
IncPath.append(ZLIBIncPath)

LibPath = []
LibPath.append(MayaLibPath)
LibPath.append(ZLIBLibPath);

######################################################

Libs = ['Foundation', 'OpenMaya', 'OpenMayaFX', 'libHalf', ZLIBLibTarget]

SourceFiles = ['M2K.cpp','PRT.cpp']

Env.SharedLibrary(target='M2K', SHLIBSUFFIX='.mll', source=SourceFiles, CPPPATH = IncPath, LIBPATH = LibPath, LIBS = Libs, CCFLAGS = W32M32CP)
