ADDITIONAL_FLAGS=
#ADDITIONAL_FLAGS+=-std=c++0x

CXXFLAGS+=$(ADDITIONAL_FLAGS) -std=c++0x
LDFLAGS+=$(ADDITIONAL_FLAGS) -lv4l2 -lv4lconvert

