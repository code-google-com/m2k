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

#ifndef PRT_H
#define PRT_H

#include <boost/shared_array.hpp>
#include <string>
#include <map>

enum DataType
{
	kUNKNOWN,
	kINT32 = 1,
	kINT64 = 2,
	kFLOAT32 = 4,
	kFLOAT64 = 5,
	kUINT32 = 7,
	kUINT64 = 8
};

typedef boost::shared_array<char> Array;

class PRTChannel
{
public:
	PRTChannel() : Arity(0), DT(kUNKNOWN)
	{
	}
	~PRTChannel()
	{
	}
	int Arity;
	DataType DT;
	Array Data;
};

typedef std::map<std::string, PRTChannel> PRTChannelDict;

class PRT
{
public:
	
	PRT(const long long Count = 0);
	~PRT();
	
	bool SaveToFile(const char*);
	bool ReadFromFile(const char*);

	void AddChannel(const std::string&, const DataType, const int, const Array&);
	void GetChannels(PRTChannelDict&) const;
	long long GetCount() const;

	void Clear();

private:
	long long mCount;
	PRTChannelDict mCh;
};

#endif
