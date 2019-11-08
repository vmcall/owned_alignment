#include "disk_hooker.hpp"

extern "C" int64_t DriverEntry(void* start_context)
{
	disk_hooker::hook_disk_driver();
}