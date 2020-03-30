#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/ssl.hpp>


#include "Manager.h"

void WorkerThread(boost::shared_ptr<boost::asio::io_context> io_context)
{

	while (true)
	{
		
		try {
			boost::system::error_code ec;
			io_context->run(ec);
			if (ec)
			{
				std::cout << "[BoostException][" << __FUNCTION__ << "]:" << ec.message() << std::endl;
			}
			break;
		}
		catch (...)
		{

		}
	}
}

int main(int argc, char* argv[])
{
	boost::shared_ptr<boost::asio::io_service> io_context = 
		boost::shared_ptr<boost::asio::io_context>(
			new boost::asio::io_context()
			);
	boost::shared_ptr<boost::asio::io_service::work> work = 
		boost::shared_ptr<boost::asio::io_service::work>(
			new boost::asio::io_service::work(*io_context)
			);

	boost::shared_ptr<boost::asio::ssl::context> ssl_context =
		boost::shared_ptr<boost::asio::ssl::context>(
			new boost::asio::ssl::context(boost::asio::ssl::context::method::sslv23)
			);



	boost::asio::strand<boost::asio::io_service::executor_type> strand(io_context->get_executor());

	ssl_context->set_default_verify_paths();
	//ssl_context->load_verify_file("ca.pem");
	boost::thread_group tg;

	for (int i = 0; i < 4; i++)
	{
		tg.create_thread(boost::bind(&WorkerThread, io_context));
	}
	std::cout << "WebCrawler started with (" << tg.size() << ") threads." << std::endl;
	
	//boost::asio::bind_executor(strand, []() { std::cout << "from callback"; });

	Manager mgr(io_context, ssl_context);


	tg.join_all();
	

	std::cout << "Do you reckon this line displays?" << std::endl;

	return 0;
}