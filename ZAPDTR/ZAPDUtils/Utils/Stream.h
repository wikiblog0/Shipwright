#pragma once

#include <cstdint>
#include <memory>

#ifdef _MSC_VER
#define BSWAP16 _byteswap_ushort
#define BSWAP32 _byteswap_ulong
#define BSWAP64 _byteswap_uint64
#else
#define BSWAP16 __builtin_bswap16
#define BSWAP32 __builtin_bswap32
#define BSWAP64 __builtin_bswap64
#endif

enum class SeekOffsetType
{
	Start,
	Current,
	End
};

class Stream
{
public:
	virtual ~Stream() = default;
	virtual uint64_t GetLength() = 0;
	uint64_t GetBaseAddress() { return baseAddress; }

	virtual void Seek(int32_t offset, SeekOffsetType seekType) = 0;

	virtual std::unique_ptr<char[]> Read(size_t length) = 0;
	virtual void Read(const char* dest, size_t length) = 0;
	virtual int8_t ReadByte() = 0;

	virtual void Write(char* destBuffer, size_t length) = 0;
	virtual void WriteByte(int8_t value) = 0;

	virtual void Flush() = 0;
	virtual void Close() = 0;

protected:
	uint64_t baseAddress;
};