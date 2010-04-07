/*
Copyright (c) Members of the EGEE Collaboration. 2004.
See http://www.eu-egee.org/partners/ for details on the
copyright holders.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include<iostream>
#include<string>
#include<pthread.h>
                                   
#include <cppunit/extensions/HelperMacros.h>


class Exception_test : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(Exception_test);
    CPPUNIT_TEST(constructor_case);
    CPPUNIT_TEST(tostring_case);
    CPPUNIT_TEST(stackTrace_case);	
  CPPUNIT_TEST_SUITE_END();


public:

	void setUp();
	void tearDown();
  
    void constructor_case();
    void tostring_case();
    void stackTrace_case();

};



