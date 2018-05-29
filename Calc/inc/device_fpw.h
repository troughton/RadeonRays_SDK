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

#include "device.h"
#include "calc_fp.h"

#include <queue>

namespace Calc
{
    // Device implementation with CLW library
    class DeviceFPw : public Device
    {
    public:
        DeviceFPw(CalcDevice *const device, DeviceSpec spec);
        ~DeviceFPw();

        // Device overrides
        // Return specification of the device
        void GetSpec(DeviceSpec& spec) override;

        // Buffer creation and deletion
        Buffer* CreateBuffer(std::size_t size, std::uint32_t flags) override;
        Buffer* CreateBuffer(std::size_t size, std::uint32_t flags, void* initdata) override;
        void DeleteBuffer(Buffer* buffer) override;

        // Data movement
        void ReadBuffer(Buffer const* buffer, std::uint32_t queue, std::size_t offset, std::size_t size, void* dst, Event** e) const override;
        void WriteBuffer(Buffer const* buffer, std::uint32_t queue, std::size_t offset, std::size_t size, void* src, Event** e) override;

        // Buffer mapping 
        void MapBuffer(Buffer const* buffer, std::uint32_t queue, std::size_t offset, std::size_t size, std::uint32_t map_type, void** mapdata, Event** e) override;
        void UnmapBuffer(Buffer const* buffer, std::uint32_t queue, void* mapdata, Event** e) override;

        // Kernel compilation
        Executable* CompileExecutable(char const* source_code, std::size_t size, char const* options) override;
        Executable* CompileExecutable(std::uint8_t const* binary_code, std::size_t size, char const* options) override;
        Executable* CompileExecutable(char const* filename, char const** headernames, int numheaders, char const* options) override;

        void DeleteExecutable(Executable* executable) override;

        // Executable management
        size_t GetExecutableBinarySize(Executable const* executable) const override;
        void GetExecutableBinary(Executable const* executable, std::uint8_t* binary) const override;

        // Execution
        void Execute(Function const* func, std::uint32_t queue, size_t global_size, size_t local_size, Event** e) override;

        // Events handling
        void WaitForEvent(Event* e) override;
        void WaitForMultipleEvents(Event** e, std::size_t num_events) override;
        void DeleteEvent(Event* e) override;

        // Queue management functions
        void Flush(std::uint32_t queue) override;
        void Finish(std::uint32_t queue) override;

        // Parallel prims handling
        bool HasBuiltinPrimitives() const override { return false; }
        Primitives* CreatePrimitives() const override { return nullptr; }
        void DeletePrimitives(Primitives* prims) override { }

        Platform GetPlatform() const override { return Platform::kFunctionPointer; }

        CalcDevice *const GetDevice() const { return m_device; }
    private:
        CalcDevice *const m_device;
        DeviceSpec m_spec;
    };
}
