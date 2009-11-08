import os

Env = Environment()
Env['ENV']['PATH'] = os.environ['PATH']
Env['ENV']['LIB'] = os.environ['LIB']
Env['ENV']['INCLUDE'] = os.environ['INCLUDE']

BoostIncPath = os.environ['BOOST_ROOT']
if( BoostIncPath == '' ) :
	print "ERROR : Can't found system variable BOOST_ROOT, you must setup it."
	exit();

CudaIncPath = os.environ['CUDA_INC_PATH']
if( CudaIncPath == '' ) :
	print "ERROR : You have to install CUDA Toolkit."
	exit();
	
STANNIncPath = Dir('E:/SDK/Inc/stann')

RPSLibPath = Dir('C:/Program Files/Pixar/RenderManProServer-15.0/lib')
RPSIncPath = Dir('C:/Program Files/Pixar/RenderManProServer-15.0/include')

GSLIncPath = Dir('E:/SDK/Inc')
GSLLibPath = Dir('E:/SDK/Lib/x64')

ThrustIncPath = Dir('E:/SDK/Inc')

CC = '/DWIN64 /D_WIN64 /DUNICODE /D_UNICODE /Ox /EHa /GF /GL /GR /MD /openmp'

Libs = ['libprman','libgsl64','libgslcblas64']
SourceFiles = ['Approx.cpp','DPR.cpp','EPR.cpp','OPR.cpp','PPR.cpp','PR.cpp']

Env.SharedLibrary(target='PR',source=SourceFiles,CPPPATH=[RPSIncPath,GSLIncPath,BoostIncPath,STANNIncPath],LIBPATH=[RPSLibPath,GSLLibPath],LIBS=Libs,CCFLAGS=CC)
