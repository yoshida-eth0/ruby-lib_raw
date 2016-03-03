#include "lib_raw.h"
#include "libraw/libraw.h"


VALUE rb_mLibRaw;

VALUE rb_mColormatrixType;
VALUE rb_mCameraMount;
VALUE rb_mCameraFormat;
VALUE rb_mSonyArw2Option;
VALUE rb_mDp2qOption;
VALUE rb_mDecoderFlag;
VALUE rb_mWarning;
VALUE rb_mProgress;
VALUE rb_mThumbnailFormat;

VALUE rb_cRawObject;
VALUE rb_cImageSize;

VALUE rb_eRawError;
VALUE rb_eUnspecifiedError;
VALUE rb_eFileUnsupported;
VALUE rb_eRequestForNonexistentImage;
VALUE rb_eOutOfOrderCall;
VALUE rb_eNoThumbnail;
VALUE rb_eUnsupportedThumbnail;
VALUE rb_eInputClosed;
VALUE rb_eUnsufficientMemory;
VALUE rb_eDataError;
VALUE rb_eIOError;
VALUE rb_eCancelledByCallback;
VALUE rb_eBadCrop;



// LibRaw Native Resource

void lib_raw_native_resource_delete(LibRawNativeResource * p)
{
	if (p->libraw) {
		delete p->libraw;
	}
	free(p);
}

LibRaw* get_lib_raw(VALUE self)
{
	VALUE resource = rb_iv_get(self, "lib_raw_native_resource");
	if (resource==Qnil) {
		return NULL;
	}

	LibRawNativeResource *p = NULL;
	Data_Get_Struct(resource, LibRawNativeResource, p);
	if (p) {
		return p->libraw;
	}

	return NULL;
}

void copy_lib_raw(VALUE dst, VALUE src)
{
	VALUE resource = rb_iv_get(src, "lib_raw_native_resource");
	rb_iv_set(dst, "lib_raw_native_resource", resource);
}

void check_errors(enum LibRaw_errors e)
{
	if (e!=LIBRAW_SUCCESS) {
		// TODO: raise
	}
}


// LibRaw::RawObject

VALUE rb_raw_object_initialize(VALUE self)
{
	LibRawNativeResource *p = ALLOC(LibRawNativeResource);
	p->libraw = NULL;

	try {
		p->libraw = new LibRaw(LIBRAW_OPTIONS_NONE);
	} catch (std::bad_alloc) {
		rb_raise(rb_eStandardError, "alloc error");
	}

	VALUE resource = Data_Wrap_Struct(CLASS_OF(self), 0, lib_raw_native_resource_delete, p);
	rb_iv_set(self, "lib_raw_native_resource", resource);

	return self;
}

VALUE rb_raw_object_open_file(VALUE self, VALUE filename)
{
	LibRaw *libraw = get_lib_raw(self);

	char *name = RSTRING_PTR(rb_obj_as_string(filename));
	int ret = libraw->open_file(name);

	return INT2NUM(ret);
}

VALUE rb_raw_object_open_buffer(VALUE self, VALUE buff)
{
	LibRaw *libraw = get_lib_raw(self);

	int ret = libraw->open_buffer(RSTRING_PTR(buff), RSTRING_LEN(buff));

	return INT2NUM(ret);
}

VALUE rb_raw_object_unpack(VALUE self)
{
	LibRaw *libraw = get_lib_raw(self);

	int ret = libraw->unpack();

	return INT2NUM(ret);
}

VALUE rb_raw_object_unpack_thumb(VALUE self)
{
	LibRaw *libraw = get_lib_raw(self);

	int ret = libraw->unpack_thumb();

	return INT2NUM(ret);
}


// LibRaw::ImgData

VALUE rb_image_size_initialize(VALUE self, VALUE raw)
{
	copy_lib_raw(self, raw);
	return self;
}

VALUE rb_image_size_raw_height(VALUE self)
{
	LibRaw *libraw = get_lib_raw(self);
	if (libraw==NULL) {
		return Qnil;
	}

	return INT2FIX(libraw->imgdata.sizes.raw_height);
}

VALUE rb_image_size_raw_width(VALUE self)
{
	LibRaw *libraw = get_lib_raw(self);
	if (libraw==NULL) {
		return Qnil;
	}

	return INT2FIX(libraw->imgdata.sizes.raw_width);
}

VALUE rb_image_size_height(VALUE self)
{
	LibRaw *libraw = get_lib_raw(self);
	if (libraw==NULL) {
		return Qnil;
	}

	return INT2FIX(libraw->imgdata.sizes.height);
}

VALUE rb_image_size_width(VALUE self)
{
	LibRaw *libraw = get_lib_raw(self);
	if (libraw==NULL) {
		return Qnil;
	}

	return INT2FIX(libraw->imgdata.sizes.width);
}

VALUE rb_image_size_top_margin(VALUE self)
{
	LibRaw *libraw = get_lib_raw(self);
	if (libraw==NULL) {
		return Qnil;
	}

	return INT2FIX(libraw->imgdata.sizes.top_margin);
}

VALUE rb_image_size_left_margin(VALUE self)
{
	LibRaw *libraw = get_lib_raw(self);
	if (libraw==NULL) {
		return Qnil;
	}

	return INT2FIX(libraw->imgdata.sizes.left_margin);
}

VALUE rb_image_size_iheight(VALUE self)
{
	LibRaw *libraw = get_lib_raw(self);
	if (libraw==NULL) {
		return Qnil;
	}

	return INT2FIX(libraw->imgdata.sizes.iheight);
}

VALUE rb_image_size_iwidth(VALUE self)
{
	LibRaw *libraw = get_lib_raw(self);
	if (libraw==NULL) {
		return Qnil;
	}

	return INT2FIX(libraw->imgdata.sizes.iwidth);
}

VALUE rb_image_size_raw_pitch(VALUE self)
{
	LibRaw *libraw = get_lib_raw(self);
	if (libraw==NULL) {
		return Qnil;
	}

	return INT2FIX(libraw->imgdata.sizes.raw_pitch);
}

VALUE rb_image_size_pixel_aspect(VALUE self)
{
	LibRaw *libraw = get_lib_raw(self);
	if (libraw==NULL) {
		return Qnil;
	}

	return rb_float_new(libraw->imgdata.sizes.pixel_aspect);
}

VALUE rb_image_size_flip(VALUE self)
{
	LibRaw *libraw = get_lib_raw(self);
	if (libraw==NULL) {
		return Qnil;
	}

	return INT2FIX(libraw->imgdata.sizes.flip);
}



extern "C" {
void Init_lib_raw(void)
{
	rb_mLibRaw = rb_define_module("LibRaw");


	// const

	// LibRaw::ColormatrixType
	rb_mColormatrixType = rb_define_module_under(rb_mLibRaw, "ColormatrixType");

	rb_define_const(rb_mColormatrixType, "NONE", INT2FIX(LIBRAW_CMATRIX_NONE));
	rb_define_const(rb_mColormatrixType, "DNG", INT2FIX(LIBRAW_CMATRIX_DNG));
	rb_define_const(rb_mColormatrixType, "DIGBACK", INT2FIX(LIBRAW_CMATRIX_DIGBACK));
	rb_define_const(rb_mColormatrixType, "OTHER", INT2FIX(LIBRAW_CMATRIX_OTHER));


	// LibRaw::CameraMount

	rb_mCameraMount = rb_define_module_under(rb_mLibRaw, "CameraMount");

	rb_define_const(rb_mCameraMount, "Unknown", INT2FIX(LIBRAW_MOUNT_Unknown));
	rb_define_const(rb_mCameraMount, "Minolta_A", INT2FIX(LIBRAW_MOUNT_Minolta_A));
	rb_define_const(rb_mCameraMount, "Sony_E", INT2FIX(LIBRAW_MOUNT_Sony_E));
	rb_define_const(rb_mCameraMount, "Canon_EF", INT2FIX(LIBRAW_MOUNT_Canon_EF));
	rb_define_const(rb_mCameraMount, "Canon_EF_S", INT2FIX(LIBRAW_MOUNT_Canon_EF_S));
	rb_define_const(rb_mCameraMount, "Canon_EF_M", INT2FIX(LIBRAW_MOUNT_Canon_EF_M));
	rb_define_const(rb_mCameraMount, "Nikon_F", INT2FIX(LIBRAW_MOUNT_Nikon_F));
	rb_define_const(rb_mCameraMount, "Nikon_CX", INT2FIX(LIBRAW_MOUNT_Nikon_CX));
	rb_define_const(rb_mCameraMount, "FT", INT2FIX(LIBRAW_MOUNT_FT));
	rb_define_const(rb_mCameraMount, "mFT", INT2FIX(LIBRAW_MOUNT_mFT));
	rb_define_const(rb_mCameraMount, "Pentax_K", INT2FIX(LIBRAW_MOUNT_Pentax_K));
	rb_define_const(rb_mCameraMount, "Pentax_Q", INT2FIX(LIBRAW_MOUNT_Pentax_Q));
	rb_define_const(rb_mCameraMount, "Pentax_645", INT2FIX(LIBRAW_MOUNT_Pentax_645));
	rb_define_const(rb_mCameraMount, "Fuji_X", INT2FIX(LIBRAW_MOUNT_Fuji_X));
	rb_define_const(rb_mCameraMount, "Leica_M", INT2FIX(LIBRAW_MOUNT_Leica_M));
	rb_define_const(rb_mCameraMount, "Leica_R", INT2FIX(LIBRAW_MOUNT_Leica_R));
	rb_define_const(rb_mCameraMount, "Leica_S", INT2FIX(LIBRAW_MOUNT_Leica_S));
	rb_define_const(rb_mCameraMount, "Samsung_NX", INT2FIX(LIBRAW_MOUNT_Samsung_NX));
	rb_define_const(rb_mCameraMount, "RicohModule", INT2FIX(LIBRAW_MOUNT_RicohModule));
	rb_define_const(rb_mCameraMount, "Samsung_NX_M", INT2FIX(LIBRAW_MOUNT_Samsung_NX_M));
	rb_define_const(rb_mCameraMount, "Leica_T", INT2FIX(LIBRAW_MOUNT_Leica_T));
	rb_define_const(rb_mCameraMount, "Contax_N", INT2FIX(LIBRAW_MOUNT_Contax_N));
	rb_define_const(rb_mCameraMount, "Sigma_X3F", INT2FIX(LIBRAW_MOUNT_Sigma_X3F));
	rb_define_const(rb_mCameraMount, "FixedLens", INT2FIX(LIBRAW_MOUNT_FixedLens));


	// LibRaw::CameraFormat

	rb_mCameraFormat = rb_define_module_under(rb_mLibRaw, "CameraFormat");

	rb_define_const(rb_mCameraFormat, "APSC", INT2FIX(LIBRAW_FORMAT_APSC));
	rb_define_const(rb_mCameraFormat, "FF", INT2FIX(LIBRAW_FORMAT_FF));
	rb_define_const(rb_mCameraFormat, "MF", INT2FIX(LIBRAW_FORMAT_MF));
	rb_define_const(rb_mCameraFormat, "APSH", INT2FIX(LIBRAW_FORMAT_APSH));
	rb_define_const(rb_mCameraFormat, "FT", INT2FIX(LIBRAW_FORMAT_FT));


	// LibRaw::SonyArw2Option

	rb_mSonyArw2Option = rb_define_module_under(rb_mLibRaw, "SonyArw2Option");

	rb_define_const(rb_mSonyArw2Option, "NONE", INT2FIX(LIBRAW_SONYARW2_NONE));
	rb_define_const(rb_mSonyArw2Option, "BASEONLY", INT2FIX(LIBRAW_SONYARW2_BASEONLY));
	rb_define_const(rb_mSonyArw2Option, "DELTAONLY", INT2FIX(LIBRAW_SONYARW2_DELTAONLY));
	rb_define_const(rb_mSonyArw2Option, "DELTAZEROBASE", INT2FIX(LIBRAW_SONYARW2_DELTAZEROBASE));
	rb_define_const(rb_mSonyArw2Option, "DELTATOVALUE", INT2FIX(LIBRAW_SONYARW2_DELTATOVALUE));


	// LibRaw::Dp2qOption

	rb_mDp2qOption = rb_define_module_under(rb_mLibRaw, "Dp2qOption");

	rb_define_const(rb_mDp2qOption, "NONE", INT2FIX(LIBRAW_DP2QOPT_NONE));
	rb_define_const(rb_mDp2qOption, "INTERPOLATERG", INT2FIX(LIBRAW_DP2Q_INTERPOLATERG));
	rb_define_const(rb_mDp2qOption, "INTERPOLATEAF", INT2FIX(LIBRAW_DP2Q_INTERPOLATEAF));


	// LibRaw::DecoderFlags

	rb_mDecoderFlag = rb_define_module_under(rb_mLibRaw, "DecoderFlag");

	//rb_define_const(rb_mDecoderFlag, "LEGACY", INT2FIX(LIBRAW_DECODER_LEGACY));
	//rb_define_const(rb_mDecoderFlag, "FLATFIELD", INT2FIX(LIBRAW_DECODER_FLATFIELD));
	rb_define_const(rb_mDecoderFlag, "USEBAYER2", INT2FIX(LIBRAW_DECODER_USEBAYER2));
	rb_define_const(rb_mDecoderFlag, "HASCURVE", INT2FIX(LIBRAW_DECODER_HASCURVE));
	rb_define_const(rb_mDecoderFlag, "SONYARW2", INT2FIX(LIBRAW_DECODER_SONYARW2));
	rb_define_const(rb_mDecoderFlag, "TRYRAWSPEED", INT2FIX(LIBRAW_DECODER_TRYRAWSPEED));
	rb_define_const(rb_mDecoderFlag, "OWNALLOC", INT2FIX(LIBRAW_DECODER_OWNALLOC));
	rb_define_const(rb_mDecoderFlag, "FIXEDMAXC", INT2FIX(LIBRAW_DECODER_FIXEDMAXC));
	rb_define_const(rb_mDecoderFlag, "NOTSET", INT2FIX(LIBRAW_DECODER_NOTSET));


	// LibRaw::Warning

	rb_mWarning = rb_define_module_under(rb_mLibRaw, "Warning");

	rb_define_const(rb_mWarning, "NONE", INT2FIX(LIBRAW_WARN_NONE));
	rb_define_const(rb_mWarning, "FOVEON_NOMATRIX", INT2FIX(LIBRAW_WARN_FOVEON_NOMATRIX));
	rb_define_const(rb_mWarning, "FOVEON_INVALIDWB", INT2FIX(LIBRAW_WARN_FOVEON_INVALIDWB));
	rb_define_const(rb_mWarning, "BAD_CAMERA_WB", INT2FIX(LIBRAW_WARN_BAD_CAMERA_WB));
	rb_define_const(rb_mWarning, "NO_METADATA", INT2FIX(LIBRAW_WARN_NO_METADATA));
	rb_define_const(rb_mWarning, "NO_JPEGLIB", INT2FIX(LIBRAW_WARN_NO_JPEGLIB));
	rb_define_const(rb_mWarning, "NO_EMBEDDED_PROFILE", INT2FIX(LIBRAW_WARN_NO_EMBEDDED_PROFILE));
	rb_define_const(rb_mWarning, "NO_INPUT_PROFILE", INT2FIX(LIBRAW_WARN_NO_INPUT_PROFILE));
	rb_define_const(rb_mWarning, "BAD_OUTPUT_PROFILE", INT2FIX(LIBRAW_WARN_BAD_OUTPUT_PROFILE));
	rb_define_const(rb_mWarning, "NO_BADPIXELMAP", INT2FIX(LIBRAW_WARN_NO_BADPIXELMAP));
	rb_define_const(rb_mWarning, "BAD_DARKFRAME_FILE", INT2FIX(LIBRAW_WARN_BAD_DARKFRAME_FILE));
	rb_define_const(rb_mWarning, "BAD_DARKFRAME_DIM", INT2FIX(LIBRAW_WARN_BAD_DARKFRAME_DIM));
	rb_define_const(rb_mWarning, "NO_JASPER", INT2FIX(LIBRAW_WARN_NO_JASPER));
	rb_define_const(rb_mWarning, "RAWSPEED_PROBLEM", INT2FIX(LIBRAW_WARN_RAWSPEED_PROBLEM));
	rb_define_const(rb_mWarning, "RAWSPEED_UNSUPPORTED", INT2FIX(LIBRAW_WARN_RAWSPEED_UNSUPPORTED));
	rb_define_const(rb_mWarning, "RAWSPEED_PROCESSED", INT2FIX(LIBRAW_WARN_RAWSPEED_PROCESSED));
	rb_define_const(rb_mWarning, "FALLBACK_TO_AHD", INT2FIX(LIBRAW_WARN_FALLBACK_TO_AHD));


	// LibRaw::Progress

	rb_mProgress = rb_define_module_under(rb_mLibRaw, "Progress");

	rb_define_const(rb_mProgress, "START", INT2FIX(LIBRAW_PROGRESS_START));
	rb_define_const(rb_mProgress, "OPEN", INT2FIX(LIBRAW_PROGRESS_OPEN));
	rb_define_const(rb_mProgress, "IDENTIFY", INT2FIX(LIBRAW_PROGRESS_IDENTIFY));
	rb_define_const(rb_mProgress, "SIZE_ADJUST", INT2FIX(LIBRAW_PROGRESS_SIZE_ADJUST));
	rb_define_const(rb_mProgress, "LOAD_RAW", INT2FIX(LIBRAW_PROGRESS_LOAD_RAW));
	rb_define_const(rb_mProgress, "RAW2_IMAGE", INT2FIX(LIBRAW_PROGRESS_RAW2_IMAGE));
	rb_define_const(rb_mProgress, "REMOVE_ZEROES", INT2FIX(LIBRAW_PROGRESS_REMOVE_ZEROES));
	rb_define_const(rb_mProgress, "BAD_PIXELS", INT2FIX(LIBRAW_PROGRESS_BAD_PIXELS));
	rb_define_const(rb_mProgress, "DARK_FRAME", INT2FIX(LIBRAW_PROGRESS_DARK_FRAME));
	rb_define_const(rb_mProgress, "FOVEON_INTERPOLATE", INT2FIX(LIBRAW_PROGRESS_FOVEON_INTERPOLATE));
	rb_define_const(rb_mProgress, "SCALE_COLORS", INT2FIX(LIBRAW_PROGRESS_SCALE_COLORS));
	rb_define_const(rb_mProgress, "PRE_INTERPOLATE", INT2FIX(LIBRAW_PROGRESS_PRE_INTERPOLATE));
	rb_define_const(rb_mProgress, "INTERPOLATE", INT2FIX(LIBRAW_PROGRESS_INTERPOLATE));
	rb_define_const(rb_mProgress, "MIX_GREEN", INT2FIX(LIBRAW_PROGRESS_MIX_GREEN));
	rb_define_const(rb_mProgress, "MEDIAN_FILTER", INT2FIX(LIBRAW_PROGRESS_MEDIAN_FILTER));
	rb_define_const(rb_mProgress, "HIGHLIGHTS", INT2FIX(LIBRAW_PROGRESS_HIGHLIGHTS));
	rb_define_const(rb_mProgress, "FUJI_ROTATE", INT2FIX(LIBRAW_PROGRESS_FUJI_ROTATE));
	rb_define_const(rb_mProgress, "FLIP", INT2FIX(LIBRAW_PROGRESS_FLIP));
	rb_define_const(rb_mProgress, "APPLY_PROFILE", INT2FIX(LIBRAW_PROGRESS_APPLY_PROFILE));
	rb_define_const(rb_mProgress, "CONVERT_RGB", INT2FIX(LIBRAW_PROGRESS_CONVERT_RGB));
	rb_define_const(rb_mProgress, "STRETCH", INT2FIX(LIBRAW_PROGRESS_STRETCH));

	rb_define_const(rb_mProgress, "STAGE20", INT2FIX(LIBRAW_PROGRESS_STAGE20));
	rb_define_const(rb_mProgress, "STAGE21", INT2FIX(LIBRAW_PROGRESS_STAGE21));
	rb_define_const(rb_mProgress, "STAGE22", INT2FIX(LIBRAW_PROGRESS_STAGE22));
	rb_define_const(rb_mProgress, "STAGE23", INT2FIX(LIBRAW_PROGRESS_STAGE23));
	rb_define_const(rb_mProgress, "STAGE24", INT2FIX(LIBRAW_PROGRESS_STAGE24));
	rb_define_const(rb_mProgress, "STAGE25", INT2FIX(LIBRAW_PROGRESS_STAGE25));
	rb_define_const(rb_mProgress, "STAGE26", INT2FIX(LIBRAW_PROGRESS_STAGE26));
	rb_define_const(rb_mProgress, "STAGE27", INT2FIX(LIBRAW_PROGRESS_STAGE27));

	rb_define_const(rb_mProgress, "THUMB_LOAD", INT2FIX(LIBRAW_PROGRESS_THUMB_LOAD));
	rb_define_const(rb_mProgress, "TRESERVED1", INT2FIX(LIBRAW_PROGRESS_TRESERVED1));
	rb_define_const(rb_mProgress, "TRESERVED2", INT2FIX(LIBRAW_PROGRESS_TRESERVED2));
	rb_define_const(rb_mProgress, "TRESERVED3", INT2FIX(LIBRAW_PROGRESS_TRESERVED3));


	// LibRaw::ThumbnailFormat

	rb_mThumbnailFormat = rb_define_module_under(rb_mLibRaw, "ThumbnailFormat");

	rb_define_const(rb_mThumbnailFormat, "UNKNOWN", INT2FIX(LIBRAW_THUMBNAIL_UNKNOWN));
	rb_define_const(rb_mThumbnailFormat, "JPEG", INT2FIX(LIBRAW_THUMBNAIL_JPEG));
	rb_define_const(rb_mThumbnailFormat, "BITMAP", INT2FIX(LIBRAW_THUMBNAIL_BITMAP));
	rb_define_const(rb_mThumbnailFormat, "LAYER", INT2FIX(LIBRAW_THUMBNAIL_LAYER));
	rb_define_const(rb_mThumbnailFormat, "ROLLEI", INT2FIX(LIBRAW_THUMBNAIL_ROLLEI));



	// class

	// LibRaw::RawObject

	rb_cRawObject = rb_define_class_under(rb_mLibRaw, "RawObject", rb_cObject);

	rb_define_method(rb_cRawObject, "initialize", RUBY_METHOD_FUNC(rb_raw_object_initialize), 0);
	rb_define_method(rb_cRawObject, "open_file", RUBY_METHOD_FUNC(rb_raw_object_open_file), 1);
	rb_define_method(rb_cRawObject, "open_buffer", RUBY_METHOD_FUNC(rb_raw_object_open_buffer), 1);
	rb_define_method(rb_cRawObject, "unpack", RUBY_METHOD_FUNC(rb_raw_object_unpack), 0);
	rb_define_method(rb_cRawObject, "unpack_thumb", RUBY_METHOD_FUNC(rb_raw_object_unpack_thumb), 0);


	// LibRaw::ImageSize

	rb_cImageSize = rb_define_class_under(rb_mLibRaw, "ImageSize", rb_cObject);

	rb_define_method(rb_cImageSize, "initialize", RUBY_METHOD_FUNC(rb_image_size_initialize), 1);
	rb_define_method(rb_cImageSize, "rb_image_size_raw_height", RUBY_METHOD_FUNC(rb_image_size_raw_height), 0);
	rb_define_method(rb_cImageSize, "rb_image_size_raw_width", RUBY_METHOD_FUNC(rb_image_size_raw_width), 0);
	rb_define_method(rb_cImageSize, "rb_image_size_height", RUBY_METHOD_FUNC(rb_image_size_height), 0);
	rb_define_method(rb_cImageSize, "rb_image_size_width", RUBY_METHOD_FUNC(rb_image_size_width), 0);
	rb_define_method(rb_cImageSize, "rb_image_size_top_margin", RUBY_METHOD_FUNC(rb_image_size_top_margin), 0);
	rb_define_method(rb_cImageSize, "rb_image_size_left_margin", RUBY_METHOD_FUNC(rb_image_size_left_margin), 0);
	rb_define_method(rb_cImageSize, "rb_image_size_iheight", RUBY_METHOD_FUNC(rb_image_size_iheight), 0);
	rb_define_method(rb_cImageSize, "rb_image_size_iwidth", RUBY_METHOD_FUNC(rb_image_size_iwidth), 0);
	rb_define_method(rb_cImageSize, "rb_image_size_raw_pitch", RUBY_METHOD_FUNC(rb_image_size_raw_pitch), 0);
	rb_define_method(rb_cImageSize, "rb_image_size_pixel_aspect", RUBY_METHOD_FUNC(rb_image_size_pixel_aspect), 0);
	rb_define_method(rb_cImageSize, "rb_image_size_flip", RUBY_METHOD_FUNC(rb_image_size_flip), 0);


	// Error

	// LibRaw::RawError
	rb_eRawError = rb_define_class_under(rb_mLibRaw, "RawError", rb_eStandardError);

	// LibRaw::UnspecifiedError
	rb_eUnspecifiedError = rb_define_class_under(rb_mLibRaw, "UnspecifiedError", rb_eRawError);

	// LibRaw::FileUnsupported
	rb_eFileUnsupported = rb_define_class_under(rb_mLibRaw, "FileUnsupported", rb_eRawError);

	// LibRaw::RequestForNonexistentImage
	rb_eRequestForNonexistentImage = rb_define_class_under(rb_mLibRaw, "RequestForNonexistentImage", rb_eRawError);

	// LibRaw::OutOfOrderCall
	rb_eOutOfOrderCall = rb_define_class_under(rb_mLibRaw, "OutOfOrderCall", rb_eRawError);

	// LibRaw::NoThumbnail
	rb_eNoThumbnail = rb_define_class_under(rb_mLibRaw, "NoThumbnail", rb_eRawError);

	// LibRaw::UnsupportedThumbnail
	rb_eUnsupportedThumbnail = rb_define_class_under(rb_mLibRaw, "UnsupportedThumbnail", rb_eRawError);

	// LibRaw::InputClosed
	rb_eInputClosed = rb_define_class_under(rb_mLibRaw, "InputClosed", rb_eRawError);

	// LibRaw::UnsufficientMemory
	rb_eUnsufficientMemory = rb_define_class_under(rb_mLibRaw, "UnsufficientMemory", rb_eRawError);

	// LibRaw::DataError
	rb_eDataError = rb_define_class_under(rb_mLibRaw, "DataError", rb_eRawError);

	// LibRaw::IOError
	rb_eIOError = rb_define_class_under(rb_mLibRaw, "IOError", rb_eRawError);

	// LibRaw::CancelledByCallback
	rb_eCancelledByCallback = rb_define_class_under(rb_mLibRaw, "CancelledByCallback", rb_eRawError);

	// LibRaw::BadCrop
	rb_eBadCrop = rb_define_class_under(rb_mLibRaw, "BadCrop", rb_eRawError);
}

}
