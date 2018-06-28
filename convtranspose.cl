
#define GROUP_SIZE_X = 2;
#define GROUP_SIZE_Y = 2;
#define GROUP_SIZE_Z = 2;

int beginsub(int x, int stide, int f)
{
    if(x < f)
        return 0;
    return (x - f + stide - 1) / stide;
}

__kernel void conv_transpose(
    int input_width,
    int input_height,
    int input_channels,

    int output_width,
    int output_height,
    int output_channels,

    int filter_width,
    int filter_height,

    int stide_x,
    int stide_y,

    read_only image2d_array_t inputimg,
    read_only image2d_array_t filterimg,
    write_only image2d_array_t outputimg
)
{
    int output_x = get_global_id(0);
    int output_y = get_global_id(1);
    int output_z = get_global_id(2);

    if(output_x < output_width && output_y < output_height && output_z < output_channels)
    {
        const sampler_t sampler_conv = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

        int input_x_0 = beginsub(output_x, stide_x, filter_width);
        int filter_x_0 = output_x - stide_x * input_x_0 + filter_width;
        int input_y_0 = beginsub(output_y, stide_y, filter_width);
        int filter_y_0 = output_y - stide_y * input_y_0 + filter_height;

        float val = 0.0f;

        for(int input_z = 0; input_z < input_channels; ++input_z)
        {
            int filter_index = input_z + output_z * input_channels;

            for(int input_x = input_x_0, filter_x = filter_x_0; input_x < input_width && filter_x >= 0; ++input_x, filter_x -= stide_x)
            {
                for(int input_y = input_y_0, filter_y = filter_y_0; input_y < input_height && filter_y >= 0; ++input_y, filter_y -= stide_y)
                {
                    float imgval = read_imagef(inputimg, sampler_conv, (int4)(input_x, input_y, input_z, 0)).x;
                    float filterval = read_imagef(filterimg, sampler_conv, (int4)(filter_x, filter_y, filter_index, 0)).x;
                    val += imgval * filterval;
                }
            }
        }
        write_imagef(outputimg, (int4)(output_x, output_y, output_z, 0), (float4)(val, 0, 0, 0));
        // write_imagef(outputimg, (int4)(output_x, output_y, output_z, 0), (float4)(1, 0, 0, 0));
    }
}
