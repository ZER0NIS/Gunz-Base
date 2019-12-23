#pragma once

#include "MCRC32.h"

class MMatchCRC32XORCache
{
private :
	DWORD m_dwXOR;
	DWORD m_dwCRC32;

public :
	MMatchCRC32XORCache()	{ Reset(); }
	~MMatchCRC32XORCache()	{}

	__forceinline	void CRC32XOR( DWORD dwData ) {

		m_dwXOR ^= dwData;
		MCRC32::AccumulateCRC32(m_dwCRC32, (BYTE*)&dwData, sizeof(dwData));

	}

	__forceinline	const DWORD GetXOR() const			{ return m_dwXOR; }
	__forceinline	const DWORD GetCRC32() const		{ return m_dwCRC32; }
	void						Reset()					{ m_dwXOR=0; m_dwCRC32=0xFFFFFFFF; }
};