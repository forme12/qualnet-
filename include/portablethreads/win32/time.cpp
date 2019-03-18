/*  Copyright (c) October 2005 Jean Gressmann (jsg@rz.uni-potsdam.de)
 *
 *  This is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version. 
 * 
 *	This file is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this file; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
 
#include <limits>
#include <cassert>

#ifdef max
#	undef max
#endif

#ifdef _MSC_VER
#	pragma warning(disable:4244) // uint64 -> unsigned conversion
#endif

namespace
{
	// this is suppposed to set the ganularity of sleep to
	// about one ms
	class TimerPrecision
	{
	public:
		~TimerPrecision()
		{
			timeEndPeriod(1);
		}
		TimerPrecision()
		{
			timeBeginPeriod(1);
		}
	};
	
	TimerPrecision prec;
	
	LONGLONG countPerMicroSecond_;
	LONGLONG countPerMilliSecond_;

	int calcCountsPerMicroSecond()
	{
		LARGE_INTEGER t;
		const int ret = QueryPerformanceFrequency(&t);
		countPerMicroSecond_ = t.QuadPart / 1000000 /* 1 mio µs to a second */;		
		countPerMilliSecond_ = t.QuadPart / 1000 /* 1K ms to a second */;		
		return ret;
	}

	const int ok = calcCountsPerMicroSecond();	
}

namespace PortableThreads 
{
	void pt_second_sleep(unsigned time)
	{
		uint64 t = static_cast<uint64>(time)*1000;
		while(t > static_cast<uint64>(std::numeric_limits<DWORD>::max()))
		{
			t -= std::numeric_limits<DWORD>::max();
			Sleep(std::numeric_limits<DWORD>::max());
		}
		Sleep(t);
	}
	void pt_milli_sleep(unsigned time)
	{
		if(time > std::numeric_limits<DWORD>::max())
		{
			time -= std::numeric_limits<DWORD>::max();
			Sleep(std::numeric_limits<DWORD>::max());
		}
		Sleep(time);
	}
	void pt_micro_sleep(unsigned time)
	{
		assert(ok && "This platform does not have a high performance counter!");
		
		LARGE_INTEGER startat;	
		QueryPerformanceCounter(&startat);

		LARGE_INTEGER countto;
		countto.QuadPart = startat.QuadPart + time * countPerMicroSecond_;

		while((countto.QuadPart - startat.QuadPart) / countPerMilliSecond_ > 0)
		{
			Sleep(1);
			QueryPerformanceCounter(&startat);
		}
		while(startat.QuadPart < countto.QuadPart)
		{
			Sleep(0);
			QueryPerformanceCounter(&startat);
		}
	}

	void pt_yield()
	{
		Sleep(0);
	}

	///////////////////////////////////////////////////////////////////////
	// Time
	///////////////////////////////////////////////////////////////////////
	PTime::time_type PTime::stamp()
	{
		LARGE_INTEGER t;
		QueryPerformanceCounter(&t);
		return t.QuadPart;
	}

	PTime::time_type PTime::calculateFrequency()
	{
		LARGE_INTEGER t;
		if(QueryPerformanceFrequency(&t) == 0)
			return 0;
		
		return t.QuadPart;
	}
	const PTime::time_type PTime::frequency_ = PTime::calculateFrequency();
}

#ifdef _MSC_VER
#	pragma warning(disable:4244) // uint64 -> unsigned conversion
#endif


