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
        BufferVulkan* vulkanBuffer = ConstCast<BufferVulkan>( buffer );

        // make sure GPU has stopped using this buffer
        WaitForFence(vulkanBuffer->m_fence_id);

        if( nullptr != e ) {
            *e = new EventFP(this);
        }

        // allocated a proxy buffer
        uint8_t* mappedMemory = new uint8_t[ size ];
        (*mapdata) = mappedMemory;

        if ( MapType::kMapRead == map_type )
        {
            // read the Vulkan buffer
            ReadBuffer( buffer, queue, offset, size, mappedMemory, e );
        }

        else if ( MapType::kMapWrite != map_type )
        {
            VK_EMPTY_IMPLEMENTATION;
        }

        vulkanBuffer->SetMappedMemory( mappedMemory, map_type, offset, size );

    }

    void DeviceFPw::UnmapBuffer( Buffer const* buffer, std::uint32_t queue, void* mapdata, Event** e )
    {
        // get the allocated proxy buffer
        BufferVulkan* vulkanBuffer = ConstCast<BufferVulkan>( buffer );

        if( nullptr != e ) {
            *e = new EventFP(this);
        }

        const MappedMemory& mappedMemory = vulkanBuffer->GetMappedMemory();

        Assert( mappedMemory.data == mapdata );

        if ( MapType::kMapWrite == mappedMemory.type )
        {
            // write the proxy buffer data to the Vulkan buffer
            WriteBuffer( buffer, queue, mappedMemory.offset, mappedMemory.size, mapdata, e );
        }

        delete[] mappedMemory.data;

        vulkanBuffer->SetMappedMemory( nullptr, 0, 0, 0 );
    }

    // Kernel compilation
    Executable* DeviceFPw::CompileExecutable( char const* source_code, std::size_t size, char const* options )
    {
        return new ExecutableVulkan( m_anvil_device, source_code, size, m_use_compute_pipe );
    }

    Executable* DeviceFPw::CompileExecutable( std::uint8_t const* binary_code, std::size_t size, char const* options )
    {
        VK_EMPTY_IMPLEMENTATION;
        return nullptr;
    }

    Executable* DeviceFPw::CompileExecutable( char const* inFilename, char const** inHeaderNames, int inHeadersNum, char const* options)
    {
        return new ExecutableVulkan( m_anvil_device, inFilename, m_use_compute_pipe );
    }

    void DeviceFPw::DeleteExecutable( Executable* executable )
    {
        delete executable;
    }

    // Executable management
    size_t DeviceFPw::GetExecutableBinarySize( Executable const* executable ) const
    {
        VK_EMPTY_IMPLEMENTATION;
        return 0;
    }

    void DeviceFPw::GetExecutableBinary( Executable const* executable, std::uint8_t* binary ) const
    {
        VK_EMPTY_IMPLEMENTATION;
    }

    // Get queue, the execution of vulkan shaders can be done through the compute queue or the graphic queue
    Anvil::Queue* DeviceFPw::GetQueue() const
    {
        Anvil::Queue* toReturn = (true == m_use_compute_pipe) ?
                                 m_anvil_device->get_compute_queue( 0 ) :
                                 m_anvil_device->get_universal_queue( 0 );
        return toReturn;
    }

    // To start recording Vulkan commands to the CommandBuffer
    void DeviceFPw::StartRecording()
    {
        Assert( false == m_is_command_buffer_recording );

        AllocNextFenceId();
        const auto fence = GetFence( m_cpu_fence_id );
        fence->reset();

        m_is_command_buffer_recording = true;
        m_command_buffer->reset( false );
        m_command_buffer->start_recording( true, false );
    }

    // Finish recording
    void DeviceFPw::EndRecording( bool in_wait_till_completed, Event** out_event )
    {

        // execute CommandBuffer if either wait_till_completed
        // TODO buffer n exec's per commit
        CommitCommandBuffer( in_wait_till_completed );

        if ( nullptr != out_event )
        {
            *out_event = new EventFP( this );
        }
    }

    // Execute CommandBuffer
    void DeviceFPw::CommitCommandBuffer( bool in_wait_till_completed )
    {
        const auto fence = GetFence( m_cpu_fence_id );

        m_is_command_buffer_recording = false;
        m_command_buffer->stop_recording();

        GetQueue()->submit_command_buffer( m_command_buffer.get(), in_wait_till_completed, fence );
    }

    // Execution Not thread safe
    void DeviceFPw::Execute( Function const* func, std::uint32_t queue, size_t global_size, size_t local_size, Event** e )
    {
        FunctionVulkan* vulkan_function = ConstCast<FunctionVulkan>( func );

        uint32_t number_of_parameters = (uint32_t)( vulkan_function->GetParameters().size() );

        // get the Function's descriptor set group
        Anvil::DescriptorSetGroup* new_descriptor_set = vulkan_function->GetDescriptorSetGroup();

        // if it's empty, this is 1st run of the Function so we have to create it
        if ( nullptr == new_descriptor_set )
        {
            // allocate it through Anvil
            new_descriptor_set = new Anvil::DescriptorSetGroup( m_anvil_device, false, 1 );

            // add bindings and items (Buffers) to the new DSG
            for ( uint32_t i = 0; i < number_of_parameters; ++i )
            {
                const Buffer* parameter = vulkan_function->GetParameters()[ i ];
                BufferVulkan* buffer = ConstCast<BufferVulkan>( parameter );
                new_descriptor_set->add_binding( 0, i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT );
            }

            // set it to the Function to be reused during any subsequent run
            vulkan_function->SetDescriptorSetGroup( new_descriptor_set );
        }

        // bind new items (Buffers), releasing the old ones
        for ( uint32_t i = 0; i < number_of_parameters; ++i )
        {
            const Buffer* parameter = vulkan_function->GetParameters()[ i ];
            BufferVulkan* buffer = ConstCast<BufferVulkan>( parameter );
            new_descriptor_set->set_binding_item( 0, i, buffer->GetAnvilBuffer() );

        }

        // get the Function's pipeline
        Anvil::ComputePipelineID pipeline_id = vulkan_function->GetPipelineID();

        // if it is invalid, this is 1st run of the Function so we have to create it
        if ( ~0u == pipeline_id )
        {
            // create the pipeline through Anvil with the shader module as a parameter
            m_anvil_device->get_compute_pipeline_manager()->add_regular_pipeline( false, false, vulkan_function->GetFunctionEntryPoint(), &pipeline_id );

            // attach the DSG to it
            m_anvil_device->get_compute_pipeline_manager()->attach_dsg_to_pipeline( pipeline_id, new_descriptor_set );

            // remember the pipeline for any seubsequent run
            vulkan_function->SetPipelineID( pipeline_id );
        }

        // indicate we'll be recording Vulkan commands to the CommandBuffer from now on
        StartRecording();

        // attach pipeline
        m_command_buffer->record_bind_pipeline( VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_id );

        Anvil::PipelineLayout* pipeline_layout = m_anvil_device->get_compute_pipeline_manager()->get_compute_pipeline_layout( pipeline_id );
        Anvil::DescriptorSet* descriptor_set = new_descriptor_set->get_descriptor_set( 0 );

        // attach layout and 0 descriptor set (we don't use any other set currently)
        m_command_buffer->record_bind_descriptor_sets( VK_PIPELINE_BIND_POINT_COMPUTE,
                                                    pipeline_layout,
                                                    0,
                                                    1,
                                                    &descriptor_set,
                                                    0,
                                                    nullptr );

        // set memory barriers 
        for ( uint32_t i = 0; i < number_of_parameters; ++i )
        {
            const Buffer* parameter = vulkan_function->GetParameters()[ i ];
            BufferVulkan* buffer = ConstCast<BufferVulkan>( parameter );

            Anvil::BufferBarrier bufferBarrier( VK_ACCESS_HOST_WRITE_BIT,
                                                VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
                                                GetQueue()->get_queue_family_index(),
                                                GetQueue()->get_queue_family_index(),
                                                buffer->GetAnvilBuffer(),
                                                0,
                                                buffer->GetSize() );

            m_command_buffer->record_pipeline_barrier( VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                                    VK_FALSE,
                                                    0, nullptr,
                                                    1, &bufferBarrier,
                                                    0, nullptr );
            // tell buffer that we are used by this submit
            buffer->SetFenceId( GetFenceId() );

        }

        // dispatch the Function's shader module
        m_command_buffer->record_dispatch( (uint32_t)global_size, 1, 1 );

        // end recording
        EndRecording( false, e );

        // remove references to buffers. they were already referenced by the CommandBuffer.
        vulkan_function->UnreferenceParametersBuffers();
    }

    // Events handling
    void DeviceFPw::WaitForEvent( Event* e )
    {
        e->Wait();

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
        if(false == m_is_command_buffer_recording)
        {
            StartRecording();
        }
        EndRecording(true, nullptr);
    }

    void DeviceFPw::Finish( std::uint32_t queue )
    {
        // TODO work out semantics of these two
        Flush(queue);
    }

    bool DeviceFPw::HasBuiltinPrimitives() const
    {
        VK_EMPTY_IMPLEMENTATION;
        return false;
    }

    Primitives* DeviceFPw::CreatePrimitives() const
    {
        VK_EMPTY_IMPLEMENTATION;
        return nullptr;
    }

    void DeviceFPw::DeletePrimitives( Primitives* prims )
    {
        VK_EMPTY_IMPLEMENTATION;
    }

    uint64_t DeviceFPw::AllocNextFenceId() {
        // stall if we have run out of fences to use
        while( m_cpu_fence_id >= m_gpu_known_fence_id + NUM_FENCE_TRACKERS)
        {
            WaitForFence(m_gpu_known_fence_id+1);
        }

        return m_cpu_fence_id.fetch_add(1);
    }

    void DeviceFPw::WaitForFence( uint64_t id ) const
    {
        AssertEx( id < m_gpu_known_fence_id + NUM_FENCE_TRACKERS,
                "CPU too far ahead of GPU" );

        while(HasFenceBeenPassed(id) == false ) {
            vkWaitForFences(m_anvil_device->get_device_vk(), 1,
                            GetFence(m_gpu_known_fence_id)->get_fence_ptr(),
                            VK_TRUE,
                            UINT64_MAX);

            // don't known id update until wait has finished
            m_gpu_known_fence_id++;
        }
    }

}

#endif // USE_VULKAN
