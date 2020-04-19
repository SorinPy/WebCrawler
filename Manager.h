#pragma once

#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/system/error_code.hpp>
#include <boost/bind.hpp>
#include <boost/chrono.hpp>

#include <mutex>


#include <algorithm>
#include <iostream>
#include <exception>
#include <list>
#include <vector>
#include <utility>

#include "Page.h"
#include "WebRequest.h"
#include "Database.h"
#include "Parser.h"

using tcp = boost::asio::ip::tcp;

#define MAX_PAGES_IN_WORK 10


class Manager
{
public:
	Manager(boost::shared_ptr < boost::asio::io_context>, boost::shared_ptr<boost::asio::ssl::context>);

private:

	void OnAppExit(const boost::system::error_code&, int);

	void OnPageRequestEnd(boost::shared_ptr<WebRequest> page, double time_spend);
	void OnPageParsed(boost::shared_ptr<Parser>, double, std::vector< boost::shared_ptr<Page> >);

	void OnInfoTimerTick();

	std::list<boost::shared_ptr<WebRequest>> m_wr_loaded;
	std::list<boost::shared_ptr<Parser>> m_parser_loaded;

	boost::shared_ptr<boost::asio::io_context> m_io_context;

	boost::shared_ptr<boost::asio::ssl::context> m_ssl_context;

	boost::asio::strand<boost::asio::io_context::executor_type> m_strand;


	std::vector<boost::shared_ptr<Page>> m_pageQueue;


	Database m_db;

	double m_totalTimeWebRequests;
	double m_totalTimePageParser;

	int m_totalPagesLoaded;
	int m_totalPagesParsed;

	int m_pagesInWork;

	boost::asio::deadline_timer m_info_timer;

	boost::chrono::time_point<boost::chrono::high_resolution_clock> m_appStartTime;

	std::mutex m_mutex;

};

