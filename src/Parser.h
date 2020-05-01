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
#include <sstream>
#include <iostream>

#include "Page.h"

class Parser;

typedef boost::function<void(boost::shared_ptr<Parser>, double, std::vector< boost::shared_ptr<Page> >)> parser_callback;



class Parser : public boost::enable_shared_from_this<Parser>
{
public:
	Parser(boost::asio::io_context& io_context) : m_io_context(io_context),m_followOnlyDomain(false) {}

	
	void ParsePage(boost::shared_ptr<Page> page, parser_callback cb);

	boost::shared_ptr<Page> getPage() { return m_page; }
	void FollowDomain(bool val) { m_followOnlyDomain = val; }
	bool FollowDomain() { return m_followOnlyDomain; }

private:

	void parseHeaders();

	boost::shared_ptr<Page> m_page;

	boost::asio::io_context& m_io_context;

	boost::chrono::high_resolution_clock::time_point m_start_time;
	boost::chrono::high_resolution_clock::time_point m_end_time;

	void searchLinks(std::string, std::vector<std::string>&);

	bool isValidUrl(const std::string&);

	parser_callback m_cb;

	bool m_followOnlyDomain;
};

