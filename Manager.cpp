#include "Manager.h"

#ifdef _WIN32
#include <windows.h> 

std::atomic<bool> programShouldExit = false;
std::atomic<bool> programClean = false;
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
		// Handle the CTRL-C signal. 
	case CTRL_C_EVENT:
		programShouldExit = true;
		return(TRUE);
		// CTRL-CLOSE: confirm that the user wants to exit. 
	case CTRL_CLOSE_EVENT:
		programShouldExit = true;
		while (!programClean.load())
			Sleep(5);
		return(TRUE);

		// Pass other signals to the next handler. 
	case CTRL_BREAK_EVENT:
		programShouldExit = true;
		return(TRUE);

	case CTRL_LOGOFF_EVENT:
		programShouldExit = true;
		return(TRUE);

	case CTRL_SHUTDOWN_EVENT:
		programShouldExit = true;
		return(TRUE);

	default:
		return FALSE;
	}
}
#endif


Manager::Manager(boost::shared_ptr<boost::asio::io_context> io_context, boost::shared_ptr<boost::asio::ssl::context> ssl_context): m_io_context(io_context) ,m_ssl_context(ssl_context),
					m_strand(io_context->get_executor()),m_info_timer(*io_context,boost::posix_time::seconds(1)),m_db("tcp://localhost:3306","root","")
{

	m_totalPagesLoaded = 0;
	m_totalPagesParsed = 0;
	m_totalTimePageParser = 0;
	m_totalTimeWebRequests = 0;
	m_pagesInWork = 0;

	
	m_info_timer.async_wait(boost::asio::bind_executor(m_strand, boost::bind(&Manager::OnInfoTimerTick, this)));

	m_db.connect();
	
	//m_signals.async_wait(boost::bind(&Manager::OnAppExit,this,_1,_2));

	/*
	TODO:implement cleanup signal handling for linux dist
	*/
#ifdef _WIN32
	if (SetConsoleCtrlHandler(CtrlHandler, TRUE))
	{
		std::cout << "[" <<  __FUNCTION__ << "]: Windows CTRLHANDLER installed." << std::endl;
	}
#endif

}

void Manager::OnAppExit(const boost::system::error_code& error, int signal)
{
	std::cout << "EXIT" << std::endl;
	std::cout << signal << std::endl;
	
	m_io_context->stop();
	while (!m_io_context->stopped());
#ifdef _WIN32
	programClean = true;
#endif // _WIN32

}

void Manager::OnPageRequestEnd(boost::shared_ptr<WebRequest> wr, double time_spend)
{
	
	boost::shared_ptr<Parser> parser = boost::shared_ptr<Parser>(new Parser(m_io_context));
	m_parser_loaded.push_back(parser);

	auto cb = boost::asio::bind_executor(m_strand, boost::bind(&Manager::OnPageParsed, this, _1, _2 , _3));

	boost::asio::post(boost::bind(&Parser::ParsePage, parser, wr->getPage(), cb));

	m_wr_loaded.erase(std::find(m_wr_loaded.begin(),m_wr_loaded.end(),wr));

	std::cout << "WR spend:" << time_spend << " millis. Loaded pages:" << m_wr_loaded.size() << std::endl;
	m_totalPagesLoaded++;
	m_totalTimeWebRequests += time_spend / 1000;
	

}

void Manager::OnPageParsed(boost::shared_ptr<Parser> parser, double time_spend, std::vector<boost::shared_ptr<Page> > pages)
{

	std::for_each(pages.begin(), pages.end(), [](boost::shared_ptr<Page> page) {
		std::cout << page->getAddress() << std::endl;
	});

	m_parser_loaded.erase(std::find(m_parser_loaded.begin(), m_parser_loaded.end(), parser));

	std::cout << "Parser spend:" << time_spend << " millis. Pages in parser:" << m_parser_loaded.size() << std::endl;
	m_totalPagesParsed++;
	m_totalTimePageParser += time_spend / 1000;
}

void Manager::OnInfoTimerTick()
{
#ifdef _WIN32
	if (programShouldExit.load())
	{
		OnAppExit(boost::system::error_code(), 0);
	}
	else {
#endif
		m_db.moveTempPages();
		if (m_pageQueue.size() == 0)
		{
			m_db.getPages(m_pageQueue, 10);
		}
		if (m_pageQueue.size() > 0 && m_wr_loaded.size() < MAX_PAGES_IN_WORK)
		{
			while (m_pageQueue.size() > 0 && m_wr_loaded.size() < MAX_PAGES_IN_WORK)
			{
				auto page = m_pageQueue.back();
				m_pageQueue.pop_back();

				boost::shared_ptr<WebRequest> wr = boost::shared_ptr<WebRequest>(new WebRequest(m_io_context, m_ssl_context));

				m_wr_loaded.push_back(wr);
				auto cb = boost::asio::bind_executor(m_strand, boost::bind(&Manager::OnPageRequestEnd, this, _1, _2));

				boost::asio::post(boost::bind(&WebRequest::LoadPage, wr, page, cb));
			}
		}
		std::cout << "Pages loaded:" << m_totalPagesLoaded << "\t\t Total time spend(seconds):" << m_totalTimeWebRequests << std::endl;
		std::cout << "Pages parsed:" << m_totalPagesParsed << "\t\t Total time spend(seconds):" << m_totalTimePageParser << std::endl;
		m_info_timer.expires_at(m_info_timer.expires_at() + boost::posix_time::seconds(1));
		m_info_timer.async_wait(boost::asio::bind_executor(m_strand, boost::bind(&Manager::OnInfoTimerTick, this)));
#ifdef _WIN32
	}
#endif // _WIN32

}
