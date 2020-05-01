#pragma once

#include <boost/chrono.hpp>
#include <boost/asio/streambuf.hpp>
#include <map>

class Page
{

public:

	Page(time_t);
	Page();

	boost::asio::streambuf& getHeadersBuff() { return m_headers; }
	boost::asio::streambuf& getContentBuff() { return m_content; }

	std::string getAddress() { return m_address; }
	void setAddress(std::string addr) { m_address = addr; }

	bool headerLoaded() { return m_headersLoaded; }
	void headerLoaded(bool val) { m_headersLoaded = val; }

	void setParseDate(time_t date) { m_parseDate = boost::chrono::system_clock::from_time_t(date); }
	void setParseDate(boost::chrono::time_point<boost::chrono::system_clock> time) { m_parseDate = time; }
	time_t getParseDate() { return boost::chrono::duration_cast<boost::chrono::seconds>(m_parseDate.time_since_epoch()).count(); }

	void setAddDate(time_t date) { m_addDate = boost::chrono::system_clock::from_time_t(date); }
	time_t getAddDate() { return boost::chrono::duration_cast<boost::chrono::seconds>(m_addDate.time_since_epoch()).count(); }

	void setBaseAddress(std::string str) { m_baseAddress = str; }
	std::string getBaseAddress() { return m_baseAddress; }


	//public non encapsulated vars

	int StatusCode;
	std::map<std::string, std::string> ResponseHeaders;
	std::string MainDomain;

	boost::asio::streambuf RawResponse;

	bool Timeout;
private:
	boost::chrono::time_point<boost::chrono::system_clock> m_addDate;
	boost::chrono::time_point<boost::chrono::system_clock> m_parseDate;

	std::string m_address;
	std::string m_baseAddress;

	boost::asio::streambuf m_headers;
	boost::asio::streambuf m_content;

	bool m_headersLoaded;
};

