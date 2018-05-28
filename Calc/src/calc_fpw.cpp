#include "calc_fp.h"
#include "calc_fpw.h"
#include "device_fpw.h"
#include "except_fpw.h"

CalcFunctionPointers g_FunctionPointers;

namespace Calc
{

    CalcFunctionPointer::CalcFunctionPointer()
    {
    }

    CalcFunctionPointer::~CalcFunctionPointer()
    {
    }

    void CalcFunctionPointer::SetFunctionPointers(CalcFunctionPointers functionPointers) {
        g_FunctionPointers = functionPointers;
    }

    // Enumerate devices 
    std::uint32_t CalcFunctionPointer::GetDeviceCount() const
    {
        return nullptr != g_FunctionPointers.GetDeviceCount ? g_FunctionPointers.GetDeviceCount() : 0;
    }

    // Get i-th device spec
    void CalcFunctionPointer::GetDeviceSpec( std::uint32_t idx, DeviceSpec& spec ) const
    {
        if ( idx < GetDeviceCount() )
        {
            CalcDeviceSpec cSpec = g_FunctionPointers.GetDeviceSpec(idx);

            spec.name = cSpec.name;
            spec.vendor = cSpec.vendor;
            spec.type = (DeviceType)cSpec.type;
            spec.sourceTypes = (SourceType)cSpec.sourceTypes;

            spec.min_alignment = cSpec.min_alignment;
            spec.max_num_queues = cSpec.max_num_queues;

            spec.global_mem_size = cSpec.global_mem_size;
            spec.local_mem_size = cSpec.local_mem_size;
            spec.max_alloc_size = cSpec.max_alloc_size;
            spec.max_local_size = cSpec.max_local_size;

            spec.has_fp16 = cSpec.has_fp16;
        }
        else
        {
            throw ExceptionFPw( "Index is out of bounds" );
        }
    }

    // Create the device with specified index
    Device* CalcFunctionPointer::CreateDevice( std::uint32_t idx ) const
    {
        Device* toReturn = nullptr;

        if ( idx < GetDeviceCount() )
        {
            CalcDevice *device = g_FunctionPointers.CreateDevice(idx);

            DeviceSpec spec;
            GetDeviceSpec(idx, spec);
            
            toReturn = new DeviceFPw(device, spec);

            return toReturn;
        }

        else
        {
            throw ExceptionFPw( "Index is out of bounds" );
        }

        return toReturn;
    }

    // Delete the device
    void CalcFunctionPointer::DeleteDevice( Device* device )
    {
        delete device;
    }
}
