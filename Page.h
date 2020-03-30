#pragma once

#include <boost/chrono.hpp>
#include <boost/asio/streambuf.hpp>
class Page
{

public:

	Page(time_t);

	boost::asio::streambuf& getHeadersBuff() { return m_headers; }
	boost::asio::streambuf& getContentBuff() { return m_content; }

	std::string getAddress() { return m_address; }
	void setAddress(std::string addr) { m_address = addr; }

	bool headerLoaded() { return m_headersLoaded; }
	void headerLoaded(bool val) { m_headersLoaded = val; }

	void setParseDate(time_t date) { m_parseDate = boost::chrono::system_clock::from_time_t(date); }
	time_t getParseDate() { return boost::chrono::duration_cast<boost::chrono::seconds>(m_parseDate.time_since_epoch()).count(); }

	void setAddDate(time_t date) { m_addDate = boost::chrono::system_clock::from_time_t(date); }
	time_t getAddDate() { return boost::chrono::duration_cast<boost::chrono::seconds>(m_addDate.time_since_epoch()).count(); }

private:
	boost::chrono::time_point<boost::chrono::system_clock> m_addDate;
	boost::chrono::time_point<boost::chrono::system_clock> m_parseDate;

	std::string m_address;

	boost::asio::streambuf m_headers;
	boost::asio::streambuf m_content;

	bool m_headersLoaded;
};
