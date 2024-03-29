
#ifndef ENVIRONNEMENT_HPP
#define ENVIRONNEMENT_HPP

#include <list>
#include <stack>
#include <set>
#include <map>

#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#include <boost/random.hpp>
#include <boost/array.hpp>
#include <boost/random/random_device.hpp>
#include <boost/chrono.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/unordered_map.hpp>

#include "Callback_Environnement.hpp"
#include "Square.hpp"
#include "Action.hpp"
#include "Entity.hpp"
#include "Board.hpp"
#include "Sheep.hpp"
#include "Wolf.hpp"
#include "EnvironnementGenetic.hpp"

namespace Logique {

	class Environnement : public Callback_Environnement {
	public:
		enum {
			ODOURONDEATH = 3,
			BASE_TIME_MILISEC = 300,
			STARTING_ZONE_MIN = 0,
			STARTING_ZONE_MAX = 15,
			LOG_TIME = 30,
			BOARD_TIME = 5
		};

		typedef std::deque< Action, boost::pool_allocator<Action> > ActionList;
		typedef std::pair< Entity*, Entity::Ptr > EntityPair;
		typedef std::map< Entity*, Entity::Ptr, std::less< Entity* >, boost::pool_allocator< EntityPair > > EntityPtrSet;
		typedef Square& (Board::*board_func)(const Coord&);

		Environnement();
		Environnement(const EnvironnementGenetic& adn);
		~Environnement();

		Environnement& operator=(const Environnement& orig);

		void run();
		void stop();
		void dump(std::ostream& stream);
		void test(int x, int y) {
			popOdour(Coord(x, y), 3);
			_board.dumpOdour();
		}

		void setBaseTime(const boost::chrono::duration<double>& time);
		void getBoard(Board& out_board);
		const EnvironnementGenetic& getAdn() const;
		void destroyEntity(Entity* value);
		void destroySheep(Sheep* value);
		void destroyWolf(Wolf* value);
		boost::thread* innerThread;

	private:
		//spawn Entity function
		bool addSheep(const Coord& loc);
		bool popSheepNear(const Coord& loc);
		void addSheep(unsigned int num);
		
		bool addWolf(const Coord& loc);
		bool popWolfNear(const Coord& loc);
		void addWolf(unsigned int num);

		unsigned int getEntityNum(const Square::EntityContain& value) const;
		unsigned int getSheepNum() const;
		unsigned int getWolfNum() const;

		bool validPerf(const float& value, Square::EntityContain type);

		void addAction(const Action& value);

		void unsafeInsertAction(const Action& value);
		void insertActionStack();
		void preRun();
		Action createBoardPlay(unsigned int start_tick);
		Action createLog(unsigned int start_tick);
		void boardPlay(unsigned int tick_start);
		void doLog(unsigned int tick_start);
		void initEntity(boost::shared_ptr<Entity> value, const Coord& loc);
		void spawnSheep();
		void onEntityDeath(Entity& value);
		void popOdour(const Coord& loc, unsigned int power = ODOURONDEATH);
		void addOdour(int x, int y, unsigned int value);

		void debugActionList();

		boost::array<unsigned int, Square::ENTITY_CONTAINER_SIZE> _entityNum;

		boost::chrono::duration<double> _baseTime;
		
		
		Board _board;

		boost::random::random_device _randomD;
		boost::random::mt19937 _gen;
		boost::random::uniform_int_distribution<unsigned int> _distri;
		
		EnvironnementGenetic _adn;
		bool _genetic;

		ActionList _actionList;
		EntityPtrSet _entityList;
		boost::object_pool<Sheep> _sheepPool;
		boost::object_pool<Wolf> _wolfPool;

		DecisionTree _sheepTree;
		DecisionTree _wolfTree;
		bool _run;
	};

}


#endif /* !ENVIRONNEMENT_HPP */

