#pragma once
#include "MXml.h"

#include "MDebug.h"
#include "MZFileSystem.h"
#include <vector>

#include "RNameSpace.h"

_NAMESPACE_REALSPACE2_BEGIN

struct RAniEventInfo
{
	int BeginFrame;
	char Filename[128];
};

struct RAniNameEventSet
{
	std::vector<RAniEventInfo> AniNameEventSet;
	char AnimationName[128];
};

struct RAniIDEventSet
{
	int ID;
	std::vector<RAniNameEventSet> AniIDEventSet;

	const RAniNameEventSet* GetAniNameEventSet(const char* AnimationName) const;
};

struct RAniEventMgr
{
	std::vector<RAniIDEventSet> AniEventMgr;

	bool ReadXml(class MZFileSystem* FileSystem, const char* Filename);
	const RAniIDEventSet* GetAniIDEventSet(int ID) const;
};

_NAMESPACE_REALSPACE2_END
