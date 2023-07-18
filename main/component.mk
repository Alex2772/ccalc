#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)
CXXFLAGS += -frtti -DLUA_32BITS -DSOL_NO_EXCEPTIONS -DCCALC_MODEL_MAJOR=4 -I~/esp-idf/CCOS/ -std=c++14
