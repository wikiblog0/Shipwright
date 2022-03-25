#pragma once

#include <vector>
#include <string>
#include "Resource.h"
#include "Vec2f.h"
#include "Vec3f.h"
#include "Color3b.h"

namespace Ship
{
	class DisplayListV0 : public ResourceFile
	{
	public:
		uint64_t ReadCommand(BinaryReader* reader);
		void ParseFileBinary(BinaryReader* reader, Resource* res) override;
	};

    class DisplayList : public Resource
    {
    public:
		std::vector<uint64_t> instructions;
    };
}