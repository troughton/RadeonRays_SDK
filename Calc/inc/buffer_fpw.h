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

#include <atomic>
#include <cassert>
#include "buffer.h"
#include "calc_fp.h"

extern CalcFunctionPointers g_FunctionPointers;

namespace Calc {

    // Class that represent a function pointer implementation of a Buffer
    class BufferFP : public Buffer {
        friend class DeviceFPw;
    public:
        BufferFP(CalcDevice *const device, CalcBuffer *buffer, size_t size)
                : Buffer(), m_device(device),
                    m_buffer(buffer),
                   m_size(size) {
        }

        ~BufferFP() {
            g_FunctionPointers.DeviceDeleteBuffer(m_device, m_buffer);
        }

        std::size_t GetSize() const override {
            return m_size;
        }
        
        CalcDevice *const m_device;
        CalcBuffer *m_buffer;
        size_t m_size;
    };

}
