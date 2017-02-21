all: # test
	nsmake gtest -g3 test/test_ExpIndexedList.cpp

test:
	nsmake gtest -g3 test/test_*.cpp


.PHONY: test