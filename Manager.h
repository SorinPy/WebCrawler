#pragma once

#include <boost/asio.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/system/error_code.hpp>
#include <boost/bind.hpp>
#include <boost/chrono.hpp>
#include <iostream>
#include <exception>

#include "Page.h"
#include "WebRequest.h"
#include "Database.h"

using tcp = boost::asio::ip::tcp;

class Manager
{
public:
	Manager(boost::shared_ptr < boost::asio::io_context>, boost::shared_ptr<boost::asio::ssl::context>);

private:
	boost::shared_ptr<boost::asio::io_context> m_io_context;

	boost::shared_ptr<boost::asio::ssl::context> m_ssl_context;

	boost::shared_ptr<WebRequest> wr;

	std::vector<boost::shared_ptr<Page>> m_pageQueue;

	Database m_db;
};

