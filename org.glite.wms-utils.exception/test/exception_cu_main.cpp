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

#include <iostream>
#include <fstream>

#include "exception_cu_suite.h"

#include <cppunit/TestResult.h>
#include <cppunit/TestRunner.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TextOutputter.h>
#include <cppunit/XmlOutputter.h>

using namespace CppUnit;
using namespace std;

int main (int argc , char** argv)
{
	std::ofstream xml("./cppUnit_output.xml",ios::app);
	
	CppUnit::TestResult controller;
	CppUnit::TestResultCollector result;
	controller.addListener( &result );
	
	TestRunner runner;
	runner.addTest(Exception_test::suite());
	runner.run(controller);
	
	CppUnit::XmlOutputter outputter( &result, xml );
	CppUnit::TextOutputter outputter2( &result, std::cerr );
	outputter.write();
	outputter2.write();
	
	return result.wasSuccessful() ? 0 : 1 ;
}
