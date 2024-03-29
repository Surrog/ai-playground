/* 
 * File:   Connection.cpp
 * Author: Cedric Creusot <cedric.creusot@gmail.com>
 * 
 * Created on January 17, 2012, 11:53 AM
 */

#include "Connection.hpp"
#include "Server.hpp"
#include <iostream>
#include <string>


Connection::Connection(const Poco::Net::StreamSocket & socket) :
Poco::Net::TCPServerConnection(socket) {
    Server::getInstance().AddConnection(this);
}

Connection::~Connection() {
    std::cout << "Disconnection from : " << socket().peerAddress().toString() << std::endl;
	Server::getInstance().RemoveConnection(this);
}

void Connection::run() {
	std::cout << "Connection from : " << socket().peerAddress().toString() << std::endl;
	Server::getInstance().synchronize(this);
	char buffer[512];
	try
	{
		while (true)
		{
			if (socket().poll(Poco::Timespan(1000), Poco::Net::Socket::SELECT_READ))
			{
				if (socket().receiveBytes(buffer, 512, 0) <= 0)
					return ;
				// TODO : interpret this.
			}
			else
			{
				boost::lock_guard<boost::mutex> lock(packet_mut_);
				while (!packets_.empty())
				{
					Packet & pack = packets_.front();
					std::size_t size = pack.Endianl(pack.GetSize());
					socket().sendBytes(reinterpret_cast<const void*>(&size), sizeof(size));
					std::size_t send = 0;
					for (; send < pack.GetSize(); )
						send = socket().sendBytes(pack.GetData() + send, pack.GetSize() - send, 0);
					packets_.pop_front();
				}
			}
		}
	}
	catch (std::exception & e)
	{
		std::cerr << e.what() << std::endl;
	}
}

void Connection::AddPacket(Packet & packet)
{
    boost::lock_guard<boost::mutex> lock(packet_mut_);
    packets_.push_back(packet);
}

void Connection::AddSynchroPacket(Packet & packet)
{
	packets_.push_back(packet);
}

void Connection::Lock()
{
	packet_mut_.lock();
}

void Connection::Unlock()
{
	packet_mut_.unlock();
}
