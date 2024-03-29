
#include <iostream>

#include <boost/foreach.hpp>
#include <boost/smart_ptr/make_shared.hpp>

#include "Callback_Environnement.hpp"
#include "Environnement.hpp"
#include "Logger.hpp"
#include "Sheep.hpp"
#include "Wolf.hpp"

namespace Logique {

	Environnement::Environnement()
		: Callback_Environnement()
		, _entityNum(), _baseTime(boost::chrono::milliseconds(BASE_TIME_MILISEC))
		, _entityList(), _actionList(), _board(), _sheepPool(), _wolfPool()
		, _randomD(), _gen(_randomD), _distri(STARTING_ZONE_MIN, STARTING_ZONE_MAX)
		, _adn(), _genetic(false), _sheepTree(), _wolfTree(), _run(true)
	{
		_entityNum[Square::SHEEP] = 0;
		_entityNum[Square::WOLF] = 0;
		addAction(createBoardPlay(0));
		addAction(createLog(0));
	}

	Environnement::Environnement(const EnvironnementGenetic& adn)
		: Callback_Environnement()
		, _entityNum(), _board(), _baseTime(boost::chrono::milliseconds(BASE_TIME_MILISEC))
		, _actionList(), _entityList(), _sheepPool(), _wolfPool()
		, _randomD(), _gen(_randomD), _distri(STARTING_ZONE_MIN, STARTING_ZONE_MAX)
		, _adn(adn), _genetic(true), _sheepTree(_adn._sheepDecisionTree), _wolfTree(_adn._wolfDecisionTree), _run(true)
	{
		_entityNum[Square::SHEEP] = 0;
		_entityNum[Square::WOLF] = 0;
		addAction(createBoardPlay(0));
		addAction(createLog(0));
	}

	Environnement::~Environnement() 
	{
		_actionList.clear();

		_board.lock();
		BOOST_FOREACH(const EntityPair& ptr, _entityList)
		{
			_board(ptr.first->getLocation()).hasEntity(ptr.first->getType(), 0);
			_entityNum[ptr.first->getType()]--;
			ptr.first->cleanVtable();
		}
		_board.unlock();

		_entityList.clear();
	}

	Environnement& Environnement::operator=(const Environnement& orig)
	{ return *this; }

	const EnvironnementGenetic& Environnement::getAdn() const
	{
		return _adn;
	}

	void Environnement::preRun() {
		addSheep(20);
		addWolf(10);

		if (_actionList.empty()) {
			std::cout << "no action" << std::endl;
		}
	}

	void Environnement::run() {
		preRun();

		boost::chrono::system_clock::time_point start;
		boost::chrono::duration<double> total_time;

		while (!_actionList.empty() && _run) 
		{
			start = boost::chrono::system_clock::now();

			unsigned int tick_passed = std::floor(total_time.count() / _baseTime.count());

			if (tick_passed == 0) 
			{
				boost::this_thread::sleep(
					boost::posix_time::milliseconds(_actionList.begin()->tickBefore() * _baseTime.count() * 1000)
					);
			} 
			else 
			{
				total_time -= (tick_passed * _baseTime);

				BOOST_FOREACH(Action& act, _actionList)
				{ act._tickStart += tick_passed; }

				// on fait les nouvelle action qui reste a faire
				while (!_actionList.empty() && _run && _actionList.begin()->execute()) 
				{ _actionList.pop_front(); }

				if (getSheepNum() <= 3 && _run) 
				{ addSheep(20); }

				if (getWolfNum() <= 3 && _run) 
				{ addWolf(10); }
			}

			total_time += boost::chrono::system_clock::now() - start;
		}
	}

	void Environnement::stop() 
	{ _run = false; }

	void Environnement::dump(std::ostream& stream)
	{
		stream << _adn;
	}

	void Environnement::addAction(const Action& value) 
	{ unsafeInsertAction(value); }

	void Environnement::unsafeInsertAction(const Action& value) 
	{
		ActionList::iterator it = _actionList.begin();
		ActionList::iterator ite = _actionList.end();

		while (it != ite && it->tickBefore() < value.tickBefore()) {
			++it;
		}
		if (it != ite) 
		{ _actionList.insert(it, value); }
		else 
		{ _actionList.push_back(value); }
	}

	void Environnement::insertActionStack() {
		/*while (!_actionTmpStack.empty()) {
			unsafeInsertAction(_actionTmpStack.top());
			_actionTmpStack.pop();
		}*/
	}

	void Environnement::setBaseTime(const boost::chrono::duration<double>& time) 
	{
		_baseTime = time;
	}

	//spawn Entity function
	bool Environnement::addSheep(const Coord& loc) 
	{
		if (!_board(loc).hasSheep()) {
			//boost::shared_ptr<Sheep> sheepPtr(new Sheep(_sheepTree));
			boost::shared_ptr<Sheep> sheepPtr(_sheepPool.construct(boost::ref(_sheepTree)), boost::bind(&Environnement::destroySheep, this, _1));
			sheepPtr->addFood(Logique::FOOD_START);
			sheepPtr->initActionArray(_board, *this);

			sheepPtr->setGetNumberSpecies(boost::bind(&Environnement::getSheepNum, this));
			sheepPtr->setPopEntityFunctor(boost::bind(&Environnement::popSheepNear, this, _1));
			initEntity(sheepPtr, loc);
			addEvent(Environnement_Event::ENTITY_SPAWN, *sheepPtr.get(), Square::SHEEP, loc);
			sheepPtr->getNewAction(0);
			return true;
		}
		return false;
	}

	bool Environnement::addWolf(const Coord& loc) 
	{
		if (!_board(loc).hasWolf()) {
			//boost::shared_ptr<Wolf> wolfPtr(new Wolf(_wolfTree));
			boost::shared_ptr<Wolf> wolfPtr(_wolfPool.construct(boost::ref(_wolfTree)), boost::bind(&Environnement::destroyWolf, this, _1));

			wolfPtr->addFood(Logique::FOOD_START);
			wolfPtr->initActionArray(_board, *this);

			wolfPtr->setGetNumberSpecies(boost::bind(&Environnement::getWolfNum, this));
			wolfPtr->setPopEntityFunctor(boost::bind(&Environnement::popWolfNear, this, _1));
			initEntity(wolfPtr, loc);
			addEvent(Environnement_Event::ENTITY_SPAWN, *wolfPtr.get(), Square::WOLF, loc);
			wolfPtr->getNewAction(0);
			return true;
		}
		return false;
	}

	void Environnement::addSheep(unsigned int num) 
	{
		Coord loc;
		unsigned int i = 0;

		while (i < num) {
			unsigned int limit = 0;

			do {
				loc = Coord(_distri(_gen), _distri(_gen));
			} while (_board(loc).hasSheep() && limit < 10);

			if (limit < 10)
				addSheep(loc);
			++i;
		}
	}

	void Environnement::addWolf(unsigned int num) 
	{
		Coord loc;
		unsigned int i = 0;

		while (i < num) {
			unsigned int limit = 0;

			do {
				loc = Coord(_distri(_gen), _distri(_gen));
			} while (_board(loc).hasWolf() && limit < 10);

			if (limit < 10)
				addWolf(loc);
			++i;
		}
	}

	bool Environnement::popSheepNear(const Coord& loc) 
	{
		bool found = false;
		Coord locFound;

		for (int x = loc.x - 1; x <= loc.x + 1 && !found; ++x) {
			for (int y = loc.y - 1; y <= loc.y + 1 && !found; ++y) {
				found = !_board.get(x, y).hasSheep();
				if (found) {
					locFound = _board.getValidValue(x, y);
				}
			}
		}

		return found && addSheep(locFound);
	}

	bool Environnement::popWolfNear(const Coord& loc) 
	{
		bool found = false;
		Coord locFound;

		for (int x = loc.x - 1; x <= loc.x + 1 && !found; ++x) {
			for (int y = loc.x - 1; y <= loc.y + 1 && !found; ++y) {
				found = !_board.get(x, y).hasWolf();
				if (found) {
					locFound = _board.getValidValue(x, y);
				}
			}
		}

		return found && addWolf(locFound);
	}

	unsigned int Environnement::getEntityNum(const Square::EntityContain& value) const
	{
		return _entityNum[value];
	}

	unsigned int Environnement::getSheepNum() const 
	{
		return _entityNum[Square::SHEEP];
	}

	unsigned int Environnement::getWolfNum() const 
	{
		return _entityNum[Square::WOLF];
	}

	bool Environnement::validPerf(const float& value, Square::EntityContain type)
	{
		float moy = 0;
		float size = 0;
		EntityPtrSet::const_iterator it = _entityList.begin();
		EntityPtrSet::const_iterator ite = _entityList.end();

		while (it != ite) {
			if (it->first->getType() == type) {
				moy += it->first->getLastMoy();
				++size;
			}
			++it;
		}

		if (size)
			moy /= size;

		return  value && (value >= moy);
	}

	void Environnement::debugActionList() {
		ActionList::iterator it = _actionList.begin();
		ActionList::iterator ite = _actionList.end();

		std::cout << "## size:" << _actionList.size() << std::endl;
		while (it != ite) {
			std::cout << "tick start:" << it->_tickStart << " tickEnd:" << it->_tickBeforeAction << " diff:" << it->tickBefore() << std::endl;
			++it;
		}
		std::cout << "##" << std::endl;
	}

	Action Environnement::createBoardPlay(unsigned int start_tick) 
	{
		Action act;

		act._action = boost::bind(&Environnement::boardPlay, this, _1);
		act._tickStart = start_tick;
		act._tickBeforeAction = BOARD_TIME;
		return act;
	}

	Action Environnement::createLog(unsigned int start_tick)
	{
		Action act;

		act._action = boost::bind(&Environnement::doLog, this, _1);
		act._tickStart = start_tick;
		act._tickBeforeAction = LOG_TIME;
		return act;
	}

	void Environnement::doLog(unsigned int tick_start)
	{
		float totalSheep = 0;
		float totalWolf = 0;

		EntityPtrSet::const_iterator it = _entityList.begin();
		EntityPtrSet::const_iterator ite = _entityList.end();

		while (it != ite)
		{
			if (it->first->getType() == Square::SHEEP)
				totalSheep += it->first->getLastMoy();
			else
				totalWolf += it->first->getLastMoy();

			++it;
		}

		if (_entityNum[Square::SHEEP] > 0)
			totalSheep /= static_cast<float>(_entityNum[Square::SHEEP]);
		else
			totalSheep = 0;

		if (_entityNum[Square::WOLF] > 0)
			totalWolf /= static_cast<float>(_entityNum[Square::WOLF]);
		else
			totalWolf = 0;

		addMetric(
			Metric(_entityNum[Square::SHEEP], _entityNum[Square::WOLF],
			totalSheep, totalWolf,
			_sheepTree.getActionNum(), _sheepTree.getActionNeural(),
			_wolfTree.getActionNum(), _wolfTree.getActionNeural())
			);
		_sheepTree.clear();
		_wolfTree.clear();
		addAction(createLog(tick_start));
	}

	void Environnement::boardPlay(unsigned int tick_start) {
		_board.lock();
		for (unsigned int x = 0; x < BOARD_SIZE; ++x) {
			for (unsigned int y = 0; y < BOARD_SIZE; ++y) {
				
				if (!_board(x, y).hasEntity() && _board(x, y).increaseGrass())
					addEvent(Environnement_Event::GRASS_UP, Coord(x, y));
				if (_board(x, y).hasEntity() && _board(x, y).decreaseGrass())
					addEvent(Environnement_Event::GRASS_DOWN, Coord(x, y));
				_board(x, y).decreaseOdour(1);
				
			}
		}
		_board.unlock();

		addAction(createBoardPlay(tick_start));
	}

	void Environnement::initEntity(boost::shared_ptr<Entity> value, const Coord& loc) 
	{
		_board.lock();
		_board(loc).hasEntity(value->getType(), value.get());
		_board.unlock();
		_entityList[value.get()] = value;
		_entityNum[value->getType()]++;
		value->setLocation(loc);
		value->setValidScore(boost::bind(&Environnement::validPerf, this, _1, value->getType()));
		value->setAddAction(boost::bind(&Environnement::addAction, this, _1));
		value->setOnDeath(boost::bind(&Environnement::onEntityDeath, this, _1));
		value->setGetSquare(boost::bind(static_cast<board_func>(&Board::get), &_board, _1));
		value->reInitPerf();
		addAction(value->createFoodAction());
	}

	void Environnement::spawnSheep() 
	{
		Coord loc;
		unsigned int limit = 0;

		do {
			loc = Coord(_distri(_gen), _distri(_gen));
		} while (_board(loc).hasSheep() && limit < 10);

		if (limit < 10)
			addSheep(loc);
	}

	void Environnement::onEntityDeath(Entity& value) 
	{
		addEvent(Environnement_Event::ENTITY_DEATH, value, value.getType(), value.getLocation());

		_board.lock();
		popOdour(value.getLocation());
		_board(value.getLocation()).hasEntity(value.getType(), 0);
		_board.unlock();
		
		_entityNum[value.getType()]--;		
		value.cleanVtable();
		EntityPtrSet::iterator it = _entityList.find(&value);
		if (it != _entityList.end()) {
			_entityList.erase(it);
		}
	}

	void Environnement::popOdour(const Coord& loc, unsigned int power) 
	{
		for (unsigned int size = power; size > 0; size--) {
			int x_start = loc.x - size + 1;
			int y_start;
			int x_end = loc.x + size;
			int y_end = loc.y + size;

			while (x_start < x_end) {
				y_start = loc.y - size + 1;

				while (y_start < y_end) {
					addOdour(x_start, y_start, 1);
					y_start++;
				}

				x_start++;
			}
		}
	}

	void Environnement::addOdour(int x, int y, unsigned int value) 
	{ _board.get(x, y).addOdour(value); }

	void Environnement::destroyEntity(Entity* value)
	{
		value->destroyMe(*this);
	}

	void Environnement::destroySheep(Sheep* value) 
	{ 
		
		_sheepPool.destroy(value); 
	
	}

	void Environnement::destroyWolf(Wolf* value) 
	{ _wolfPool.destroy(value); }

	void Environnement::getBoard(Board& out_Board)
	{
		_board.lock();
		out_Board = _board;
		_board.unlock();
	}
}
