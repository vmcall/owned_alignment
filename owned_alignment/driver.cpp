#include "driver.hpp"

portable_executable driver::get_pe()
{
	return portable_executable(this->base_address());
}

uint8_t* driver::base_address()
{
	return reinterpret_cast<uint8_t*>(this->get_object()->DriverStart);
}

uint64_t driver::size()
{
	return this->get_object()->Size;
}

bool driver::valid()
{
	return this->get_object() != nullptr;
}

DRIVER_OBJECT* driver::get_object()
{
	return driver_obj.get();
}

PDRIVER_DISPATCH* driver::major_function(int index)
{
	return &this->get_object()->MajorFunction[index];
}