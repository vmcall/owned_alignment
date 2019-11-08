#pragma once
#include <cstdint>

namespace shellcode_generator
{
	__forceinline void call_proxy(uint8_t* buffer, void* fn)
	{
		uint8_t trampoline_data[]{
			0x58,														// pop rax
			0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // movabs rax,0x0000000000000000
			0xFF, 0xE0 };												// jmp rax


		*reinterpret_cast<uint64_t*>(trampoline_data + 3) = reinterpret_cast<uint64_t>(fn);

		// MEMCPY
		for (size_t i = 0; i < sizeof(trampoline_data); i++)
			buffer[i] = trampoline_data[i];
	};
}