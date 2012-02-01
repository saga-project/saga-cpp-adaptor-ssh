#  Copyright (c) 2005-2007 Andre Merzky (andre@merzky.net)
# 
#  Distributed under the Boost Software License, 
#  Version 1.0. (See accompanying LICENSE file 
#  or copy at http://www.boost.org/LICENSE_1_0.txt)

-include ../config/make.cfg

SAGA_SUBDIRS      = test

SAGA_SRC          = $(wildcard *.cpp)
SAGA_OBJ          = $(SAGA_SRC:%.cpp=%.o)

SAGA_ADAPTOR_TYPE = filesystem
SAGA_ADAPTOR_NAME = sshfs_file
SAGA_ADAPTOR_INI  = sshfs_file_adaptor.ini
SAGA_ADAPTOR_OBJ  = $(SAGA_OBJ)

SAGA_ADAPTOR_PACKAGES = filesystem namespace

-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.adaptor.mk

# SAGA_CPPINCS     += -I../ssh_suite
# SAGA_LDFLAGS     += -L../ssh_suite -lsaga_adaptor_ssh_suite

distclean::
	@$(RM) $(SAGA_ADAPTOR_INI)

