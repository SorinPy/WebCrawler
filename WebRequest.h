#pragma once
#include "Connection.h"
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/chrono.hpp>
#include <iostream>
#include <boost/function.hpp>

#include "Page.h"

enum {HTTPS,HTTP};
enum {GET,POST};

typedef boost::function<void(boost::shared_ptr<Page>,double)> wr_callback;

class WebRequest :
	public Connection
{
public:
	WebRequest(boost::shared_ptr<boost::asio::io_context>, boost::shared_ptr<boost::asio::ssl::context>);


	void LoadPage(boost::shared_ptr<Page>);
	void LoadPage(boost::shared_ptr<Page>, wr_callback);

private:

	wr_callback m_callback;

	boost::chrono::high_resolution_clock::time_point m_start_time;
	boost::chrono::high_resolution_clock::time_point m_end_time;

	void ParseLink();

	boost::shared_ptr<Page> m_page;

	int m_RequestMethod;
	int m_RequestType;
	std::string m_RequestHost = "";
	std::string m_RequestQuery = "";
	std::string m_FullLink = "";

	boost::asio::streambuf m_request;
	boost::asio::streambuf m_response;

	boost::asio::streambuf m_content;
	boost::asio::streambuf m_headers;

	// Inherited via Connection
	virtual void OnConnect() override;

	// Inherited via Connection
	virtual void OnSend() override;
	virtual void OnRecv(size_t) override;

	// Inherited via Connection
	virtual void OnHandshake() override;

	// Inherited via Connection
	virtual void OnDisconnect() override;
};
