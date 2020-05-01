#pragma once
#include "Connection.h"
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/chrono.hpp>
#include <iostream>
#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "Page.h"

enum {HTTPS,HTTP};
enum {GET,POST};

class WebRequest;

typedef boost::function<void(boost::shared_ptr<WebRequest>,double)> wr_callback;

class WebRequest : public boost::enable_shared_from_this<WebRequest> ,
	public Connection 
{
public:
	WebRequest(boost::asio::io_context&, boost::shared_ptr<boost::asio::ssl::context>);


	void LoadPage(boost::shared_ptr<Page>);
	void LoadPage(boost::shared_ptr<Page>, wr_callback);

	boost::shared_ptr<Page> getPage() { return m_page; }

private:

	wr_callback m_callback;

	boost::chrono::high_resolution_clock::time_point m_start_time;
	boost::chrono::high_resolution_clock::time_point m_end_time;

	void ParseLink();

	bool m_returned;

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


	size_t m_request_size;
	size_t m_request_size_done;

	std::string m_status;

	// Inherited via Connection
	virtual void OnConnect() override;

	// Inherited via Connection
	virtual void OnSend(const boost::system::error_code& error, std::size_t bytes_transferred) override;
	virtual void OnRecv(size_t) override;

	// Inherited via Connection
	virtual void OnHandshake() override;

	// Inherited via Connection
	virtual void OnDisconnect() override;

	// Inherited via Connection
	virtual void OnTimeout(boost::system::error_code) override;
};
