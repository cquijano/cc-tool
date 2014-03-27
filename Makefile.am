ACLOCAL_AMFLAGS = -I m4
man_MANS = man/cc-tool.1

AM_CPPFLAGS = $(LIBUSB_CFLAGS) $(BOOST_CPPFLAGS) -O3 -Isrc/programmer -Isrc/common -Isrc/usb -Isrc -Isrc/application
AM_LDFLAGS = \
	$(BOOST_FILESYSTEM_LDFLAGS) \
	$(BOOST_REGEX_LDFLAGS) \
	$(BOOST_SYSTEM_LDFLAGS) \
	$(BOOST_PROGRAM_OPTIONS_LDFLAGS)

#	$(BOOST_THREADS_LDFLAGS)
      
LDADD = $(LIBUSB_LIBS) 

LIBS = -s \
	$(BOOST_FILESYSTEM_LIBS) \
	$(BOOST_REGEX_LIBS) \
	$(BOOST_SYSTEM_LIBS) \
	$(BOOST_PROGRAM_OPTIONS_LIBS)
   
#	$(BOOST_THREADS_LIBS)

bin_PROGRAMS=cc-tool
cc_tool_SOURCES=src/main.cpp src/application/cc_flasher.cpp src/application/cc_base.cpp \
		src/common/log.cpp src/common/common.cpp src/common/timer.cpp \
		src/usb/usb_device.cpp \
		src/data/binary_file.cpp src/data/data_section.cpp src/data/data_section_store.cpp \
		src/data/file.cpp src/data/hex_file.cpp src/data/read_target.cpp \
		src/data/progress_watcher.cpp \
		src/programmer/cc_253x_254x.cpp src/programmer/cc_251x_111x.cpp \
		src/programmer/cc_243x.cpp src/programmer/cc_programmer.cpp \
		src/programmer/cc_unit_driver.cpp src/programmer/cc_unit_info.cpp

