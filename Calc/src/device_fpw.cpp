#include "except_fpw.h"

#include <cassert>

#include "calc_common.h"
#include "buffer.h"
#include "event.h"
#include "executable.h"


#include "buffer_fpw.h"
#include "device_fpw.h"
#include "event_fpw.h"


namespace Calc
{    

    // ctor
    DeviceFPw::DeviceFPw( CalcDevice *const device, const DeviceSpec spec) :
        Device(),
         m_device(device),
         m_spec(spec)
    {
    }

    // dtor
    DeviceFPw::~DeviceFPw()
    {
      g_FunctionPointers.DeleteDevice(m_device);
    }

    // Return specification of the device
    void DeviceFPw::GetSpec( DeviceSpec& spec )
    {
        spec = m_spec;
    }

    // Buffer creation and deletion
    Buffer* DeviceFPw::CreateBuffer( std::size_t size, std::uint32_t flags )
    {
        return CreateBuffer( size, flags, nullptr );
    }

    Buffer* DeviceFPw::CreateBuffer( std::size_t size, std::uint32_t flags, void* initdata )
    {
        if(size == 0 )
        {
            throw ExceptionFPw("Buffer size of 0 isn't valid" );
            return nullptr;
        }

        CalcBuffer* newBuffer = g_FunctionPointers.DeviceCreateBuffer(m_device, size, flags, initdata);

        return new BufferFP( m_device, newBuffer, size );
    }

    void DeviceFPw::DeleteBuffer( Buffer* buffer )
    {
        if ( nullptr != buffer )
        {
            delete buffer;
        }
    }

    // Data movement
    void DeviceFPw::ReadBuffer( Buffer const* buffer, std::uint32_t queue, std::size_t offset, std::size_t size, void* dst, Event** e ) const
    {
        CalcEvent *cE = nullptr;
        CalcEvent **cEPtr = (e == nullptr) ? nullptr : &cE;
        
        g_FunctionPointers.DeviceReadBuffer(m_device, ((BufferFP const*)buffer)->m_buffer, queue, offset, size, dst, cEPtr);
        
        if (nullptr != e) {
            *e = new EventFP(m_device, cE);
        }
    }

    void DeviceFPw::WriteBuffer( Buffer const* buffer, std::uint32_t queue, std::size_t offset, std::size_t size, void* src, Event** e )
    {
        CalcEvent *cE = nullptr;
        CalcEvent **cEPtr = (e == nullptr) ? nullptr : &cE;

        g_FunctionPointers.DeviceWriteBuffer(m_device, ((BufferFP const*)buffer)->m_buffer, queue, offset, size, src, cEPtr);

        if (nullptr != e) {
            *e = new EventFP(m_device, cE);
        }
    }

    // Buffer mapping 
    void DeviceFPw::MapBuffer(   Buffer const* buffer,
                                    std::uint32_t queue,
                                    std::size_t offset,
                                    std::size_t size,
                                    std::uint32_t map_type,
                                    void** mapdata,
                                    Event** e )
    {
        CalcEvent *cE = nullptr;
        CalcEvent **cEPtr = (e == nullptr) ? nullptr : &cE;
        
        g_FunctionPointers.DeviceMapBuffer(m_device, ((BufferFP const*)buffer)->m_buffer, queue, offset, size, map_type, mapdata, cEPtr);
        
        if (nullptr != e) {
            *e = new EventFP(m_device, cE);
        }
    }

    void DeviceFPw::UnmapBuffer( Buffer const* buffer, std::uint32_t queue, void* mapdata, Event** e )
    {
        CalcEvent *cE = nullptr;
        CalcEvent **cEPtr = (e == nullptr) ? nullptr : &cE;
        
        g_FunctionPointers.DeviceUnmapBuffer(m_device, ((BufferFP const*)buffer)->m_buffer, queue, mapdata, cEPtr);
        
        if (nullptr != e) {
            *e = new EventFP(m_device, cE);
        }
    }
    
    // Executable implementation
    class FunctionFPw : public Function
    {
    public:
        FunctionFPw(CalcFunction *function) : m_function(function) {}
        ~FunctionFPw() { }
        
        CalcFunction *m_function;
        
        // Argument setters
        void SetArg(std::uint32_t idx, std::size_t arg_size, void* arg) override {
            g_FunctionPointers.FunctionSetArg(m_function, idx, arg_size, arg);
        }
        
        void SetArg(std::uint32_t idx, Buffer const* arg) override {
            g_FunctionPointers.FunctionSetBuffer(m_function, idx, ((BufferFP*)arg)->m_buffer);
        }
        
        void SetArg(std::uint32_t idx, std::size_t size, SharedMemory shmem) override {
            g_FunctionPointers.FunctionSetSharedMemory(m_function, idx, size);
        }
    };
    
    
    // Executable implementation
    class ExecutableFPw : public Executable
    {
    public:
        ExecutableFPw(CalcExecutable *executable) : m_executable(executable) {}
        ~ExecutableFPw() {}
        
        // Function management
        Function* CreateFunction(char const* name) override {
            CalcFunction *function = g_FunctionPointers.ExecutableCreateFunction(m_executable, name);
            return new FunctionFPw(function);
        }
        void DeleteFunction(Function* func) override {
            g_FunctionPointers.ExecutableDeleteFunction(m_executable, ((FunctionFPw*)func)->m_function);
            delete func;
        }
        
        CalcExecutable *m_executable;
    };

    // Kernel compilation
    Executable* DeviceFPw::CompileExecutable( char const* source_code, std::size_t size, char const* options )
    {
        CalcExecutable *executable = g_FunctionPointers.DeviceCompileExecutableSource(m_device, source_code, size, options);
        return new ExecutableFPw( executable );
    }

    Executable* DeviceFPw::CompileExecutable( std::uint8_t const* binary_code, std::size_t size, char const* options )
    {
        CalcExecutable *executable = g_FunctionPointers.DeviceCompileExecutableBinary(m_device, binary_code, size, options);
        return new ExecutableFPw( executable );
    }

    Executable* DeviceFPw::CompileExecutable( char const* inFilename, char const** inHeaderNames, int inHeadersNum, char const* options)
    {
        CalcExecutable *executable = g_FunctionPointers.DeviceCompileExecutableFile(m_device, inFilename, inHeaderNames, inHeadersNum, options);
        return new ExecutableFPw( executable );
    }

    void DeviceFPw::DeleteExecutable( Executable* executable )
    {
        delete executable;
    }

    // Executable management
    size_t DeviceFPw::GetExecutableBinarySize( Executable const* executable ) const
    {
        return g_FunctionPointers.DeviceGetExecutableBinarySize(m_device, ((ExecutableFPw const*)executable)->m_executable);
    }

    void DeviceFPw::GetExecutableBinary( Executable const* executable, std::uint8_t* binary ) const
    {
        g_FunctionPointers.DeviceGetExecutableBinary(m_device, ((ExecutableFPw const*)executable)->m_executable, binary);
    }

    // Execution Not thread safe
    void DeviceFPw::Execute( Function const* func, std::uint32_t queue, size_t global_size, size_t local_size, Event** e )
    {
        CalcEvent *cE = nullptr;
        CalcEvent **cEPtr = (e == nullptr) ? nullptr : &cE;
        
        g_FunctionPointers.DeviceExecute(m_device, ((FunctionFPw const*)func)->m_function, queue, global_size, local_size, cEPtr);
        
        if (nullptr != e) {
            *e = new EventFP(m_device, cE);
        }
    }

    // Events handling
    void DeviceFPw::WaitForEvent( Event* e )
    {
        g_FunctionPointers.DeviceWaitForEvent(m_device, ((EventFP*)e)->m_event);
    }

    void DeviceFPw::WaitForMultipleEvents( Event** e, std::size_t num_events )
    {
        if(nullptr == e)
            return;

        // TODO optimize
        for (size_t i = 0; i < num_events; ++i)
        {
            (*e)[i].Wait();
        }
    }

    void DeviceFPw::DeleteEvent( Event* e )
    {
        if ( nullptr != e )
        {
            delete e;
        }
    }

    // Queue management functions
    void DeviceFPw::Flush( std::uint32_t queue )
    {
        g_FunctionPointers.DeviceFlush(m_device, queue);
    }

    void DeviceFPw::Finish( std::uint32_t queue )
    {
        g_FunctionPointers.DeviceFinish(m_device, queue);
    }
}
