# Copyright 2018 The Fuchsia Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

LOCAL_DIR := $(GET_LOCAL_DIR)

#
# FIDL compiler host tests.
#

MODULE := $(LOCAL_DIR)

MODULE_TYPE := hosttest

MODULE_NAME := fidl-compiler-test

EXAMPLE_DIR := system/host/fidl/examples/

EXAMPLE_FILES := \
    $(EXAMPLE_DIR)/alignment.fidl \
    $(EXAMPLE_DIR)/empty.fidl \
    $(EXAMPLE_DIR)/enums.fidl \
    $(EXAMPLE_DIR)/events.fidl \
    $(EXAMPLE_DIR)/example-0.fidl \
    $(EXAMPLE_DIR)/example-1.fidl \
    $(EXAMPLE_DIR)/example-2.fidl \
    $(EXAMPLE_DIR)/example-3.fidl \
    $(EXAMPLE_DIR)/example-4.fidl \
    $(EXAMPLE_DIR)/example-5.fidl \
    $(EXAMPLE_DIR)/example-6.fidl \
    $(EXAMPLE_DIR)/example-7.fidl \
    $(EXAMPLE_DIR)/example-8.fidl \
    $(EXAMPLE_DIR)/example-9.fidl \
    $(EXAMPLE_DIR)/interface-ordinals.fidl \
    $(EXAMPLE_DIR)/library-a/point.fidl \
    $(EXAMPLE_DIR)/library-b/view.fidl \
    $(EXAMPLE_DIR)/simple.fidl \
    $(EXAMPLE_DIR)/tables.fidl \
    $(EXAMPLE_DIR)/test.fidl \
    $(EXAMPLE_DIR)/types.fidl \
    $(LOCAL_DIR)/testdata/goodformat.fidl \
    $(LOCAL_DIR)/testdata/badformat.fidl \

BUILDGEN_DIR=$(BUILDDIR)/system/utest/fidl-compiler

$(BUILDGEN_DIR)/examples.cpp: $(EXAMPLE_FILES)
	@$(MKDIR)
	$(call BUILDECHO, generating FIDL test example file)
	$(NOECHO)rm -rf $@ && \
	printf "\
#include <map>\n\
#include <string>\n\
#include \"examples.h\"\n\
// Autogenerated: Do not modify!\n\
std::map<std::string, std::string> Examples::map_ = {\n" >> $@ && \
	for i in $^; do \
	  printf "    {\""$${i}"\", R\"FIDL(""$$(cat $${i})"")FIDL\"}," >> $@; \
	done &&  \
	printf "\
};\n" >> $@ \


MODULE_SRCS := \
    $(LOCAL_DIR)/main.cpp \
    $(LOCAL_DIR)/formatter_unittests.cpp \
    $(LOCAL_DIR)/max_bytes_tests.cpp \
    $(LOCAL_DIR)/max_handle_tests.cpp \
    $(LOCAL_DIR)/superinterface_tests.cpp \
    $(LOCAL_DIR)/using_tests.cpp \
    $(LOCAL_DIR)/visitor_unittests.cpp \
    $(BUILDGEN_DIR)/examples.cpp \

MODULE_COMPILEFLAGS := \
    -Isystem/ulib/unittest/include \
    -Isystem/utest/fidl-compiler \

MODULE_HOST_LIBS := \
    system/host/fidl \
    system/ulib/pretty.hostlib \
    system/ulib/unittest.hostlib \

MODULE_PACKAGE_INCS := \
    $(LOCAL_DIR)/examples.h \

include make/module.mk
