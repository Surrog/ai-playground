
#include "Callback_Environnement.hpp"
#include "Environnement.hpp"

namespace Logique {

	Callback_Environnement::Callback_Environnement() 
	{
		initEventTypeString();
		initEntityTypeString();
	}

	Callback_Environnement::~Callback_Environnement() {}

	Callback_Environnement::Callback_Environnement(const Callback_Environnement&) {}

	void Callback_Environnement::addAction(Environnement_Event::Type value, Entity& id, Square::EntityContain type, Coord pos, Coord newPos)
	{
		#ifdef LOGONOUT
		//debugEvent(value, type, pos, newPos);
		#else
		_mut.lock();
		_eventQueue.push_back(Environnement_Event(value, id, type, pos, newPos));
		_mut.unlock();
		#endif
	}

	void Callback_Environnement::addAction(Environnement_Event::Type value, Entity& id, Square::EntityContain type, Coord pos)
	{
		#ifdef LOGONOUT
		//debugEvent(value, type, pos);
		#else

		_mut.lock();
		_eventQueue.push_back(Environnement_Event(value, id, type, pos));
		_mut.unlock();

		#endif
	}
	
	void Callback_Environnement::addAction(Environnement_Event::Type value, Coord pos)
	{
		#ifdef LOGONOUT
		//debugEvent(value, pos);
		#else
		_mut.lock();
		_eventQueue.push_back(Environnement_Event(value, pos));
		_mut.unlock();
		#endif
	}

	void Callback_Environnement::addMetric(const Metric& value)
	{
		#ifdef LOGONOUT
		debugMetric(value);
		#else
		_metricMut.lock();
		_metricQueue.push_back(value);
		_metricMut.unlock();
		#endif
	}

	DequeProxy<Callback_Environnement::EventDeque> Callback_Environnement::getEventProxy() {
		return DequeProxy<EventDeque>(_eventQueue, _mut);
	}

	DequeProxy<Callback_Environnement::MetricDeque> Callback_Environnement::getMetricProxy()
	{
		return DequeProxy<MetricDeque>(_metricQueue, _metricMut);
	}

	void Callback_Environnement::debugEvent(const Environnement_Event& ev) 
	{
		if (ev._entityId > 0) 
		{
			std::cout << _eventTypeString[ev._type] << " id:" << ev._entityId << " " << _entityTypeString[ev._entityType] << " " << ev._pos;
			if (ev._pos != ev._newPos)
				std::cout << " " << ev._newPos << std::endl;
			else 
				std::cout << std::endl;
		} else {
			std::cout << _eventTypeString[ev._type] << " " << ev._pos << std::endl;
		}
	}
	
	void Callback_Environnement::debugMetric(const Metric& ev)
	{
		std::cout << "sheepNum:" << ev.sheepNum << " wolfNum:" << ev.wolfNum << std::endl;
		std::cout << " sheepMoy:" << ev.sheepMoy << " wolfMoy:" << ev.wolfMoy << std::endl;
		std::cout << " sheepActionNum:" << ev.sheepActionNum << " sheepActionNeural:" << ev.sheepActionNeural << std::endl;
		std::cout << " wolfActionNum:" << ev.wolfActionNum << " wolfActionNeural:" << ev.wolfActionNeural << std::endl;
	}

	void Callback_Environnement::debugEvent(Environnement_Event::Type value, Square::EntityContain type, Coord pos)
	{
		std::cout << _eventTypeString[value] << " " << _entityTypeString[type] << " " << pos << std::endl;
	}

	void Callback_Environnement::debugEvent(Environnement_Event::Type value, Square::EntityContain type, Coord pos, Coord newPos)
	{
		std::cout << _eventTypeString[value] << " " << _entityTypeString[type] << " " << pos << " " << newPos << std::endl;
	}

	void Callback_Environnement::debugEvent(Environnement_Event::Type value, Coord pos)
	{
		std::cout << _eventTypeString[value] << " " << pos << std::endl;
	}

	void Callback_Environnement::initEventTypeString() 
	{
		_eventTypeString[Environnement_Event::NONE] = "NONE";
		_eventTypeString[Environnement_Event::ENTITY_SPAWN] = "ENTITY_SPAWN";
		_eventTypeString[Environnement_Event::ENTITY_MOVE] = "ENTITY_MOVE";
		_eventTypeString[Environnement_Event::ENTITY_REPRODUCE] = "ENTITY_REPRODUCE";
		_eventTypeString[Environnement_Event::ENTITY_EAT] = "ENTITY_EAT";
		_eventTypeString[Environnement_Event::ENTITY_DEATH] = "ENTITY_DEATH";
		_eventTypeString[Environnement_Event::GRASS_UP] = "GRASS_UP";
		_eventTypeString[Environnement_Event::GRASS_DOWN] = "GRASS_DOWN";
	}

	void Callback_Environnement::initEntityTypeString() 
	{
		_entityTypeString[Square::SHEEP] = "SHEEP";
		_entityTypeString[Square::WOLF] = "WOLF";
	}

	std::size_t Callback_Environnement::getEventSize()
	{
		_mut.lock();
		std::size_t val = _eventQueue.size();
		_mut.unlock();
		return val;
	}

	std::size_t Callback_Environnement::getMetricSize()
	{
		_metricMut.lock();
		std::size_t val = _metricQueue.size();
		_metricMut.unlock();
		return val;
	}

	Environnement_Event Callback_Environnement::popEventFromFront()
	{
		_mut.lock();
		Environnement_Event val = _eventQueue.front();
		_mut.unlock();
		return val;
	}

	Metric Callback_Environnement::popMetricFromFront()
	{
		_metricMut.lock();
		Metric val = _metricQueue.front();
		_metricMut.unlock();
		return val;
	}
}
