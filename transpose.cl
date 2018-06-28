
__kernel void transpose(
    read_only image2d_array_t input,
    write_only image2d_array_t output,
    int N,
    int size)
{
    size_t x = get_global_id(0);
    size_t y = get_global_id(1);
    size_t z = get_global_id(2);

    size_t nx = get_global_size(0);
    size_t ny = get_global_size(1);
    size_t nz = get_global_size(2);

    const sampler_t samplerA = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_REPEAT | CLK_FILTER_NEAREST;

    for(size_t i = x; i < size; i += nx)
    {
        for(size_t j = y; j < size; j += ny)
        {
            for(size_t k = z; k < N; k += nz)
            {
                float4 val = read_imagef(input, samplerA, (int4)(0, j, k, 0));
                write_imagef(output, (int4)(i, j, k, 0), val);
            }
        }
    }
}
