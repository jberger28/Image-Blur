# Image-Blur
Provide 2 different multi-threaded approaches for applying a blurring filter to JPEG images, 
part of a larger project that provided a framework for reading in and writing JPEGs.

"image_blur()" function uses pthreads to either blur the image using row blocking or row interleaving, depending on whether "BLOCK" OR "INTERLEAVE" are defined.
