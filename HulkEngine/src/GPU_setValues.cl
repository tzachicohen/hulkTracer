


__kernel void SetInitialValues(__write_only image2d_t                    frame_buffer,
                                __global float4 *            pHitData)
{
     uint column =get_global_id(0);
     uint row =get_global_id(1);
     uint imageWidth = get_image_width(frame_buffer);
     int index  = column + (row*imageWidth);
     pHitData[index].w= FLT_MAX;
}