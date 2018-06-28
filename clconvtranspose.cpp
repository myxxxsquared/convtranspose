#include <cassert>
#include <stdexcept>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cstdlib>

#include "CL/cl.h"

const char *getErrorString(cl_int error)
{
    switch (error)
    {
    // run-time and JIT compiler errors
    case 0:
        return "CL_SUCCESS";
    case -1:
        return "CL_DEVICE_NOT_FOUND";
    case -2:
        return "CL_DEVICE_NOT_AVAILABLE";
    case -3:
        return "CL_COMPILER_NOT_AVAILABLE";
    case -4:
        return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
    case -5:
        return "CL_OUT_OF_RESOURCES";
    case -6:
        return "CL_OUT_OF_HOST_MEMORY";
    case -7:
        return "CL_PROFILING_INFO_NOT_AVAILABLE";
    case -8:
        return "CL_MEM_COPY_OVERLAP";
    case -9:
        return "CL_IMAGE_FORMAT_MISMATCH";
    case -10:
        return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
    case -11:
        return "CL_BUILD_PROGRAM_FAILURE";
    case -12:
        return "CL_MAP_FAILURE";
    case -13:
        return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
    case -14:
        return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
    case -15:
        return "CL_COMPILE_PROGRAM_FAILURE";
    case -16:
        return "CL_LINKER_NOT_AVAILABLE";
    case -17:
        return "CL_LINK_PROGRAM_FAILURE";
    case -18:
        return "CL_DEVICE_PARTITION_FAILED";
    case -19:
        return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

    // compile-time errors
    case -30:
        return "CL_INVALID_VALUE";
    case -31:
        return "CL_INVALID_DEVICE_TYPE";
    case -32:
        return "CL_INVALID_PLATFORM";
    case -33:
        return "CL_INVALID_DEVICE";
    case -34:
        return "CL_INVALID_CONTEXT";
    case -35:
        return "CL_INVALID_QUEUE_PROPERTIES";
    case -36:
        return "CL_INVALID_COMMAND_QUEUE";
    case -37:
        return "CL_INVALID_HOST_PTR";
    case -38:
        return "CL_INVALID_MEM_OBJECT";
    case -39:
        return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
    case -40:
        return "CL_INVALID_IMAGE_SIZE";
    case -41:
        return "CL_INVALID_SAMPLER";
    case -42:
        return "CL_INVALID_BINARY";
    case -43:
        return "CL_INVALID_BUILD_OPTIONS";
    case -44:
        return "CL_INVALID_PROGRAM";
    case -45:
        return "CL_INVALID_PROGRAM_EXECUTABLE";
    case -46:
        return "CL_INVALID_KERNEL_NAME";
    case -47:
        return "CL_INVALID_KERNEL_DEFINITION";
    case -48:
        return "CL_INVALID_KERNEL";
    case -49:
        return "CL_INVALID_ARG_INDEX";
    case -50:
        return "CL_INVALID_ARG_VALUE";
    case -51:
        return "CL_INVALID_ARG_SIZE";
    case -52:
        return "CL_INVALID_KERNEL_ARGS";
    case -53:
        return "CL_INVALID_WORK_DIMENSION";
    case -54:
        return "CL_INVALID_WORK_GROUP_SIZE";
    case -55:
        return "CL_INVALID_WORK_ITEM_SIZE";
    case -56:
        return "CL_INVALID_GLOBAL_OFFSET";
    case -57:
        return "CL_INVALID_EVENT_WAIT_LIST";
    case -58:
        return "CL_INVALID_EVENT";
    case -59:
        return "CL_INVALID_OPERATION";
    case -60:
        return "CL_INVALID_GL_OBJECT";
    case -61:
        return "CL_INVALID_BUFFER_SIZE";
    case -62:
        return "CL_INVALID_MIP_LEVEL";
    case -63:
        return "CL_INVALID_GLOBAL_WORK_SIZE";
    case -64:
        return "CL_INVALID_PROPERTY";
    case -65:
        return "CL_INVALID_IMAGE_DESCRIPTOR";
    case -66:
        return "CL_INVALID_COMPILER_OPTIONS";
    case -67:
        return "CL_INVALID_LINKER_OPTIONS";
    case -68:
        return "CL_INVALID_DEVICE_PARTITION_COUNT";

    // extension errors
    case -1000:
        return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
    case -1001:
        return "CL_PLATFORM_NOT_FOUND_KHR";
    case -1002:
        return "CL_INVALID_D3D10_DEVICE_KHR";
    case -1003:
        return "CL_INVALID_D3D10_RESOURCE_KHR";
    case -1004:
        return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
    case -1005:
        return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
    default:
        return "Unknown OpenCL error";
    }
}

void printclerror(cl_int errorcode)
{
    fprintf(stderr, "ERROR: %d, %s\n", (int)errorcode, getErrorString(errorcode));
}

using namespace std;

cl_platform_id platformID;
cl_device_id deviceID;
cl_context context;
cl_command_queue commandqueue;

void createContext()
{
    clGetPlatformIDs(1, &platformID, nullptr);
    clGetDeviceIDs(platformID, CL_DEVICE_TYPE_ALL, 1, &deviceID, nullptr);

    cl_context_properties properties[] = {
        CL_CONTEXT_PLATFORM,
        reinterpret_cast<cl_context_properties>(platformID),
        0,
        0};

    cl_int clerror = 0;
    context = clCreateContext(
        properties,
        1,
        &deviceID,
        nullptr,
        nullptr,
        &clerror);

    if (clerror != CL_SUCCESS)
    {
        printclerror(clerror);
        throw runtime_error("fail to create context");
    }

    commandqueue = clCreateCommandQueue(
        context,
        deviceID,
        0,
        &clerror);

    if (clerror != CL_SUCCESS)
    {
        printclerror(clerror);
        throw runtime_error("fail to create command queue");
    }
}

cl_program compileProgram(const char *filename)
{
    // load program code from file
    ifstream clfile{filename};
    string clprogram;
    clprogram.assign(std::istreambuf_iterator<char>(clfile),
                     std::istreambuf_iterator<char>());
    const char *strprogram = clprogram.c_str();
    size_t lengthprogram = clprogram.size();

    // create program
    cl_int clerror = 0;
    cl_program program;
    program = clCreateProgramWithSource(
        context,
        1,
        &strprogram,
        &lengthprogram,
        &clerror);
    if (clerror != CL_SUCCESS)
    {
        printclerror(clerror);
        throw runtime_error("fail to create program");
    }

    clerror = clBuildProgram(program, 1, &deviceID, nullptr, nullptr, nullptr);
    if (clerror == CL_BUILD_PROGRAM_FAILURE)
    {
        size_t log_size;
        clGetProgramBuildInfo(program, deviceID, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        char *log = (char *)malloc(log_size);
        clGetProgramBuildInfo(program, deviceID, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        fprintf(stderr, "%s\n", log);
        free(log);
    }
    if (clerror != CL_SUCCESS)
    {
        printclerror(clerror);
        throw runtime_error("fail to build program");
    }

    return program;
}

// const int input_width = 4;
// const int input_height = 4;
// const int input_channels = 2;
// const int output_width = 8;
// const int output_height = 8;
// const int output_channels = 1;
// const int filter_width = 1;
// const int filter_height = 1;
// const int stide_x = 2;
// const int stide_y = 2;

//[input_channels][input_height][input_width]
//[output_channels][output_height][output_width]
//[output_channels][input_channels][2 * filter_height + 1][2 * filter_width + 1]

float *input_image;
float *output_image;
float *filter_image;

cl_mem createImage(int width, int height, int channels, int flag)
{
    cl_int clerror = 0;

    cl_image_format format;
    format.image_channel_data_type = CL_FLOAT;
    format.image_channel_order = CL_R;

    cl_image_desc desc;
    desc.image_type = CL_MEM_OBJECT_IMAGE2D_ARRAY;
    desc.image_width = width;
    desc.image_height = height;
    desc.image_depth = 0;
    desc.image_array_size = channels;
    desc.image_row_pitch = 0;
    desc.image_slice_pitch = 0;
    desc.num_mip_levels = 0;
    desc.num_samples = 0;
    desc.mem_object = nullptr;

    cl_mem img = clCreateImage(
        context,
        flag,
        &format,
        &desc,
        nullptr,
        &clerror);
    if (clerror != CL_SUCCESS)
    {
        printclerror(clerror);
        throw runtime_error("fail to create image");
    }

    return img;
}

cl_kernel createKernel(cl_program program, const char *name)
{
    cl_int clerror = 0;
    cl_kernel kernel = clCreateKernel(program, name, &clerror);
    if (clerror != CL_SUCCESS)
    {
        printclerror(clerror);
        throw runtime_error("fail to create kernel");
    }
    return kernel;
}

void setKernelArg(cl_kernel kernel, cl_uint index, size_t size, const void *ptr)
{
    cl_int clerror = clSetKernelArg(kernel, index, size, ptr);
    if (clerror != CL_SUCCESS)
    {
        printclerror(clerror);
        throw runtime_error("fail to set arg");
    }
}

void writeImage(cl_mem mem, int width, int height, int channels, void *image)
{
    cl_int clerror = 0;

    size_t origin[] = {0, 0, 0};
    size_t region[] = {(size_t)width, (size_t)height, (size_t)channels};

    clerror = clEnqueueWriteImage(
        commandqueue,
        mem,
        CL_TRUE,
        origin,
        region,
        0,
        0,
        image,
        0,
        nullptr,
        nullptr);
    if (clerror != CL_SUCCESS)
    {
        printclerror(clerror);
        throw runtime_error("fail to write image");
    }
}

void readImage(cl_mem mem, int width, int height, int channels, void *image)
{
    cl_int clerror = 0;

    size_t origin[] = {0, 0, 0};
    size_t region[] = {(size_t)width, (size_t)height, (size_t)channels};

    clerror = clEnqueueReadImage(
        commandqueue,
        mem,
        CL_TRUE,
        origin,
        region,
        0,
        0,
        image,
        0,
        nullptr,
        nullptr);
    if (clerror != CL_SUCCESS)
    {
        printclerror(clerror);
        throw runtime_error("fail to read image");
    }
}

void writeFile(const char* filename, int length, void* data)
{
    FILE *f = fopen(filename, "wb");
    int write = (int)fwrite(data, sizeof(float), length, f);
    if(write != length)
        throw runtime_error("fail to write file");
    fclose(f);
}

int main()
{
    int input_width = 1920;
    int input_height = 1080;
    int input_channels = 32;
    int stide_x = 2;
    int stide_y = 2;
    int output_channels = 1;
    int filter_width = 4;
    int filter_height = 4;

    int output_width = input_width * stide_x;
    int output_height = input_height * stide_y;

    int input_length = input_channels * input_height * input_width;
    int output_length = output_channels * output_height * output_width;
    int filter_length = output_channels * input_channels * (2 * filter_height + 1) * (2 * filter_width + 1);

    input_image = new float[input_length];
    output_image = new float[output_length];
    filter_image = new float[filter_length];

    srand((unsigned int)time(NULL));

    for(int i = 0; i < input_length; ++i)
        input_image[i] = (float)rand() / RAND_MAX;
    for(int i = 0; i < filter_length; ++i)
        filter_image[i] = (float)rand() / RAND_MAX;

    cl_int clerror = 0;

    createContext();
    cl_program program = compileProgram("convtranspose.cl");

    cl_mem mem_input = createImage(input_width, input_height, input_channels, CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY);
    cl_mem mem_filter = createImage(2 * filter_width + 1, 2 * filter_height + 1, output_channels * input_channels, CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY);
    cl_mem mem_output = createImage(output_width, output_height, output_channels, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY);

    cl_kernel kernel = createKernel(program, "conv_transpose");

    {
        int tmp;
        tmp = input_width;
        setKernelArg(kernel, 0, sizeof(int), &tmp);
        tmp = input_height;
        setKernelArg(kernel, 1, sizeof(int), &tmp);
        tmp = input_channels;
        setKernelArg(kernel, 2, sizeof(int), &tmp);
        tmp = output_width;
        setKernelArg(kernel, 3, sizeof(int), &tmp);
        tmp = output_height;
        setKernelArg(kernel, 4, sizeof(int), &tmp);
        tmp = output_channels;
        setKernelArg(kernel, 5, sizeof(int), &tmp);
        tmp = filter_width;
        setKernelArg(kernel, 6, sizeof(int), &tmp);
        tmp = filter_height;
        setKernelArg(kernel, 7, sizeof(int), &tmp);
        tmp = stide_x;
        setKernelArg(kernel, 8, sizeof(int), &tmp);
        tmp = stide_y;
        setKernelArg(kernel, 9, sizeof(int), &tmp);
        setKernelArg(kernel, 10, sizeof(cl_mem), &mem_input);
        setKernelArg(kernel, 11, sizeof(cl_mem), &mem_filter);
        setKernelArg(kernel, 12, sizeof(cl_mem), &mem_output);
    }

    writeImage(mem_input, input_width, input_height, input_channels, input_image);
    writeImage(mem_filter, 2 * filter_width + 1, 2 * filter_height + 1, output_channels * input_channels, filter_image);

    size_t globalsize[] = {(size_t)output_width, (size_t)output_height, 1};
    size_t localsize[] = {4, 4, 1};

    clerror = clEnqueueNDRangeKernel(
        commandqueue,
        kernel,
        3,
        nullptr,
        globalsize,
        localsize,
        0,
        nullptr,
        nullptr);
    if (clerror != CL_SUCCESS)
    {
        printclerror(clerror);
        throw runtime_error("fail to run kernel");
    }

    readImage(mem_output, output_width, output_height, output_channels, output_image);

    writeFile("input.data", input_length, input_image);
    writeFile("filter.data", filter_length, filter_image);
    writeFile("output.data", output_length, output_image);

    return 0;
}
