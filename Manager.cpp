#include "Manager.h"

Manager::Manager(boost::shared_ptr<boost::asio::io_context> io_context, boost::shared_ptr<boost::asio::ssl::context> ssl_context): m_io_context(io_context) ,m_ssl_context(ssl_context)
{
	std::vector<Page> v;
	m_db.getPages(m_pageQueue , 10);

	std::for_each(m_pageQueue.begin(), m_pageQueue.end(), [](boost::shared_ptr<Page> page) {
		std::cout << page->getAddDate() << ":" << page->getAddress() << std::endl;
	});

	m_db.insertPages(m_pageQueue);

	boost::shared_ptr<Page> page = boost::shared_ptr<Page>(new Page(1585215154));

	page->setAddress("https://facebook.com");

	wr = boost::shared_ptr<WebRequest>(new WebRequest(m_io_context, m_ssl_context));
	/*wr->LoadPage(page, [io_context](boost::shared_ptr<Page> page , double time_spend) {
		std::cout << "WR spend:" << time_spend << " millis" << std::endl;
		std::istream headers(&page->getHeadersBuff());
		std::string line;
		while (!headers.eof())
		{
			std::getline(headers, line);
			std::cout << line << std::endl;
		}
		io_context->stop();
	});*/


}
