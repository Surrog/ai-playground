/* 
 * File:   Server.cpp
 * Author: Cedric Creusot <cedric.creusot@gmail.com>
 * 
 * Created on January 17, 2012, 4:54 PM
 */

#include "Server.hpp"
#include "Callback_Environnement.hpp"
#include <boost/chrono.hpp>
#include <iostream>

Server::Server() :
	socket_(16000),
	server_(new Poco::Net::TCPServerConnectionFactoryImpl<Connection>(),
	socket_),
	connections_(),
	environnement_(),
	connections_mut_() {
		forge_[Logique::Environnement_Event::ENTITY_SPAWN] = boost::bind(&Server::spawnForge, this, _1, _2);
		forge_[Logique::Environnement_Event::ENTITY_MOVE] = boost::bind(&Server::moveForge, this, _1, _2);
		forge_[Logique::Environnement_Event::ENTITY_EAT] = boost::bind(&Server::eatForge, this, _1, _2);
		forge_[Logique::Environnement_Event::ENTITY_DEATH] = boost::bind(&Server::dieForge, this, _1, _2);
		forge_[Logique::Environnement_Event::ENTITY_REPRODUCE] = boost::bind(&Server::reproduceForge, this, _1, _2);
		forge_[Logique::Environnement_Event::GRASS_UP] = boost::bind(&Server::grassUpForge, this, _1, _2);
		forge_[Logique::Environnement_Event::GRASS_DOWN] = boost::bind(&Server::grassDownForge, this, _1, _2);
}

Server::~Server() {
}


void Server::Start()
{
    std::cout << "server start" << std::endl;
    server_.start();
	boost::thread thr(boost::bind(&Server::run, this));
    environnement_.run();
}

void Server::AddConnection(Connection* connection)
{
	boost::lock_guard<boost::mutex> lock(connections_mut_);
    connections_[reinterpret_cast<ptrdiff_t>(connection)] = connection; 
}

void Server::RemoveConnection(Connection* connection)
{
	boost::lock_guard<boost::mutex> lock(connections_mut_);
    if (connections_.find(reinterpret_cast<ptrdiff_t>(connection)) != connections_.end())
        connections_.erase(reinterpret_cast<ptrdiff_t>(connection));
}

void Server::SendPacket(Packet& packet)
{
	boost::lock_guard<boost::mutex> lock(connections_mut_);
    std::map<ptrdiff_t, Connection*>::iterator it = connections_.begin();
    std::map<ptrdiff_t, Connection*>::iterator ite= connections_.end();
    
    for (; it != ite; ++it)
        it->second->AddPacket(packet);
}

void Server::synchronize(Connection * connection)
{
	std::cout << "Synchronize..." << std::endl;
	Packet packet;
	Logique::Board board;

	boost::lock_guard<boost::mutex> lock(connections_mut_);
	connection->Lock();
	environnement_.getBoard(board);
	board.lock();

	packet << Logique::Environnement_Event::NONE << Logique::BOARD_SIZE;
	connection->AddSynchroPacket(packet);
	for (uint32_t x = 0; x < Logique::BOARD_SIZE; x++)
	{
		for (uint32_t y = 0; y < Logique::BOARD_SIZE; y++)
		{
			Packet grass;
			Packet entity;
			Logique::Square sq = board.get(x, y);
			if (sq.hasEntity())
			{
				Logique::Entity * e = NULL;
				if (sq.hasSheep())
					e = reinterpret_cast<Logique::Entity*>(sq.getId(Logique::Square::SHEEP));
				if (sq.hasWolf())
					e = reinterpret_cast<Logique::Entity*>(sq.getId(Logique::Square::WOLF));
				if (e != NULL)
				{
					std::cout << "SPAWN" << std::endl;
					entity << static_cast<uint8_t>(Logique::Environnement_Event::ENTITY_SPAWN) << reinterpret_cast<ptrdiff_t>(e);
					entity << e->getType() << static_cast<int32_t>(x) << static_cast<int32_t>(y);
					connection->AddSynchroPacket(entity);
				}
			}
			if (sq.hasGrass())
			{
				grass << static_cast<uint8_t>(Logique::Environnement_Event::GRASS_UP) << static_cast<int32_t>(x) << static_cast<int32_t>(y);
				connection->AddSynchroPacket(grass);
			}
		}
	}
	board.unlock();
	connection->Unlock();
	std::cout << "...end" << std::endl;
}

Packet * Server::forgePacket(Logique::Environnement_Event & e)
{
	Packet * packet = new Packet();

	if (e._type > Logique::Environnement_Event::NONE && e._type < Logique::Environnement_Event::TYPE_SIZE)
		forge_[e._type](*packet, e);
	return packet;
}

void Server::spawnForge(Packet & output, Logique::Environnement_Event &e)
{
	output << static_cast<uint8_t>(Logique::Environnement_Event::ENTITY_SPAWN) << e._entityId << e._entityType << e._pos.x << e._pos.y;
}

void Server::moveForge(Packet & output, Logique::Environnement_Event &e)
{
	output << static_cast<uint8_t>(Logique::Environnement_Event::ENTITY_MOVE) << e._entityId << e._newPos.x << e._newPos.y;
}

void Server::eatForge(Packet & output, Logique::Environnement_Event &e)
{
	output << static_cast<uint8_t>(Logique::Environnement_Event::ENTITY_EAT) << e._entityId;
}

void Server::dieForge(Packet & output, Logique::Environnement_Event &e)
{
	output << static_cast<uint8_t>(Logique::Environnement_Event::ENTITY_DEATH) << e._entityId;
}

void Server::reproduceForge(Packet & output, Logique::Environnement_Event &e)
{
	output << static_cast<uint8_t>(Logique::Environnement_Event::ENTITY_REPRODUCE) << e._entityId;
}

void Server::grassUpForge(Packet & output, Logique::Environnement_Event &e)
{
	output << static_cast<uint8_t>(Logique::Environnement_Event::GRASS_UP) << e._pos.x << e._pos.y;
}

void Server::grassDownForge(Packet & output, Logique::Environnement_Event &e)
{
	output << static_cast<uint8_t>(Logique::Environnement_Event::GRASS_DOWN) << e._pos.x << e._pos.y;
}

void Server::run()
{
	boost::xtime xt;
	boost::xtime_get(&xt, boost::TIME_UTC);
	xt.sec += 5;
	while (true)
	{
		boost::thread::sleep(xt);
		Logique::Callback_Environnement::EventProxy proxy = Logique::Callback_Environnement::getInstance().getEventProxy();
		//Logique::Callback_Environnement::MetricProxy mProxy = Logique::Callback_Environnement::getInstance().getMetricProxy();
		Logique::Callback_Environnement::EventDeque::iterator it = proxy.begin();
		Logique::Callback_Environnement::EventDeque::iterator ite = proxy.end();
		for (;it != ite && connections_.size() > 0; ++it)
		{
			Packet * packet = forgePacket((*it));
			SendPacket(*packet);
			delete packet;
		}
		proxy.clear();
		//mProxy.clear();
		xt.sec += 2;
	}
}