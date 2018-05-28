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

#include "device_fpw.h"
#include "calc_fpw.h"

namespace Calc {

    class EventFP : public Event
    {
        friend class DeviceFPw;
    public:
        EventFP( const CalcDevice* in_device, CalcEvent *in_event) :
                m_device( in_device ),
                m_event( in_event )
                 {
        }

        virtual ~EventFP()
        {
            m_device = nullptr;
        }

        void Wait() override
        {
            g_FunctionPointers.DeviceWaitForEvent(m_device, m_event);
        }

        bool IsComplete() const override
        {
            return  g_FunctionPointers.DeviceEventIsComplete(m_device, m_event);
        }
    private:
        const CalcDevice *m_device;
        CalcEvent *m_event;
    };
}
