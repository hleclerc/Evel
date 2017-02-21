all: # test
	nsmake --very-verbose gtest -g3 test/test_Dtls.cpp

test:
	nsmake gtest -g3 test/test_*.cpp


.PHONY: test
