// Copyright (c) 2009, Bo Zhou<http://jedimaster.cnblogs.com>
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the organization to which the authors belong nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY AUTHORS AND CONTRIBUTORS ''AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL AUTHORS AND CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#include "PRT.h"

#include <boost/scoped_array.hpp>
#include <iostream>
#include <zlib.h>

using namespace std;

struct Header
{
	char MagicNumber[8];
	int Len;
	char Signature[32];
	int Version;
	long long Count;
};

struct Channel
{
	char Name[32];
	int DataType;
	int Arity;
	int Offset;
};

void PRT::AddChannel(const std::string& Attr, const DataType DT, const int Arity, const Array& Data)
{
	if( Attr == "" || DT == kUNKNOWN || Arity == 0 || Data.get() == NULL )
		return;

	PRTChannel Ch;
	Ch.Arity = Arity;
	Ch.DT = DT;
	Ch.Data = Data;

	mCh.insert( std::make_pair(Attr, Ch) );
}

bool PRT::SaveToFile(const char* Path)
{
	if( mCount == 0 )
		return false;
		
	//Create file to write
	FILE* FP = fopen(Path, "wb");
	if( FP == NULL )
		return false;
	
	//Initialize the header
	Header H;
	memset(&H,0,sizeof(Header));
	H.MagicNumber[0] = 192;
	H.MagicNumber[1] = 'P';
	H.MagicNumber[2] = 'R';
	H.MagicNumber[3] = 'T';
	H.MagicNumber[4] = '\r';
	H.MagicNumber[5] = '\n';
	H.MagicNumber[6] = 26;
	H.MagicNumber[7] = '\n';
	H.Len = 56;
	sprintf(H.Signature,"Extensible Particle Format");
	H.Version = 1;
	H.Count = mCount;

	fwrite(&H,sizeof(Header),1,FP);
	int RESERVED = 4;
	fwrite(&RESERVED,sizeof(int),1,FP);

	fflush(FP);
	
	try
	{
		//We need double sized memory space.
		size_t TotalSize = 0;
		size_t SingleSize = 0;
		for( std::map<std::string, PRTChannel>::iterator itr = mCh.begin(); itr != mCh.end(); ++itr)
		{
			const DataType CurrDT = itr->second.DT;
			const int Arity = itr->second.Arity;

			if( CurrDT == kINT32 || CurrDT == kFLOAT32 || CurrDT == kUINT32 )
				SingleSize += 4*Arity;
			else
				SingleSize += 8*Arity;
		}
		TotalSize = SingleSize*mCount;

		boost::shared_array<Bytef> Cache( new Bytef[SingleSize] );
		boost::shared_array<Bytef> OutputBuf( new Bytef[TotalSize*2] );

		for( long long i=0; i<mCount; ++i )
		{
			size_t Offset = 0;
			for( std::map<std::string, PRTChannel>::iterator itr = mCh.begin(); itr != mCh.end(); ++itr)
			{
				const DataType CurrDT = itr->second.DT;
				const int Arity = itr->second.Arity;
				const char* p = itr->second.Data.get();
				size_t Len = 0;

				if( CurrDT == kINT32 || CurrDT == kFLOAT32 || CurrDT == kUINT32 )
					Len = 4*Arity;
				else
					Len = 8*Arity;

				memcpy( Cache.get() + Offset, p + i*Len, Len );

				Offset += Len;
			}

			memcpy( OutputBuf.get()+i*SingleSize, Cache.get(), SingleSize );
		}
		
		//Compress data
		uLongf ZSrcBufLen = TotalSize;
		Bytef* ZSrcBuf = (Bytef*)OutputBuf.get();

		uLongf ZDstBufLen = ZSrcBufLen;
		Bytef* ZDstBuf = ZSrcBuf + ZSrcBufLen;
	

		int ZR = compress2(ZDstBuf,&ZDstBufLen,ZSrcBuf,ZSrcBufLen,6);
		if( ZR != Z_OK )
		{
			fclose(FP);
			remove(Path);
			return false;
		}

		//Write channels information
		int ChanNumer = 4;
		fwrite(&ChanNumer,sizeof(int),1,FP);
		int ChanLen = 44;
		fwrite(&ChanLen,sizeof(int),1,FP);
		fflush(FP);
		
		int Offset = 0;
		for( std::map<std::string, PRTChannel>::iterator itr = mCh.begin(); itr != mCh.end(); ++itr)
		{
			Channel Ch;
			memset(&Ch,0,sizeof(Channel));

			Ch.Arity = itr->second.Arity;
			Ch.DataType = (int)itr->second.DT;
			strncpy(Ch.Name, itr->first.c_str(), 31);
			Ch.Offset = Offset;

			const DataType CurrDT = itr->second.DT;
			if( CurrDT == kINT32 || CurrDT == kFLOAT32 || CurrDT == kUINT32 )
				Offset += 4*Ch.Arity;
			else
				Offset += 8*Ch.Arity;

			fwrite(&Ch,sizeof(Channel),1,FP);
		}

		fwrite(ZDstBuf,ZDstBufLen,1,FP);

		fflush(FP);
		fclose(FP);
	}catch(...)
	{
		fclose(FP);
		remove(Path);
		return false;
	}
	
	return true;
}
