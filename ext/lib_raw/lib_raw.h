#ifndef LIB_RAW_H
#define LIB_RAW_H 1

#include <time.h>
#include "ruby.h"
#include "libraw/libraw.h"


typedef struct {
	LibRaw *libraw;
} LibRawNativeResource;

typedef struct {
	libraw_output_params_t params;
} OutputParamNativeResource;


extern VALUE rb_mLibRaw;

extern VALUE rb_mColormatrixType;
extern VALUE rb_mCameraMount;
extern VALUE rb_mCameraFormat;
extern VALUE rb_mSonyArw2Option;
extern VALUE rb_mDp2qOption;
extern VALUE rb_mDecoderFlag;
extern VALUE rb_mWarning;
extern VALUE rb_mProgress;
extern VALUE rb_mThumbnailFormat;

extern VALUE rb_cRawObject;

extern VALUE rb_cIParam;
extern VALUE rb_cImageSize;
extern VALUE rb_cOutputParam;
extern VALUE rb_cMakerNote;
extern VALUE rb_cLensInfo;

extern VALUE rb_eRawError;
extern VALUE rb_eUnspecifiedError;
extern VALUE rb_eFileUnsupported;
extern VALUE rb_eRequestForNonexistentImage;
extern VALUE rb_eOutOfOrderCall;
extern VALUE rb_eNoThumbnail;
extern VALUE rb_eUnsupportedThumbnail;
extern VALUE rb_eInputClosed;
extern VALUE rb_eUnsufficientMemory;
extern VALUE rb_eDataError;
extern VALUE rb_eIOError;
extern VALUE rb_eCancelledByCallback;
extern VALUE rb_eBadCrop;


// LibRaw Native Resource
extern void lib_raw_native_resource_delete(LibRawNativeResource * p);
extern void output_param_native_resource_delete(OutputParamNativeResource * p);
extern LibRaw* get_lib_raw(VALUE self);
extern void copy_lib_raw(VALUE dst, VALUE src);
extern void check_errors(int e);

// LibRaw::RawObject
extern void apply_rawobject(VALUE self);
extern void apply_data(VALUE self, libraw_data_t *p);
extern VALUE rb_raw_object_initialize(VALUE self);
extern VALUE rb_raw_object_open_file(VALUE self, VALUE filename);
extern VALUE rb_raw_object_open_buffer(VALUE self, VALUE buff);
extern VALUE rb_raw_object_unpack(VALUE self);
extern VALUE rb_raw_object_unpack_thumb(VALUE self);
extern VALUE rb_raw_object_recycle_datastream(VALUE self);
extern VALUE rb_raw_object_recycle(VALUE self);
extern VALUE rb_raw_object_dcraw_ppm_tiff_writer(VALUE self, VALUE filename);
extern VALUE rb_raw_object_dcraw_thumb_writer(VALUE self, VALUE filename);
extern VALUE rb_raw_object_dcraw_process(VALUE self, VALUE param);

// LibRaw::IParam
extern void apply_iparam(VALUE self, libraw_iparams_t *p);

// LibRaw::ImageSize
extern void apply_image_size(VALUE self, libraw_image_sizes_t *p);

// LibRaw::ColorData
extern void apply_colordata(VALUE self, libraw_colordata_t *p);

// LibRaw::ImgOther
extern void apply_imgother(VALUE self, libraw_imgother_t *p);

// LibRaw::OutputParam
extern void apply_output_param(VALUE self, libraw_output_params_t *p);
extern VALUE rb_output_param_greybox(VALUE self, VALUE x, VALUE y, VALUE w, VALUE h);
extern VALUE rb_output_param_cropbox(VALUE self, VALUE x, VALUE y, VALUE w, VALUE h);
extern VALUE rb_output_param_gamma(VALUE self, VALUE pwr, VALUE ts);
extern VALUE rb_output_param_whitebalance(VALUE self, VALUE r, VALUE g, VALUE b, VALUE g2);
extern VALUE rb_output_param_set_bright(VALUE self, VALUE val);
extern VALUE rb_output_param_set_highlight(VALUE self, VALUE val);
extern VALUE rb_output_param_set_use_auto_wb(VALUE self, VALUE val);
extern VALUE rb_output_param_set_use_camera_wb(VALUE self, VALUE val);
extern VALUE rb_output_param_set_fbdd_noiserd(VALUE self, VALUE val);

// LibRaw::MakerNote
extern void apply_makernote(VALUE self, libraw_makernotes_lens_t *p);

// LibRaw::LensInfo
extern void apply_lensinfo(VALUE self, libraw_lensinfo_t *p);


#endif /* LIB_RAW_H */
