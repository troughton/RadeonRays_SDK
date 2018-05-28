
/**********************************************************************
Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
********************************************************************/
#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdlib.h>

typedef enum BufferType {
    BufferTypeRead = 0x1,
    BufferTypeWrite = 0x2,
    BufferTypePinned = 0x4
} BufferType;

typedef enum MapType {
    MapTypeRead = 0x1,
    MapTypeWrite = 0x2
} MapType;

typedef enum DeviceType {
    DeviceTypeUnknown,
    DeviceTypeGpu,
    DeviceTypeCpu,
    DeviceTypeAccelerator
} DeviceType;

typedef enum SourceType {
    SourceTypeOpenCL          = (1 << 0),
    SourceTypeGLSL            = (1 << 1),
    SourceTypeCL_SPIRV        = (1 << 2),
    SourceTypeGLSL_SPIRV      = (1 << 3),

    SourceTypeHostNative      = (1 << 4),
    SourceTypeAccelNative     = (1 << 5),
} SourceType;

// Calc device specification
typedef struct CalcDeviceSpec { // NOT layout compatible with the C++ struct.
    char const* name;
    char const* vendor;

    DeviceType  type;
    SourceType  sourceTypes;

    uint32_t min_alignment;
    uint32_t max_num_queues;

    size_t global_mem_size;
    size_t local_mem_size;
    size_t max_alloc_size;
    size_t max_local_size;

    bool has_fp16;
} CalcDeviceSpec;

typedef struct CalcDevice CalcDevice;
typedef struct CalcBuffer CalcBuffer;
typedef struct CalcEvent CalcEvent;
typedef struct CalcExecutable CalcExecutable;
typedef struct CalcFunction CalcFunction;

typedef struct CalcFunctionPointers {
    uint32_t (*GetDeviceCount)();
    CalcDeviceSpec (*GetDeviceSpec)(uint32_t index);
    CalcDevice* (*CreateDevice)(uint32_t index);
    void (*DeleteDevice)(CalcDevice* device);

    CalcBuffer* (*DeviceCreateBuffer)(CalcDevice const* device, size_t size, uint32_t flags, void* initialData);
    void (*DeviceDeleteBuffer)(CalcDevice const* device, CalcBuffer* buffer);

    // Data movement
    // Calls are blocking if passed nullptr for an event, otherwise use Event to sync
    void (*DeviceReadBuffer)(CalcDevice const* device, CalcBuffer const* buffer, uint32_t queue, size_t offset, size_t size, void* dst, CalcEvent** e);
    void (*DeviceWriteBuffer)(CalcDevice const* device, CalcBuffer const* buffer, uint32_t queue, size_t offset, size_t size, void* src, CalcEvent** e);

    // Buffer mapping
    // Calls are blocking if passed nullptr for an event, otherwise use Event to sync
    void (*DeviceMapBuffer)(CalcDevice const* device, CalcBuffer const* buffer, uint32_t queue, size_t offset, size_t size, uint32_t map_type, void** mapdata, CalcEvent** e);
    void (*DeviceUnmapBuffer)(CalcDevice const* device, CalcBuffer const* buffer, uint32_t queue, void* mapdata, CalcEvent** e);

    CalcFunction* (*ExecutableCreateFunction)(CalcExecutable* executable, char const* name);
    void (*ExecutableDeleteFunction)(CalcExecutable* executable, CalcFunction *function);
    
    // Kernel compilation
    CalcExecutable* (*DeviceCompileExecutableSource)(CalcDevice const* device, char const* source_code, size_t size, char const* options);
    CalcExecutable* (*DeviceCompileExecutableBinary)(CalcDevice const* device, uint8_t const* binary_code, size_t size, char const*  options);
    CalcExecutable* (*DeviceCompileExecutableFile)(CalcDevice const* device, char const* filename,
                                            char const** headernames,
                                            int numheaders, char const* options);

    void (*DeviceDeleteExecutable)(CalcDevice const* device, CalcExecutable* executable);

    // Executable management
    size_t (*DeviceGetExecutableBinarySize)(CalcDevice const* device, CalcExecutable const* executable);
    void (*DeviceGetExecutableBinary)(CalcDevice const* device, CalcExecutable const* executable, uint8_t* binary);

    // Execution
    // Calls are blocking if passed nullptr for an event, otherwise use Event to sync
    void (*DeviceExecute)(CalcDevice const* device, CalcFunction const* func, uint32_t queue, size_t global_size, size_t local_size, CalcEvent** e);

    // Events handling
    void (*DeviceWaitForEvent)(CalcDevice const* device, CalcEvent* e);
    void (*DeviceWaitForMultipleEvents)(CalcDevice const* device, CalcEvent** e, size_t num_events);
    void (*DeviceDeleteEvent)(CalcDevice const* device, CalcEvent* e);

    // Queue management functions
    void (*DeviceFlush)(CalcDevice const* device, uint32_t queue);
    void (*DeviceFinish)(CalcDevice const* device, uint32_t queue);

    bool (*DeviceEventIsComplete)(CalcDevice const* device, CalcEvent *const event);
    
    // Argument setters
    void (*FunctionSetArg)(CalcFunction *function, uint32_t idx, size_t arg_size, void* arg);
    void (*FunctionSetBuffer)(CalcFunction *function, uint32_t idx, CalcBuffer const* arg);
    void (*FunctionSetSharedMemory)(CalcFunction *function, uint32_t idx, size_t size);
} CalcFunctionPointers;

void SetCalcFunctionPointers(CalcFunctionPointers functionPointers);

#if defined(__cplusplus)
}
#endif
