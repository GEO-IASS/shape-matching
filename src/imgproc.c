#include <opencv2/core/core_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#include "imgproc.h"
#include "global_definitions.h"
#include "rand_value_generator.h"

#define WHITE_COLOR_CV (cvScalarAll (255))
#define BLACK_COLOR_CV (cvScalarAll (0))

#define MIN_SCALE_RATIO 0.5
#define MAX_SCALE_RATIO 1.5

static IplImage*
pixbuf2ipl (const GdkPixbuf *image)
{
  IplImage *res_image;
  int width, height;
  int depth, n_channels;
  int stride, res_img_stride;
  const guchar *image_data;
  guchar *res_image_data;
  gboolean has_alpha;

  width = P_WIDTH (image);
  height = P_HEIGHT (image);
  depth = P_DEPTH (image);
  n_channels = P_N_CHANNELS (image);
  stride = P_STRIDE (image);
  has_alpha = P_HAS_ALPHA (image);

  g_assert(depth == CHANNEL_DEPTH);

  image_data = P_READ_PIXELS (image);
  res_image = cvCreateImage(cvSize(width, height),
                            depth, n_channels);
  res_image_data = (guchar*)res_image->imageData;
  res_img_stride = res_image->widthStep;

  for(int i = 0; i < height; ++i)
    for(int j = 0; j < width; ++j)
      {
        int index = i * res_img_stride + j * n_channels;
        res_image_data[index] = image_data[index + 2];
        res_image_data[index + 1] = image_data[index + 1];
        res_image_data[index + 2] = image_data[index];
      }

  return res_image;
}

static GdkPixbuf *
ipl2pixbuf(const IplImage *image)
{
  uchar *imageData;
  guchar *pixbufData;
  int widthStep, n_channels, res_stride;
  int width, height, depth, res_n_channels;
  int data_order;
  GdkPixbuf *res_image;
  long ipl_depth;
  CvSize roi;

  cvGetRawData (image, &imageData, &widthStep, &roi);
  width = roi.width;
  height = roi.height;
  n_channels = image->nChannels;
  data_order = image->dataOrder;

  g_assert(data_order == IPL_DATA_ORDER_PIXEL);
  g_assert(n_channels == N_CHANNELS_RGB  ||
           n_channels == N_CHANNELS_RGBA ||
           n_channels == N_CHANNELS_GRAY);

  switch(ipl_depth = image->depth)
    {
    case IPL_DEPTH_8U:
      depth = 8;
      break;
    default:
      depth = 0;
      break;
    }
  g_assert(depth == CHANNEL_DEPTH);

  res_image = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE,
                             depth, width, height);
  pixbufData = P_PIXELS (res_image);
  res_stride = P_STRIDE (res_image);
  res_n_channels = N_CHANNELS_RGB;

  for(int i = 0; i < height; ++i)
    for(int j = 0; j < width; ++j)
      {
        int index = i * widthStep + j * n_channels;
        int res_index = i * res_stride + j * res_n_channels;

        if(n_channels == N_CHANNELS_GRAY)
          {
            pixbufData[res_index] = pixbufData[res_index + 1] =
                pixbufData[res_index + 2] = imageData[index];
          }
        else
          {
            pixbufData[res_index] = imageData[index + 2];
            pixbufData[res_index + 1] = imageData[index + 1];
            pixbufData[res_index + 2] = imageData[index];
          }
      }

  return res_image;
}

struct point*
get_contour_points_from_image_with_size (const GdkPixbuf *image,
                                         int             *size)
{
  IplImage *ipl_image, *ipl_gray;
  CvMemStorage *contours;
  CvSeq *first_contour;
  CvScalar black, white;
  struct point *result;

  black = cvScalarAll (0);
  white = cvScalarAll (255);

  ipl_image = pixbuf2ipl (image);

  ipl_gray = cvCreateImage (cvGetSize (ipl_image),
                            ipl_image->depth,
                            N_CHANNELS_GRAY);

  cvCvtColor (ipl_image, ipl_gray, CV_BGR2GRAY);
  cvThreshold (ipl_gray, ipl_gray, 127, 255, CV_THRESH_BINARY|CV_THRESH_OTSU);
  cvSmooth (ipl_gray, ipl_gray, CV_GAUSSIAN, 15, 15, 0, 0);

  contours = cvCreateMemStorage (0);
  first_contour = NULL;
  cvFindContours (ipl_gray,
                  contours,
                  &first_contour,
                  sizeof (CvContour),
                  CV_RETR_LIST,
                  CV_CHAIN_APPROX_NONE,
                  cvPoint (0,0));

  result = (struct point*) malloc (sizeof (struct point) * first_contour->total);
  for (int i = 0; i < first_contour->total; ++i)
    {
      CvPoint *contour_point;

      contour_point = CV_GET_SEQ_ELEM (CvPoint, first_contour, i);

      result[i].x = contour_point->x;
      result[i].y = contour_point->y;
    }

  *size = first_contour->total;

  cvReleaseImage (&ipl_image);
  cvReleaseImage (&ipl_gray);
  cvReleaseMemStorage (&contours);

  return result;
}

static void
crop (const GdkPixbuf *image,
      int             *x,
      int             *y,
      int             *width,
      int             *height)
{
  int rowstride;
  int image_width;
  int image_height;
  guchar *pixels;
  struct point
  {
    int x,y;
  } top;
  struct point right;
  struct point left;
  struct point bottom;
  int index, n_channels;

  rowstride = gdk_pixbuf_get_rowstride(image);
  image_width = gdk_pixbuf_get_width(image);
  image_height = gdk_pixbuf_get_height(image);
  pixels = gdk_pixbuf_get_pixels(image);
  n_channels = gdk_pixbuf_get_n_channels(image);

  /* top */
  for(int i = 0; i < image_height; ++i)
    for(int j = 0; j < image_width; ++j)
      {
        index = i * rowstride + j * n_channels;
        if(pixels[index] == 0)
          {
            top.x = j;
            top.y = i;
            goto left;
          }
      }

left:
  for(int i = 0; i < image_width; ++i)
    for(int j = 0; j < image_height; ++j)
      {
        index = j * rowstride + i * n_channels;
        if(pixels[index] == 0)
          {
            left.x = i;
            left.y = j;
            goto bottom;
          }
      }

bottom:
  for(int i = image_height - 1; i >= 0; --i)
    for(int j = 0; j < image_width; ++j)
      {
        index = i * rowstride + j * n_channels;
        if(pixels[index] == 0)
          {
            bottom.x = j;
            bottom.y = i;
            goto right;
          }
      }

right:
  for(int i = image_width - 1; i >= 0; --i)
    for(int j = 0; j < image_height; ++j)
      {
        index = j * rowstride + i * n_channels;
        if(pixels[index] == 0)
          {
            right.x = i;
            right.y = j;
            goto end;
          }
      }

end:
  *x = left.x;
  *y = top.y;
  *width = right.x - left.x + 1;
  *height = bottom.y - top.y + 1;
}

GdkPixbuf*
get_noised_image_from_image (const GdkPixbuf *image)
{
  IplImage *ipl_orig, *ipl_scaled, *ipl_transformed;
  CvRect roi;
  int new_width, new_height;
  GdkPixbuf *res;
  RandValueGenerator *generator;
  double scale_ratio;
  int x_shifted, y_shifted;

  rand_value_generator_init (&generator);

  crop (image, &roi.x, &roi.y, &roi.width, &roi.height);

  ipl_orig = pixbuf2ipl (image);

  scale_ratio = rand_value_generator_get_next_double (generator,
                                                      MIN_SCALE_RATIO,
                                                      MAX_SCALE_RATIO);
  new_width = (new_width = floor ((double)roi.width * scale_ratio)) > ipl_orig->width ? ipl_orig->width - 1 :
                                                                                        new_width;
  new_height = (new_height = floor ((double)roi.height * scale_ratio)) > ipl_orig->height ? ipl_orig->height - 1 :
                                                                                            new_height;

  ipl_scaled = cvCreateImage (cvSize (new_width, new_height),
                              ipl_orig->depth,
                              ipl_orig->nChannels);

  ipl_transformed = cvCreateImage (cvGetSize (ipl_orig),
                                   ipl_orig->depth,
                                   ipl_orig->nChannels);

  cvSetImageROI (ipl_orig, roi);
  cvResize (ipl_orig, ipl_scaled, CV_INTER_CUBIC);



  x_shifted = rand_value_generator_get_next_int (generator, 0,
                                                 ipl_transformed->width - new_width);
  y_shifted = rand_value_generator_get_next_int (generator, 0,
                                                 ipl_transformed->height - new_height);
  cvSet (ipl_transformed, cvScalarAll (255), NULL);
  cvSetImageROI (ipl_transformed, cvRect (x_shifted,
                                          y_shifted,
                                          new_width,
                                          new_height));
  cvCopy (ipl_scaled, ipl_transformed, NULL);
  cvResetImageROI (ipl_transformed);

  res = ipl2pixbuf (ipl_transformed);
  
  rand_value_generator_release (&generator);

  cvReleaseImage (&ipl_orig);
  cvReleaseImage (&ipl_scaled);
  cvReleaseImage (&ipl_transformed);

  return res;
}
