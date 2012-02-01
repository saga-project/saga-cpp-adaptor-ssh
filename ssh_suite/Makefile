#  Copyright (c) 2005-2006 Andre Merzky (andre@merzky.net)
#  Copyright (c) 2006-2007 Ole Weidner (oweidner@cct.lsu.edu)
# 
#  Use, modification and distribution is subject to the Boost Software
#  License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)

SAGA_ADAPTOR_SUITE            = yes

-include ../config/make.cfg

SAGA_SRC                      = $(wildcard *.cpp)
SAGA_OBJ                      = $(SAGA_SRC:%.cpp=%.o)

SAGA_ADAPTOR_UTIL_NAME        = ssh
SAGA_ADAPTOR_UTIL_OBJ         = $(SAGA_OBJ)

SAGA_ADAPTOR_UTIL_OPTIONS     = ssh

-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.adaptor.util.mk 

