#include "Connection.h"

Connection::Connection(boost::asio::io_context& io_context, boost::shared_ptr<boost::asio::ssl::context> ssl_context)
	: m_io_context(io_context) 
	, m_socket(io_context)
	, m_ssl_socket(io_context,*ssl_context)
	, m_strand(boost::asio::make_strand(io_context)) 
	, m_timeout_time(io_context,boost::asio::chrono::seconds(60))
{
	m_timeout_canceled = false;
	HandleTimeout({});
}

void Connection::Connect(const std::string& host, const std::string &port)
{
	setTimeout(5);
	try 
	{
		boost::system::error_code ec;
		tcp::resolver resolver(m_io_context);
		auto endpoint = resolver.resolve(host, port,ec);
		if (!ec)
		{
			m_ssl_socket.set_verify_mode(boost::asio::ssl::verify_none);
			/*m_ssl_socket.set_verify_callback([](bool preverified,
				boost::asio::ssl::verify_context& ctx) { 
				
				return true; 
			});*/

			boost::asio::async_connect(m_ssl_socket.lowest_layer(), endpoint, boost::asio::bind_executor(m_strand, boost::bind(&Connection::HandleConnect, this, _1)));
		}
		else {
			m_timeout_time.cancel_one();
			OnDisconnect();
			//std::cout << host << std::endl;
			//std::cout << "[BoostException][" << __FUNCTION__ << "]:" << ec.message() << std::endl;
		}
	}
	catch (std::exception e)
	{
		m_timeout_time.cancel_one();
		OnDisconnect();
		//std::cout << "[Exception][" << __FUNCTION__ << "]:" << e.what() << std::endl;
	}
	

}

void Connection::Send(boost::asio::streambuf& buffer)
{
	setTimeout(5);
	try {
		boost::asio::async_write(m_ssl_socket, buffer, boost::asio::bind_executor(m_strand, boost::bind(&Connection::HandleSend, this, _1,_2)));
	}
	catch (std::exception e)
	{
		m_timeout_time.cancel_one();
		//std::cout << "[Exception][" << __FUNCTION__ << "]:" << e.what() << std::endl;
		OnDisconnect();
	}
}

void Connection::Recv(boost::asio::streambuf& buffer)
{
	setTimeout(5);
	try {
		boost::asio::async_read(m_ssl_socket, buffer,boost::asio::transfer_at_least(1), boost::asio::bind_executor(m_strand, boost::bind(&Connection::HandleRecv, this, _1 ,_2)));
	}
	catch (std::exception e)
	{
		m_timeout_time.cancel_one();
		//std::cout << "[Exception][" << __FUNCTION__ << "]:" << e.what() << std::endl;
		OnDisconnect();
	}
}

void Connection::Close() 
{ 
	//m_ssl_socket.lowest_layer().shutdown(boost::asio::socket_base::shutdown_type::shutdown_both); 
	//m_ssl_socket.shutdown(); 
}

void Connection::setTimeout(int seconds)
{
	m_timeout_time.expires_after(boost::asio::chrono::seconds(seconds));
	m_timeout_time.async_wait(
		boost::asio::bind_executor(
			m_strand,
			boost::bind(
				&Connection::HandleTimeout,
				this,
				_1)));
}

void Connection::HandleTimeout(boost::system::error_code ec)
{
	if (ec && ec != boost::asio::error::operation_aborted)
	{
		//std::cout << "THIS IS NOT POSSIBLE!" << std::endl;
		return;
	}
	if (ec && ec == boost::asio::error::operation_aborted)
	{
		return;
	}
	// Verify that the timer really expired since the deadline may have moved.
	if (m_timeout_time.expiry() <= boost::asio::chrono::steady_clock::now())
	{
		if (m_ssl_socket.lowest_layer().is_open())
		{
			m_ssl_socket.lowest_layer().shutdown(tcp::socket::shutdown_both, ec);
			m_ssl_socket.lowest_layer().close(ec);
		}
		OnTimeout(ec);
		return;
	}
}

void Connection::HandleConnect(boost::system::error_code error)
{
	if (error)
	{
		//std::cout << "[BoostException][" << __FUNCTION__ << "]:" << error.message() << std::endl;
		m_timeout_time.cancel_one();

		OnDisconnect();
	}
	else {
		if (m_ssl_socket.lowest_layer().is_open())
		{
			setTimeout(5);
			m_ssl_socket.async_handshake(boost::asio::ssl::stream_base::client, 
				boost::asio::bind_executor(m_strand, boost::bind(&Connection::HandleHandshake, this,_1))
			);
			OnConnect();
		}
		else {
			m_timeout_time.cancel_one();
			OnDisconnect();
		}
	}
}

void Connection::HandleSend(boost::system::error_code error,size_t size)
{
	if (!error)
	{
		OnSend(error,size);
	}
	else {
		//std::cout << "[BoostException][" << __FUNCTION__ << "]:" << error.message() << std::endl;
		m_timeout_time.cancel_one();

		OnDisconnect();

	}
}

void Connection::HandleRecv(boost::system::error_code error,size_t size)
{
	if (!error)
	{
		OnRecv(size);
	}
	else {
		//std::cout << "[BoostException][" << __FUNCTION__ << "]:" << error.message() << std::endl;
		m_timeout_time.cancel_one();

		OnDisconnect();

	}
}

void Connection::HandleHandshake(boost::system::error_code error)
{
	if (!error)
	{
		OnHandshake();
	}
	else {
		std::cout << "[BoostException][" << __FUNCTION__ << "]:" << error.message() << std::endl;
		m_timeout_time.cancel_one();

		OnDisconnect();
	}
}
