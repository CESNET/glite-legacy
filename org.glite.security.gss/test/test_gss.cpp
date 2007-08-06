#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/TestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>

#include "glite_gss.h"

class GSSTest: public  CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(GSSTest);
	CPPUNIT_TEST(echo);
	CPPUNIT_TEST(echo);
	CPPUNIT_TEST(bigecho);
	CPPUNIT_TEST(errorTest);
	CPPUNIT_TEST_SUITE_END();

public:
	void echo();
	void bigecho();
	void errorTest();

	void setUp();

private:
	edg_wll_GssCred	my_cred;
	char *		my_subject;
	int		sock, port;
	struct timeval	timeout;
	
	void replier();
};


void GSSTest::replier() {
	edg_wll_GssConnection	conn;
	edg_wll_GssStatus	stat;
	struct sockaddr_in      a;
	socklen_t		alen = sizeof(a);
	int                     s, len;
	char 			buf[8*BUFSIZ];
	
	std::cerr << "replier " << getpid() << std::endl;
	
	if ( (s = accept(sock, (struct sockaddr *) &a, &alen)) < 0 ) exit(1);
	
	if ( edg_wll_gss_accept(my_cred, s, &timeout, &conn, &stat) ) exit(1);

	while ( (len = edg_wll_gss_read(&conn, buf, sizeof(buf), &timeout, &stat)) >= 0 ) {
		if ( edg_wll_gss_write(&conn, buf, len, &timeout, &stat) ) exit(1);
	}	

	exit(0);
}


void GSSTest::setUp(void) {
	pid_t pid;
	edg_wll_GssStatus stat;
	struct sockaddr_in      a;
	socklen_t 		alen = sizeof(a);
	char *			cred_file = NULL;
	char *			key_file = NULL;
	char * 			to = getenv("GSS_TEST_TIMEOUT");

	timeout.tv_sec = to ? atoi(to) : 10 ;
	timeout.tv_usec = 0;
	
	key_file = cred_file = getenv("X509_USER_PROXY");
	CPPUNIT_ASSERT_MESSAGE("credential file", cred_file);
	
	if (edg_wll_gss_acquire_cred_gsi(cred_file, key_file, &my_cred, &my_subject, &stat))
		CPPUNIT_ASSERT_MESSAGE("gss_acquire_cred", 0);
	
        sock = socket(PF_INET,SOCK_STREAM,0);
	CPPUNIT_ASSERT_MESSAGE("socket()", sock >= 0);

        a.sin_family = AF_INET;
        a.sin_port = 0;
        a.sin_addr.s_addr = INADDR_ANY;

        if (bind(sock,(struct sockaddr *) &a,sizeof(a))) {
		CPPUNIT_ASSERT_MESSAGE("bind()", 0);
        }

        if (listen(sock,1)) {
		CPPUNIT_ASSERT_MESSAGE("listen()", 0);
	}

	getsockname(sock,(struct sockaddr *) &a,&alen);
	port = ntohs(a.sin_port);

	if ( !(pid = fork()) ) replier();
	else close(sock);
}



void GSSTest::echo()
{
	edg_wll_GssConnection	conn;
	edg_wll_GssStatus	stat;
	size_t			total;
	int			err;
	char 			buf[] = "f843fejwfanczn nc4*&686%$$&^(*)*#$@WSH";	
	char			buf2[100];	

	std::cerr << "echo " << getpid() << std::endl;

	err = edg_wll_gss_connect(my_cred, "localhost", port, &timeout, &conn, &stat);
	CPPUNIT_ASSERT_MESSAGE("edg_wll_gss_connect()", !err);
	
	err = edg_wll_gss_write(&conn, buf, strlen(buf)+1, &timeout, &stat);
	CPPUNIT_ASSERT_MESSAGE("edg_wll_gss_write()", !err);
	
	err = edg_wll_gss_read_full(&conn, buf2, strlen(buf)+1, &timeout, &total, &stat);
	CPPUNIT_ASSERT_MESSAGE("edg_wll_gss_read_full()", !err);

	CPPUNIT_ASSERT(strlen(buf)+1 == total && !strcmp(buf,buf2) );

	edg_wll_gss_close(&conn, &timeout);
		
}

void GSSTest::bigecho()
{
	edg_wll_GssConnection	conn;
	edg_wll_GssStatus	stat;
	size_t			total;
	int			err;
	char 			buf[7*BUFSIZ];
	char			buf2[7*BUFSIZ];	

	std::cerr << "bigecho " << getpid() << std::endl;

	err = edg_wll_gss_connect(my_cred, "localhost", port, &timeout, &conn, &stat);
	CPPUNIT_ASSERT_MESSAGE("edg_wll_gss_connect()", !err);
	
	err = edg_wll_gss_write(&conn, buf, sizeof buf, &timeout, &stat);
	CPPUNIT_ASSERT_MESSAGE("edg_wll_gss_write()", !err);
	
	err = edg_wll_gss_read_full(&conn, buf2, sizeof buf2, &timeout, &total, &stat);
	CPPUNIT_ASSERT_MESSAGE("edg_wll_gss_read_full()", !err);

	CPPUNIT_ASSERT(sizeof buf == total && !memcmp(buf,buf2,sizeof buf) );

	edg_wll_gss_close(&conn, &timeout);
		
}


void GSSTest::errorTest()
{
	edg_wll_GssConnection	conn;
	edg_wll_GssStatus	stat;
	int			err;
	char *			msg = NULL;


	err = edg_wll_gss_connect(my_cred, "xxx.porno.net", port, &timeout, &conn, &stat);
	if (err) edg_wll_gss_get_error(&stat, "gss_connect()", &msg);
	CPPUNIT_ASSERT_MESSAGE("edg_wll_gss_get_error()", msg);
}


CPPUNIT_TEST_SUITE_REGISTRATION( GSSTest );

int main (int ac,const char *av[])
{
	assert(ac == 2);
	std::ofstream	xml(av[1]);

	CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();
	CppUnit::TestRunner runner;

	CppUnit::TestResult controller;
	CppUnit::TestResultCollector result;
	controller.addListener( &result );

	runner.addTest(suite);
	runner.run(controller);


	CppUnit::XmlOutputter xout( &result, xml );
	CppUnit::CompilerOutputter tout( &result, std::cout);
	xout.write();
	tout.write();

	return result.wasSuccessful() ? 0 : 1 ;
}
