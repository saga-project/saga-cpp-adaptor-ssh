
-include ../config/make.cfg

SAGA_SRC          = $(wildcard *.cpp)
SAGA_OBJ          = $(SAGA_SRC:%.cpp=%.o)

SAGA_ADAPTOR_NAME = scp_file
SAGA_ADAPTOR_INI  = scp_file_adaptor.ini
SAGA_ADAPTOR_OBJ  = $(SAGA_OBJ)

SAGA_ADAPTOR_PACKAGES = filesystem namespace

include $(SAGA_LOCATION)/share/saga/make/saga.adaptor.mk

distclean::
	@rm -f scp_file_adaptor.ini

