require "mkmf"

have_library("stdc++")
have_library("raw_r")


#$CFLAGS << " -I#{File.dirname(__FILE__)}/src"
#$CFLAGS << " -I#{File.dirname(__FILE__)}/internal"

#try_link("#{File.dirname(__FILE__)}/src/libraw_cxx.cpp")

=begin
$srcs = %w{
./lib_raw.cpp
./internal/aahd_demosaic.cpp
./internal/dcraw_common.cpp
./internal/dcraw_fileio.cpp
./internal/demosaic_packs.cpp
./internal/dht_demosaic.cpp
./internal/libraw_x3f.cpp
./internal/wf_filtering.cpp
./src/libraw_c_api.cpp
./src/libraw_cxx.cpp
./src/libraw_datastream.cpp
}
p $srcs
=end

#have_library("libraw_r")

create_makefile("lib_raw/lib_raw")
