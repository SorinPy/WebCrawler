#pragma once

#include <boost/asio.hpp>
#include <boost/chrono.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <map>

#include <vector>
#include <list>
#include <iostream>
#include <regex>

#include "Page.h"

class Parser;

typedef boost::function<void(boost::shared_ptr<Parser>, double, std::vector< boost::shared_ptr<Page> >)> parser_callback;



class Parser : public boost::enable_shared_from_this<Parser>
{
public:
	Parser(boost::shared_ptr<boost::asio::io_context> io_context) : m_io_context(io_context) {}

	
	void ParsePage(boost::shared_ptr<Page> page, parser_callback cb);

	boost::shared_ptr<Page> getPage() { return m_page; }
private:

	void parseHeaders();

	boost::shared_ptr<Page> m_page;

	boost::shared_ptr<boost::asio::io_context> m_io_context;

	boost::chrono::high_resolution_clock::time_point m_start_time;
	boost::chrono::high_resolution_clock::time_point m_end_time;

	void searchLinks(std::string, std::vector<std::string>&);

	bool isValidUrl(const std::string&);

	parser_callback m_cb;
};

