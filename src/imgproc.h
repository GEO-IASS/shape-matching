#ifndef IMGPROC_H
#define IMGPROC_H

#include <gtk/gtk.h>

#define CHANNEL_DEPTH   8
#define N_CHANNELS_RGB  3
#define N_CHANNELS_RGBA 4
#define N_CHANNELS_GRAY 1

#define P_WIDTH(p_image)        (gdk_pixbuf_get_width (p_image))
#define P_HEIGHT(p_image)       (gdk_pixbuf_get_height (p_image))
#define P_DEPTH(p_image)        (gdk_pixbuf_get_bits_per_sample (p_image))
#define P_N_CHANNELS(p_image)   (gdk_pixbuf_get_n_channels (p_image))
#define P_STRIDE(p_image)       (gdk_pixbuf_get_rowstride (p_image))
#define P_HAS_ALPHA(p_image)    (gdk_pixbuf_get_has_alpha (p_image))
#define P_PIXELS(p_image)       (gdk_pixbuf_get_pixels (p_image))
#define P_READ_PIXELS(p_image)  (gdk_pixbuf_read_pixels (p_image))
#define CLEAR_WHITE(image)      (gdk_pixbuf_fill (image, 0xffffffff))

GdkPixbuf*    get_contour_image_from_image            (const GdkPixbuf *image);
struct point* get_contour_points_from_image_with_size (const GdkPixbuf *image,
                                                       int             *size);
GdkPixbuf*    get_noised_image_from_image             (const GdkPixbuf *image);

#endif // IMGPROC_H
