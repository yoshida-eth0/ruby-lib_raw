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

VALUE rb_cIParam;
VALUE rb_cImageSize;
VALUE rb_cImgOther;
VALUE rb_cOutputParam;
VALUE rb_cMakerNote;
VALUE rb_cLensInfo;

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

void output_param_native_resource_delete(OutputParamNativeResource * p)
{
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

libraw_output_params_t* get_output_params(VALUE self)
{
	VALUE resource = rb_iv_get(self, "output_param_native_resource");
	if (resource==Qnil) {
		return NULL;
	}

	OutputParamNativeResource *p = NULL;
	Data_Get_Struct(resource, OutputParamNativeResource, p);
	if (p) {
		return &p->params;
	}

	return NULL;
}

void copy_lib_raw(VALUE dst, VALUE src)
{
	VALUE resource = rb_iv_get(src, "lib_raw_native_resource");
	rb_iv_set(dst, "lib_raw_native_resource", resource);
}

void check_errors(int e)
{
	enum LibRaw_errors errorcode = (LibRaw_errors)e;
	if (e!=LIBRAW_SUCCESS) {
		const char *mess = libraw_strerror(e);

		switch (e) {
		case LIBRAW_UNSPECIFIED_ERROR:
			rb_raise(rb_eUnspecifiedError, "%s", mess);
		case LIBRAW_FILE_UNSUPPORTED:
			rb_raise(rb_eFileUnsupported, "%s", mess);
		case LIBRAW_REQUEST_FOR_NONEXISTENT_IMAGE:
			rb_raise(rb_eRequestForNonexistentImage, "%s", mess);
		case LIBRAW_OUT_OF_ORDER_CALL:
			rb_raise(rb_eOutOfOrderCall, "%s", mess);
		case LIBRAW_NO_THUMBNAIL:
			rb_raise(rb_eNoThumbnail, "%s", mess);
		case LIBRAW_UNSUPPORTED_THUMBNAIL:
			rb_raise(rb_eUnsupportedThumbnail, "%s", mess);
		case LIBRAW_INPUT_CLOSED:
			rb_raise(rb_eInputClosed, "%s", mess);
		case LIBRAW_UNSUFFICIENT_MEMORY:
			rb_raise(rb_eUnsufficientMemory, "%s", mess);
		case LIBRAW_DATA_ERROR:
			rb_raise(rb_eDataError, "%s", mess);
		case LIBRAW_IO_ERROR:
			rb_raise(rb_eIOError, "%s", mess);
		case LIBRAW_CANCELLED_BY_CALLBACK:
			rb_raise(rb_eCancelledByCallback, "%s", mess);
		case LIBRAW_BAD_CROP:
			rb_raise(rb_eBadCrop, "%s", mess);
		default:
			rb_raise(rb_eRawError, "%s", mess);
		}
	}
}


// LibRaw::RawObject

void apply_rawobject(VALUE self)
{
	LibRaw *raw = get_lib_raw(self);
	if (raw) {
		apply_data(self, &raw->imgdata);
	}
}

void apply_data(VALUE self, libraw_data_t *p)
{
	if (p) {
		// size
		VALUE size = rb_iv_get(self, "@size");
		if (size==Qnil) {
			size = rb_class_new_instance(0, NULL, rb_cImageSize);
			rb_iv_set(self, "@size", size);
		}
		apply_image_size(size, &p->sizes);

		// idata
		VALUE idata = rb_iv_get(self, "@idata");
		if (idata==Qnil) {
			idata = rb_class_new_instance(0, NULL, rb_cIParam);
			rb_iv_set(self, "@idata", idata);
		}
		apply_iparam(idata, &p->idata);

		// lens
		VALUE lens = rb_iv_get(self, "@lens");
		if (lens==Qnil) {
			lens = rb_class_new_instance(0, NULL, rb_cLensInfo);
			rb_iv_set(self, "@lens", lens);
		}
		apply_lensinfo(lens, &p->lens);

		// TODO: params

		// TODO: color
/*
		VALUE color = rb_iv_get(self, "@color");
		if (color==Qnil) {
			color = rb_class_new_instance(0, NULL, rb_cColorData);
			rb_iv_set(self, "@color", color);
		}
		apply_colordata(color, &p->color);
*/

		// other
		VALUE other = rb_iv_get(self, "@other");
		if (other==Qnil) {
			other = rb_class_new_instance(0, NULL, rb_cImgOther);
			rb_iv_set(self, "@other", other);
		}
		apply_imgother(other, &p->other);

		// param
		VALUE param = rb_iv_get(self, "@param");
		if (param==Qnil) {
			param = rb_class_new_instance(0, NULL, rb_cOutputParam);
			rb_iv_set(self, "@param", param);
		}
		apply_output_param(param, &p->params);
	}
}

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

	apply_data(self, &p->libraw->imgdata);

	return self;
}

VALUE rb_raw_object_open_file(VALUE self, VALUE filename)
{
	LibRaw *libraw = get_lib_raw(self);

	char *name = RSTRING_PTR(rb_obj_as_string(filename));
	int ret = libraw->open_file(name);
	apply_rawobject(self);
	check_errors(ret);

	return Qtrue;
}

VALUE rb_raw_object_open_buffer(VALUE self, VALUE buff)
{
	LibRaw *libraw = get_lib_raw(self);

	int ret = libraw->open_buffer(RSTRING_PTR(buff), RSTRING_LEN(buff));
	apply_rawobject(self);
	check_errors(ret);

	return Qtrue;
}

VALUE rb_raw_object_unpack(VALUE self)
{
	LibRaw *libraw = get_lib_raw(self);

	int ret = libraw->unpack();
	check_errors(ret);

	return Qtrue;
}

VALUE rb_raw_object_unpack_thumb(VALUE self)
{
	LibRaw *libraw = get_lib_raw(self);

	int ret = libraw->unpack_thumb();
	check_errors(ret);

	return Qtrue;
}

VALUE rb_raw_object_recycle_datastream(VALUE self)
{
	LibRaw *libraw = get_lib_raw(self);

	libraw->recycle_datastream();

	return Qnil;
}

VALUE rb_raw_object_recycle(VALUE self)
{
	LibRaw *libraw = get_lib_raw(self);

	libraw->recycle();

	return Qnil;
}

VALUE rb_raw_object_dcraw_ppm_tiff_writer(VALUE self, VALUE filename)
{
	LibRaw *libraw = get_lib_raw(self);

	const char *name = RSTRING_PTR(filename);
	int ret = libraw->dcraw_ppm_tiff_writer(name);
	check_errors(ret);

	return Qtrue;
}

VALUE rb_raw_object_dcraw_thumb_writer(VALUE self, VALUE filename)
{
	LibRaw *libraw = get_lib_raw(self);

	const char *name = RSTRING_PTR(filename);
	int ret = libraw->dcraw_thumb_writer(name);
	check_errors(ret);

	return Qtrue;
}

VALUE rb_raw_object_dcraw_process(VALUE self, VALUE param)
{
	LibRaw *libraw = get_lib_raw(self);
	libraw_output_params_t *params = get_output_params(param);

	memmove(&libraw->imgdata.params, params, sizeof(libraw_output_params_t));

	int ret = libraw->dcraw_process();
	check_errors(ret);

	return Qtrue;
}


// LibRaw::IParam

void apply_iparam(VALUE self, libraw_iparams_t *p)
{
	if (p) {
		rb_iv_set(self, "@make", rb_str_new2(p->make));
		rb_iv_set(self, "@model", rb_str_new2(p->model));
		rb_iv_set(self, "@software", rb_str_new2(p->software));
		rb_iv_set(self, "@raw_count", INT2FIX(p->raw_count));
		rb_iv_set(self, "@dng_version", INT2FIX(p->dng_version));
		rb_iv_set(self, "@is_foveon", p->is_foveon ? Qtrue : Qfalse);
		rb_iv_set(self, "@colors", INT2FIX(p->colors));
		rb_iv_set(self, "@filters", INT2FIX(p->filters));
		rb_iv_set(self, "@cdesc", rb_str_new2(p->cdesc));
	}
}


// LibRaw::ImageSize

void apply_image_size(VALUE self, libraw_image_sizes_t *p)
{
	if (p) {
		rb_iv_set(self, "@raw_height", INT2FIX(p->raw_height));
		rb_iv_set(self, "@raw_width", INT2FIX(p->raw_width));
		rb_iv_set(self, "@height", INT2FIX(p->height));
		rb_iv_set(self, "@width", INT2FIX(p->width));
		rb_iv_set(self, "@top_margin", INT2FIX(p->top_margin));
		rb_iv_set(self, "@left_margin", INT2FIX(p->left_margin));
		rb_iv_set(self, "@iheight", INT2FIX(p->iheight));
		rb_iv_set(self, "@iwidth", INT2FIX(p->iwidth));
		rb_iv_set(self, "@raw_pitch", INT2FIX(p->raw_pitch));
		rb_iv_set(self, "@pixel_aspect", rb_float_new(p->pixel_aspect));
		rb_iv_set(self, "@flip", INT2FIX(p->flip));
	}
}


// LibRaw::ColorData

void apply_colordata(VALUE self, libraw_colordata_t *p)
{
	if (p) {
		// TODO
	}
}


// LibRaw::ImgOther

void apply_imgother(VALUE self, libraw_imgother_t *p)
{
	if (p) {
		rb_iv_set(self, "@iso_speed", rb_float_new(p->iso_speed));
		rb_iv_set(self, "@shutter", rb_float_new(p->shutter));
		rb_iv_set(self, "@aperture", rb_float_new(p->aperture));
		rb_iv_set(self, "@focal_len", rb_float_new(p->focal_len));
		rb_iv_set(self, "@timestamp", INT2FIX(p->timestamp));
		rb_iv_set(self, "@shot_order", INT2FIX(p->shot_order));
		rb_iv_set(self, "@desc", rb_str_new2(p->desc));
		rb_iv_set(self, "@artist", rb_str_new2(p->artist));

		// TODO: parsed_gps
	}
}


// LibRaw::OutputParam

VALUE rb_output_param_initialize(VALUE self)
{
	OutputParamNativeResource *p = ALLOC(OutputParamNativeResource);


	memset(&p->params, 0, sizeof(libraw_output_params_t));

	double aber[4] = {1,1,1,1};
	double gamm[6] = { 0.45,4.5,0,0,0,0 };
	unsigned greybox[4] =  { 0, 0, UINT_MAX, UINT_MAX };
	unsigned cropbox[4] =  { 0, 0, UINT_MAX, UINT_MAX };

	memmove(&p->params.aber,&aber,sizeof(aber));
	memmove(&p->params.gamm,&gamm,sizeof(gamm));
	memmove(&p->params.greybox,&greybox,sizeof(greybox));
	memmove(&p->params.cropbox,&cropbox,sizeof(cropbox));

	p->params.bright=1;
	p->params.use_camera_matrix=1;
	p->params.user_flip=-1;
	p->params.user_black=-1;
	p->params.user_cblack[0]=p->params.user_cblack[1]=p->params.user_cblack[2]=p->params.user_cblack[3]=-1000001;
	p->params.user_sat=-1;
	p->params.user_qual=-1;
	p->params.output_color=1;
	p->params.output_bps=8;
	p->params.use_fuji_rotate=1;
	p->params.exp_shift = 1.0; 
	p->params.auto_bright_thr = LIBRAW_DEFAULT_AUTO_BRIGHTNESS_THRESHOLD;
	p->params.adjust_maximum_thr= LIBRAW_DEFAULT_ADJUST_MAXIMUM_THRESHOLD;
	p->params.use_rawspeed = 1; 
	p->params.no_auto_scale = 0; 
	p->params.no_interpolation = 0; 
	p->params.sraw_ycc = 0; 
	p->params.force_foveon_x3f = 0; 
	p->params.x3f_flags = LIBRAW_DP2Q_INTERPOLATERG|LIBRAW_DP2Q_INTERPOLATEAF;
	p->params.sony_arw2_options = 0; 
	p->params.sony_arw2_posterization_thr = 0; 
	p->params.green_matching = 0; 
	p->params.coolscan_nef_gamma = 1.0f;


	VALUE resource = Data_Wrap_Struct(CLASS_OF(self), 0, output_param_native_resource_delete, p);
	rb_iv_set(self, "output_param_native_resource", resource);

	apply_output_param(self, &p->params);

	return self;
}

void apply_output_param(VALUE self, libraw_output_params_t *p)
{
	if (p) {
		// TODO: greybox
		// TODO: cropbox
		// TODO: aber
		// TODO: gamm
		// TODO: user_mul
		rb_iv_set(self, "@shot_select", INT2FIX(p->shot_select));
		rb_iv_set(self, "@bright", rb_float_new(p->bright));
		rb_iv_set(self, "@threshold", rb_float_new(p->threshold));
		rb_iv_set(self, "@half_size", INT2FIX(p->half_size));
		rb_iv_set(self, "@four_color_rgb", INT2FIX(p->four_color_rgb));
		rb_iv_set(self, "@highlight", INT2FIX(p->highlight));
		rb_iv_set(self, "@use_auto_wb", p->use_auto_wb ? Qtrue : Qfalse);
		rb_iv_set(self, "@use_camera_wb", p->use_camera_wb ? Qtrue : Qfalse);
		rb_iv_set(self, "@use_camera_matrix", p->use_camera_matrix ? Qtrue : Qfalse);
		rb_iv_set(self, "@output_color", INT2FIX(p->output_color));
		// TODO: output_profile
		// TODO: camera_profile
		// TODO: bad_pixels
		// TODO: dark_frame
		rb_iv_set(self, "@output_bps", INT2FIX(p->output_bps));
		rb_iv_set(self, "@output_tiff", INT2FIX(p->output_tiff));
		rb_iv_set(self, "@user_flip", INT2FIX(p->user_flip));
		rb_iv_set(self, "@user_qual", INT2FIX(p->user_qual));
		rb_iv_set(self, "@user_black", INT2FIX(p->user_black));
		// TODO: user_cblack
		rb_iv_set(self, "@user_sat", INT2FIX(p->user_sat));
		rb_iv_set(self, "@med_passes", INT2FIX(p->med_passes));
		rb_iv_set(self, "@auto_bright_thr", rb_float_new(p->auto_bright_thr));
		rb_iv_set(self, "@adjust_maximum_thr", rb_float_new(p->adjust_maximum_thr));
		rb_iv_set(self, "@no_auto_bright", p->no_auto_bright ? Qtrue : Qfalse);
		rb_iv_set(self, "@use_fuji_rotate", p->use_fuji_rotate ? Qtrue : Qfalse);
		rb_iv_set(self, "@green_matching", INT2FIX(p->green_matching));

		// DCB parameters
		rb_iv_set(self, "@dcb_iterations", INT2FIX(p->dcb_iterations));
		rb_iv_set(self, "@dcb_enhance_fl", INT2FIX(p->dcb_enhance_fl));
		rb_iv_set(self, "@fbdd_noiserd", INT2FIX(p->fbdd_noiserd));

		// VCD parameters
		rb_iv_set(self, "@eeci_refine", INT2FIX(p->eeci_refine));
		rb_iv_set(self, "@es_med_passes", INT2FIX(p->es_med_passes));

		// AMaZE
		rb_iv_set(self, "@ca_correc", INT2FIX(p->ca_correc));
		rb_iv_set(self, "@cared", rb_float_new(p->cared));
		rb_iv_set(self, "@cablue", rb_float_new(p->cablue));
		rb_iv_set(self, "@cfaline", INT2FIX(p->cfaline));
		rb_iv_set(self, "@linenoise", rb_float_new(p->linenoise));
		rb_iv_set(self, "@cfa_clean", INT2FIX(p->cfa_clean));
		rb_iv_set(self, "@lclean", rb_float_new(p->lclean));
		rb_iv_set(self, "@cclean", rb_float_new(p->cclean));
		rb_iv_set(self, "@cfa_green", INT2FIX(p->cfa_green));
		rb_iv_set(self, "@green_thresh", rb_float_new(p->green_thresh));
		rb_iv_set(self, "@exp_correc", INT2FIX(p->exp_correc));
		rb_iv_set(self, "@exp_shift", rb_float_new(p->exp_shift));
		rb_iv_set(self, "@exp_preser", rb_float_new(p->exp_preser));

		// WF debanding
		rb_iv_set(self, "@wf_debanding", INT2FIX(p->wf_debanding));
		// TODO: wf_deband_treshold

		// Raw speed
		rb_iv_set(self, "@use_rawspeed", p->use_rawspeed ? Qtrue : Qfalse);

		// Disable Auto-scale
		rb_iv_set(self, "@no_auto_scale", p->no_auto_scale ? Qtrue : Qfalse);

		// Disable intepolation
		rb_iv_set(self, "@no_interpolation", p->no_interpolation ? Qtrue : Qfalse);

		// Disable sRAW YCC to RGB conversion
		rb_iv_set(self, "@sraw_ycc", INT2FIX(p->sraw_ycc));

		// Force use x3f data decoding either if demosaic pack GPL2 enabled
		rb_iv_set(self, "@force_foveon_x3f", INT2FIX(p->force_foveon_x3f));
		rb_iv_set(self, "@x3f_flags", INT2FIX(p->x3f_flags));

		// Sony ARW2 digging mode
		rb_iv_set(self, "@sony_arw2_options", INT2FIX(p->sony_arw2_options));
		rb_iv_set(self, "@sony_arw2_posterization_thr", INT2FIX(p->sony_arw2_posterization_thr));

		// Nikon Coolscan
		rb_iv_set(self, "@coolscan_nef_gamma", rb_float_new(p->coolscan_nef_gamma));
	}
}

VALUE rb_output_param_greybox(VALUE self, VALUE x, VALUE y, VALUE w, VALUE h)
{
	libraw_output_params_t *params = get_output_params(self);

	params->greybox[0] = NUM2LONG(x);
	params->greybox[1] = NUM2LONG(y);
	params->greybox[2] = NUM2LONG(w);
	params->greybox[3] = NUM2LONG(h);

	apply_output_param(self, params);

	return self;
}

VALUE rb_output_param_cropbox(VALUE self, VALUE x, VALUE y, VALUE w, VALUE h)
{
	libraw_output_params_t *params = get_output_params(self);

	params->cropbox[0]= NUM2LONG(x);
	params->cropbox[1]= NUM2LONG(y);
	params->cropbox[2]= NUM2LONG(w);
	params->cropbox[3]= NUM2LONG(h);

	apply_output_param(self, params);

	return self;
}

VALUE rb_output_param_gamma(VALUE self, VALUE pwr, VALUE ts)
{
	libraw_output_params_t *params = get_output_params(self);

	params->gamm[0] = RFLOAT_VALUE(rb_Float(pwr));
	params->gamm[1] = RFLOAT_VALUE(rb_Float(ts));

	apply_output_param(self, params);

	return self;
}

VALUE rb_output_param_whitebalance(VALUE self, VALUE r, VALUE g, VALUE b, VALUE g2)
{
	libraw_output_params_t *params = get_output_params(self);

	params->user_mul[0] = RFLOAT_VALUE(rb_Float(r));
	params->user_mul[1] = RFLOAT_VALUE(rb_Float(g));
	params->user_mul[2] = RFLOAT_VALUE(rb_Float(b));
	params->user_mul[3] = RFLOAT_VALUE(rb_Float(g2));

	apply_output_param(self, params);

	return self;
}

VALUE rb_output_param_set_bright(VALUE self, VALUE val)
{
	libraw_output_params_t *params = get_output_params(self);

	params->bright = RFLOAT_VALUE(rb_Float(val));

	apply_output_param(self, params);

	return self;
}

VALUE rb_output_param_set_highlight(VALUE self, VALUE val)
{
	libraw_output_params_t *params = get_output_params(self);

	params->highlight = NUM2LONG(val);

	apply_output_param(self, params);

	return self;
}

VALUE rb_output_param_set_use_auto_wb(VALUE self, VALUE val)
{
	libraw_output_params_t *params = get_output_params(self);

	params->use_auto_wb = !(val==Qnil || val==Qfalse);

	apply_output_param(self, params);

	return self;
}

VALUE rb_output_param_set_use_camera_wb(VALUE self, VALUE val)
{
	libraw_output_params_t *params = get_output_params(self);

	params->use_camera_wb = !(val==Qnil || val==Qfalse);

	apply_output_param(self, params);

	return self;
}

VALUE rb_output_param_set_fbdd_noiserd(VALUE self, VALUE val)
{
	libraw_output_params_t *params = get_output_params(self);

	params->fbdd_noiserd = NUM2LONG(val);

	apply_output_param(self, params);

	return self;
}


// LibRaw::MakerNote

void apply_makernote(VALUE self, libraw_makernotes_lens_t *p)
{
	if (p) {
		rb_iv_set(self, "@lens_id", INT2FIX(p->LensID));
		rb_iv_set(self, "@lens", rb_str_new2(p->Lens));
		rb_iv_set(self, "@lens_format", INT2FIX(p->LensFormat));
		rb_iv_set(self, "@lens_mount", INT2FIX(p->LensMount));
		rb_iv_set(self, "@cam_id", INT2FIX(p->CamID));
		rb_iv_set(self, "@camera_format", INT2FIX(p->CameraFormat));
		rb_iv_set(self, "@camera_mount", INT2FIX(p->CameraMount));
		rb_iv_set(self, "@body", rb_str_new2(p->body));
		rb_iv_set(self, "@focal_type", INT2FIX(p->FocalType));
		rb_iv_set(self, "@lens_features_pre", rb_str_new2(p->LensFeatures_pre));
		rb_iv_set(self, "@lens_features_suf", rb_str_new2(p->LensFeatures_suf));
		rb_iv_set(self, "@min_focal", rb_float_new(p->MinFocal));
		rb_iv_set(self, "@max_focal", rb_float_new(p->MaxFocal));
		rb_iv_set(self, "@max_ap_4_min_focal", rb_float_new(p->MaxAp4MinFocal));
		rb_iv_set(self, "@max_ap_4_max_focal", rb_float_new(p->MaxAp4MaxFocal));
		rb_iv_set(self, "@min_ap_4_min_focal", rb_float_new(p->MinAp4MinFocal));
		rb_iv_set(self, "@min_ap_4_max_focal", rb_float_new(p->MinAp4MaxFocal));
		rb_iv_set(self, "@max_ap", rb_float_new(p->MaxAp));
		rb_iv_set(self, "@min_ap", rb_float_new(p->MinAp));
		rb_iv_set(self, "@cur_focal", rb_float_new(p->CurFocal));
		rb_iv_set(self, "@cur_ap", rb_float_new(p->CurAp));
		rb_iv_set(self, "@max_ap_4_cur_focal", rb_float_new(p->MaxAp4CurFocal));
		rb_iv_set(self, "@min_ap_4_cur_focal", rb_float_new(p->MinAp4CurFocal));
		rb_iv_set(self, "@lens_f_stops", rb_float_new(p->LensFStops));
		rb_iv_set(self, "@teleconverter_id", INT2FIX(p->TeleconverterID));
		rb_iv_set(self, "@teleconverter", rb_str_new2(p->Teleconverter));
		rb_iv_set(self, "@adapter_id", INT2FIX(p->AdapterID));
		rb_iv_set(self, "@adapter", rb_str_new2(p->Adapter));
		rb_iv_set(self, "@attachment_id", INT2FIX(p->AttachmentID));
		rb_iv_set(self, "@attachment", rb_str_new2(p->Attachment));
		rb_iv_set(self, "@canon_focal_units", INT2FIX(p->CanonFocalUnits));
		rb_iv_set(self, "@focal_length_in_35mm_format", rb_float_new(p->FocalLengthIn35mmFormat));
	}
}


// LibRaw::LensInfo

void apply_lensinfo(VALUE self, libraw_lensinfo_t *p)
{
	if (p) {
		rb_iv_set(self, "@min_focal", rb_float_new(p->MinFocal));
		rb_iv_set(self, "@max_focal", rb_float_new(p->MaxFocal));
		rb_iv_set(self, "@max_ap_4_min_focal", rb_float_new(p->MaxAp4MinFocal));
		rb_iv_set(self, "@max_ap_4_max_focal", rb_float_new(p->MaxAp4MaxFocal));
		rb_iv_set(self, "@exif_max_ap", rb_float_new(p->EXIF_MaxAp));
		rb_iv_set(self, "@lens_make", rb_str_new2(p->LensMake));
		rb_iv_set(self, "@lens", rb_str_new2(p->Lens));
		rb_iv_set(self, "@focal_length_in_35mm_format", INT2FIX(p->FocalLengthIn35mmFormat));
		// TODO: nikon
		// TODO: dng

		// makernotes
		VALUE makernotes = rb_iv_get(self, "@makernotes");
		if (makernotes==Qnil) {
			makernotes = rb_class_new_instance(0, NULL, rb_cMakerNote);
			rb_iv_set(self, "@makernotes", makernotes);
		}
		apply_makernote(makernotes, &p->makernotes);
	}
}



extern "C" void Init_lib_raw(void)
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
	rb_define_const(rb_mCameraMount, "MFT", INT2FIX(LIBRAW_MOUNT_mFT));
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

	rb_define_attr(rb_cRawObject, "size", 1, 0);
	rb_define_attr(rb_cRawObject, "idata", 1, 0);
	rb_define_attr(rb_cRawObject, "lens", 1, 0);
	rb_define_attr(rb_cRawObject, "other", 1, 0);
	rb_define_attr(rb_cRawObject, "param", 1, 0);

	rb_define_method(rb_cRawObject, "initialize", RUBY_METHOD_FUNC(rb_raw_object_initialize), 0);
	rb_define_method(rb_cRawObject, "open_file", RUBY_METHOD_FUNC(rb_raw_object_open_file), 1);
	rb_define_method(rb_cRawObject, "open_buffer", RUBY_METHOD_FUNC(rb_raw_object_open_buffer), 1);
	rb_define_method(rb_cRawObject, "unpack", RUBY_METHOD_FUNC(rb_raw_object_unpack), 0);
	rb_define_method(rb_cRawObject, "unpack_thumb", RUBY_METHOD_FUNC(rb_raw_object_unpack_thumb), 0);
	rb_define_method(rb_cRawObject, "recycle_datastream", RUBY_METHOD_FUNC(rb_raw_object_recycle_datastream), 0);
	rb_define_method(rb_cRawObject, "recycle", RUBY_METHOD_FUNC(rb_raw_object_recycle), 0);
	rb_define_method(rb_cRawObject, "dcraw_ppm_tiff_writer", RUBY_METHOD_FUNC(rb_raw_object_dcraw_ppm_tiff_writer), 1);
	rb_define_method(rb_cRawObject, "dcraw_thumb_writer", RUBY_METHOD_FUNC(rb_raw_object_dcraw_thumb_writer), 1);
	rb_define_method(rb_cRawObject, "dcraw_process", RUBY_METHOD_FUNC(rb_raw_object_dcraw_process), 1);


	// LibRaw::IParam

	rb_cIParam = rb_define_class_under(rb_mLibRaw, "IParam", rb_cObject);

	rb_define_attr(rb_cIParam, "make", 1, 0);
	rb_define_attr(rb_cIParam, "model", 1, 0);
	rb_define_attr(rb_cIParam, "software", 1, 0);
	rb_define_attr(rb_cIParam, "raw_count", 1, 0);
	rb_define_attr(rb_cIParam, "dng_version", 1, 0);
	rb_define_attr(rb_cIParam, "is_foveon", 1, 0);
	rb_define_attr(rb_cIParam, "colors", 1, 0);
	rb_define_attr(rb_cIParam, "filters", 1, 0);
	rb_define_attr(rb_cIParam, "cdesc", 1, 0);


	// LibRaw::ImageSize

	rb_cImageSize = rb_define_class_under(rb_mLibRaw, "ImageSize", rb_cObject);

	rb_define_attr(rb_cImageSize, "raw_height", 1, 0);
	rb_define_attr(rb_cImageSize, "raw_width", 1, 0);
	rb_define_attr(rb_cImageSize, "height", 1, 0);
	rb_define_attr(rb_cImageSize, "width", 1, 0);
	rb_define_attr(rb_cImageSize, "top_margin", 1, 0);
	rb_define_attr(rb_cImageSize, "left_margin", 1, 0);
	rb_define_attr(rb_cImageSize, "iheight", 1, 0);
	rb_define_attr(rb_cImageSize, "iwidth", 1, 0);
	rb_define_attr(rb_cImageSize, "raw_pitch", 1, 0);
	rb_define_attr(rb_cImageSize, "pixel_aspect", 1, 0);
	rb_define_attr(rb_cImageSize, "flip", 1, 0);


	// LibRaw::ImgOther

	rb_cImgOther = rb_define_class_under(rb_mLibRaw, "ImgOther", rb_cObject);

	rb_define_attr(rb_cImgOther, "iso_speed", 1, 0);
	rb_define_attr(rb_cImgOther, "shutter", 1, 0);
	rb_define_attr(rb_cImgOther, "aperture", 1, 0);
	rb_define_attr(rb_cImgOther, "focal_len", 1, 0);
	rb_define_attr(rb_cImgOther, "timestamp", 1, 0);
	rb_define_attr(rb_cImgOther, "shot_order", 1, 0);
	rb_define_attr(rb_cImgOther, "desc", 1, 0);
	rb_define_attr(rb_cImgOther, "artist", 1, 0);


	// LibRaw::OutputParam

	rb_cOutputParam = rb_define_class_under(rb_mLibRaw, "OutputParam", rb_cObject);

	rb_define_attr(rb_cOutputParam, "shot_select", 1, 0);
	rb_define_attr(rb_cOutputParam, "bright", 1, 0);
	rb_define_attr(rb_cOutputParam, "threshold", 1, 0);
	rb_define_attr(rb_cOutputParam, "half_size", 1, 0);
	rb_define_attr(rb_cOutputParam, "four_color_rgb", 1, 0);
	rb_define_attr(rb_cOutputParam, "highlight", 1, 0);
	rb_define_attr(rb_cOutputParam, "use_auto_wb", 1, 0);
	rb_define_attr(rb_cOutputParam, "use_camera_wb", 1, 0);
	rb_define_attr(rb_cOutputParam, "use_camera_matrix", 1, 0);
	rb_define_attr(rb_cOutputParam, "output_color", 1, 0);
	rb_define_attr(rb_cOutputParam, "output_bps", 1, 0);
	rb_define_attr(rb_cOutputParam, "output_tiff", 1, 0);
	rb_define_attr(rb_cOutputParam, "user_flip", 1, 0);
	rb_define_attr(rb_cOutputParam, "user_qual", 1, 0);
	rb_define_attr(rb_cOutputParam, "user_black", 1, 0);
	rb_define_attr(rb_cOutputParam, "user_sat", 1, 0);
	rb_define_attr(rb_cOutputParam, "med_passes", 1, 0);
	rb_define_attr(rb_cOutputParam, "auto_bright_thr", 1, 0);
	rb_define_attr(rb_cOutputParam, "adjust_maximum_thr", 1, 0);
	rb_define_attr(rb_cOutputParam, "no_auto_bright", 1, 0);
	rb_define_attr(rb_cOutputParam, "use_fuji_rotate", 1, 0);
	rb_define_attr(rb_cOutputParam, "green_matching", 1, 0);

	rb_define_attr(rb_cOutputParam, "dcb_iterations", 1, 0);
	rb_define_attr(rb_cOutputParam, "dcb_enhance_fl", 1, 0);
	rb_define_attr(rb_cOutputParam, "fbdd_noiserd", 1, 0);

	rb_define_attr(rb_cOutputParam, "eeci_refine", 1, 0);
	rb_define_attr(rb_cOutputParam, "es_med_passes", 1, 0);

	rb_define_attr(rb_cOutputParam, "ca_correc", 1, 0);
	rb_define_attr(rb_cOutputParam, "cared", 1, 0);
	rb_define_attr(rb_cOutputParam, "cablue", 1, 0);
	rb_define_attr(rb_cOutputParam, "cfaline", 1, 0);
	rb_define_attr(rb_cOutputParam, "linenoise", 1, 0);
	rb_define_attr(rb_cOutputParam, "cfa_clean", 1, 0);
	rb_define_attr(rb_cOutputParam, "lclean", 1, 0);
	rb_define_attr(rb_cOutputParam, "cclean", 1, 0);
	rb_define_attr(rb_cOutputParam, "cfa_green", 1, 0);
	rb_define_attr(rb_cOutputParam, "green_thresh", 1, 0);
	rb_define_attr(rb_cOutputParam, "exp_correc", 1, 0);
	rb_define_attr(rb_cOutputParam, "exp_shift", 1, 0);
	rb_define_attr(rb_cOutputParam, "exp_preser", 1, 0);

	rb_define_attr(rb_cOutputParam, "wf_debanding", 1, 0);

	rb_define_attr(rb_cOutputParam, "use_rawspeed", 1, 0);

	rb_define_attr(rb_cOutputParam, "no_auto_scale", 1, 0);

	rb_define_attr(rb_cOutputParam, "no_interpolation", 1, 0);

	rb_define_attr(rb_cOutputParam, "sraw_ycc", 1, 0);

	rb_define_attr(rb_cOutputParam, "force_foveon_x3f", 1, 0);
	rb_define_attr(rb_cOutputParam, "x3f_flags", 1, 0);

	rb_define_attr(rb_cOutputParam, "sony_arw2_options", 1, 0);
	rb_define_attr(rb_cOutputParam, "sony_arw2_posterization_thr", 1, 0);

	rb_define_attr(rb_cOutputParam, "coolscan_nef_gamma", 1, 0);

	rb_define_method(rb_cOutputParam, "initialize", RUBY_METHOD_FUNC(rb_output_param_initialize), 0);
	rb_define_method(rb_cOutputParam, "greybox", RUBY_METHOD_FUNC(rb_output_param_greybox), 4);
	rb_define_method(rb_cOutputParam, "cropbox", RUBY_METHOD_FUNC(rb_output_param_cropbox), 4);
	rb_define_method(rb_cOutputParam, "gamma", RUBY_METHOD_FUNC(rb_output_param_gamma), 2);
	rb_define_method(rb_cOutputParam, "whitebalance", RUBY_METHOD_FUNC(rb_output_param_whitebalance), 4);
	rb_define_method(rb_cOutputParam, "bright=", RUBY_METHOD_FUNC(rb_output_param_set_bright), 1);
	rb_define_method(rb_cOutputParam, "highlight=", RUBY_METHOD_FUNC(rb_output_param_set_highlight), 1);
	rb_define_method(rb_cOutputParam, "use_auto_wb=", RUBY_METHOD_FUNC(rb_output_param_set_use_auto_wb), 1);
	rb_define_method(rb_cOutputParam, "use_camera_wb=", RUBY_METHOD_FUNC(rb_output_param_set_use_camera_wb), 1);
	rb_define_method(rb_cOutputParam, "fbdd_noiserd=", RUBY_METHOD_FUNC(rb_output_param_set_fbdd_noiserd), 1);


	// LibRaw::MakerNote

	rb_cMakerNote = rb_define_class_under(rb_mLibRaw, "MakerNote", rb_cObject);

	rb_define_attr(rb_cMakerNote, "lens_id", 1, 0);
	rb_define_attr(rb_cMakerNote, "lens", 1, 0);
	rb_define_attr(rb_cMakerNote, "lens_format", 1, 0);
	rb_define_attr(rb_cMakerNote, "lens_mount", 1, 0);
	rb_define_attr(rb_cMakerNote, "cam_id", 1, 0);
	rb_define_attr(rb_cMakerNote, "camera_format", 1, 0);
	rb_define_attr(rb_cMakerNote, "camera_mount", 1, 0);
	rb_define_attr(rb_cMakerNote, "body", 1, 0);
	rb_define_attr(rb_cMakerNote, "focal_type", 1, 0);
	rb_define_attr(rb_cMakerNote, "lens_features_pre", 1, 0);
	rb_define_attr(rb_cMakerNote, "lens_features_suf", 1, 0);
	rb_define_attr(rb_cMakerNote, "min_focal", 1, 0);
	rb_define_attr(rb_cMakerNote, "max_focal", 1, 0);
	rb_define_attr(rb_cMakerNote, "max_ap_4_min_focal", 1, 0);
	rb_define_attr(rb_cMakerNote, "max_ap_4_max_focal", 1, 0);
	rb_define_attr(rb_cMakerNote, "min_ap_4_min_focal", 1, 0);
	rb_define_attr(rb_cMakerNote, "min_ap_4_max_focal", 1, 0);
	rb_define_attr(rb_cMakerNote, "max_ap", 1, 0);
	rb_define_attr(rb_cMakerNote, "min_ap", 1, 0);
	rb_define_attr(rb_cMakerNote, "cur_focal", 1, 0);
	rb_define_attr(rb_cMakerNote, "cur_ap", 1, 0);
	rb_define_attr(rb_cMakerNote, "max_ap_4_cur_focal", 1, 0);
	rb_define_attr(rb_cMakerNote, "min_ap_4_cur_focal", 1, 0);
	rb_define_attr(rb_cMakerNote, "lens_f_stops", 1, 0);
	rb_define_attr(rb_cMakerNote, "teleconverter_id", 1, 0);
	rb_define_attr(rb_cMakerNote, "teleconverter", 1, 0);
	rb_define_attr(rb_cMakerNote, "adapter_id", 1, 0);
	rb_define_attr(rb_cMakerNote, "adapter", 1, 0);
	rb_define_attr(rb_cMakerNote, "attachment_id", 1, 0);
	rb_define_attr(rb_cMakerNote, "attachment", 1, 0);
	rb_define_attr(rb_cMakerNote, "canon_focal_units", 1, 0);
	rb_define_attr(rb_cMakerNote, "focal_length_in_35mm_format", 1, 0);


	// LibRaw::LensInfo

	rb_cLensInfo = rb_define_class_under(rb_mLibRaw, "LensInfo", rb_cObject);
	rb_define_attr(rb_cLensInfo, "min_focal", 1, 0);
	rb_define_attr(rb_cLensInfo, "max_focal", 1, 0);
	rb_define_attr(rb_cLensInfo, "max_ap_4_min_focal", 1, 0);
	rb_define_attr(rb_cLensInfo, "max_ap_4_max_focal", 1, 0);
	rb_define_attr(rb_cLensInfo, "exif_max_ap", 1, 0);
	rb_define_attr(rb_cLensInfo, "lens_make", 1, 0);
	rb_define_attr(rb_cLensInfo, "lens", 1, 0);
	rb_define_attr(rb_cLensInfo, "focal_length_in_35mm_format", 1, 0);
	rb_define_attr(rb_cLensInfo, "nikon", 1, 0);
	rb_define_attr(rb_cLensInfo, "dng", 1, 0);
	rb_define_attr(rb_cLensInfo, "makernotes", 1, 0);


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
