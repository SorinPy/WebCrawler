#include "Connection.h"

Connection::Connection(boost::shared_ptr<boost::asio::io_context> io_context, boost::shared_ptr<boost::asio::ssl::context> ssl_context)
	:m_io_context(io_context) ,m_socket(*io_context),m_ssl_socket(*io_context,*ssl_context)
{
}

void Connection::Connect(const std::string& host, const std::string &port)
{
	try 
	{
		boost::system::error_code ec;
		tcp::resolver resolver(*m_io_context);
		auto endpoint = resolver.resolve(host, port,ec);
		if (!ec)
		{
			m_ssl_socket.set_verify_mode(boost::asio::ssl::verify_none);
			/*m_ssl_socket.set_verify_callback([](bool preverified,
				boost::asio::ssl::verify_context& ctx) { 
				
				return true; 
			});*/

			boost::asio::async_connect(m_ssl_socket.lowest_layer(), endpoint, boost::bind(&Connection::HandleConnect, this, _1));
		}
		else {
			OnDisconnect();
			std::cout << "[BoostException][" << __FUNCTION__ << "]:" << ec.message() << std::endl;
		}
	}
	catch (std::exception e)
	{
		OnDisconnect();
		std::cout << "[Exception][" << __FUNCTION__ << "]:" << e.what() << std::endl;
	}
	

}

void Connection::Send(boost::asio::streambuf& buffer)
{
	try {
		boost::asio::async_write(m_ssl_socket, buffer, boost::bind(&Connection::HandleSend, this, _1,_2));
	}
	catch (std::exception e)
	{
		
		std::cout << "[Exception][" << __FUNCTION__ << "]:" << e.what() << std::endl;
		OnDisconnect();
	}
}

void Connection::Recv(boost::asio::streambuf& buffer)
{
	try {
		boost::asio::async_read(m_ssl_socket, buffer,boost::asio::transfer_at_least(1), boost::bind(&Connection::HandleRecv, this, _1 ,_2));
	}
	catch (std::exception e)
	{
		std::cout << "[Exception][" << __FUNCTION__ << "]:" << e.what() << std::endl;
		OnDisconnect();
	}
}

void Connection::Close() 
{ 
	m_ssl_socket.lowest_layer().close();
	//m_ssl_socket.lowest_layer().shutdown(boost::asio::socket_base::shutdown_type::shutdown_both); 
	//m_ssl_socket.shutdown(); 
}

void Connection::HandleConnect(boost::system::error_code error)
{
	if (error)
	{
		std::cout << "[BoostException][" << __FUNCTION__ << "]:" << error.message() << std::endl;
		if (error != boost::asio::error::operation_aborted)
		{
			OnDisconnect();
		}
	}
	else {
		if (m_ssl_socket.lowest_layer().is_open())
		{
			
			m_ssl_socket.async_handshake(boost::asio::ssl::stream_base::client,
				boost::bind(&Connection::HandleHandshake, this,
					_1));
			OnConnect();
		}
	}
}

void Connection::HandleSend(boost::system::error_code error,size_t size)
{
	if (!error)
	{
		OnSend();
	}
	else {
		std::cout << "[BoostException][" << __FUNCTION__ << "]:" << error.message() << std::endl;
		if (error != boost::asio::error::operation_aborted)
		{
			OnDisconnect();
		}
	}
}

void Connection::HandleRecv(boost::system::error_code error,size_t size)
{
	if (!error)
	{
		OnRecv(size);
	}
	else {
		if (error != boost::asio::error::operation_aborted)
		{
			OnDisconnect();
		}
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
		if (error != boost::asio::error::operation_aborted)
		{
			OnDisconnect();
		}
	}
}
