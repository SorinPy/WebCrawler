#pragma once

#include <boost/asio.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/system/error_code.hpp>
#include <boost/bind.hpp>


#include <iostream>
#include <exception>

using tcp = boost::asio::ip::tcp;

class Connection 
{
private:
	boost::asio::io_context& m_io_context;

	tcp::socket m_socket;
	boost::asio::ssl::stream<tcp::socket> m_ssl_socket;

	tcp::endpoint m_endpoint;

	boost::asio::strand<boost::asio::io_context::executor_type> m_strand;

	boost::asio::steady_timer m_timeout_time;
	bool m_timeout_canceled;

public:

	Connection(boost::asio::io_context&, boost::shared_ptr<boost::asio::ssl::context>);

	void Connect(const std::string&,const std::string& port);

	void Send(boost::asio::streambuf&);

	void Recv(boost::asio::streambuf&);

	void Close();

	boost::asio::strand<boost::asio::io_context::executor_type>& GetStrand() { return m_strand; }

private:

	void setTimeout(int seconds);

	void HandleTimeout(boost::system::error_code);
	virtual void OnTimeout(boost::system::error_code) = 0;

	virtual void OnHandshake() = 0;

	virtual void OnDisconnect() = 0;

	void HandleConnect(boost::system::error_code);
	virtual void OnConnect() = 0;

	void HandleSend(boost::system::error_code, size_t);
	virtual void OnSend(const boost::system::error_code& error,std::size_t bytes_transferred) = 0;

	void HandleRecv(boost::system::error_code,size_t);
	virtual void OnRecv(size_t) = 0;

	void HandleHandshake(boost::system::error_code);

};

