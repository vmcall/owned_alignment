#pragma once
#include "nt.hpp"

struct REQUEST_STRUCT
{
	IO_COMPLETION_ROUTINE* old_routine;
	void* old_context;
	uint32_t output_buffer_length;
	void* system_buffer;
};

// DATA READ FROM ALLOCATED POOL IN COMPLETION HOOKS
template <class T>
struct callback_data
{
	REQUEST_STRUCT*				request;
	uint32_t					buffer_length;
	T*	buffer;
	IO_COMPLETION_ROUTINE*		old_routine;
	void*						old_context;
};

namespace ioctl_helper
{
	// SET UP COMPLETION CALLBACK, THAT IS CALLED BEFORE IOCTL RETURNS VALUE TO CALLER
	__forceinline void set_completion_callback(IRP* irp, IO_STACK_LOCATION* io_stack_location, IO_COMPLETION_ROUTINE* callback)
	{
		// RUN CALLBACK ON SUCCESS
		io_stack_location->Control = SL_INVOKE_ON_SUCCESS;
		auto old_context = io_stack_location->Context;
		io_stack_location->Context = ExAllocatePoolWithTag(NonPagedPool, sizeof(REQUEST_STRUCT), 0);

		auto request = reinterpret_cast<REQUEST_STRUCT*>(io_stack_location->Context);
		request->old_context = old_context;
		request->old_routine = io_stack_location->CompletionRoutine;
		request->system_buffer = irp->AssociatedIrp.SystemBuffer;
		request->output_buffer_length = io_stack_location->Parameters.DeviceIoControl.OutputBufferLength;

		// setup callback
		io_stack_location->CompletionRoutine = callback;
	}

	// READ CALLBACK DATA FROM ALLOCATED POOl TO STACK, THEN FREES POOL
	template <class T>
	__forceinline callback_data<T> fetch_callback_data(void* context)
	{
		callback_data<T> result{};

		result.request = reinterpret_cast<REQUEST_STRUCT*>(context);
		result.buffer_length = result.request->output_buffer_length;
		result.buffer = reinterpret_cast<T*>(result.request->system_buffer);
		result.old_routine = result.request->old_routine;
		result.old_context = result.request->old_context;
		nt_fn::ExFreePoolWithTag(context, 0);

		return result;
	}
}