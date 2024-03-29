
#ifndef ALGOGEN
#define ALGOGEN

#include <deque>
#include <utility>
#include <boost/pool/pool_alloc.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

#include "Environnement.hpp"

class AlgoGen
{
public:
	enum {
		ENV_SIZE = 15,
		TIMEBEFOREMERGE = 80
	};

	typedef Logique::Environnement Environnement;
	typedef boost::shared_ptr<Environnement> EnvPtr;
	typedef std::deque< EnvPtr > EnvironnementArray;
	typedef boost::array< double, ENV_SIZE > DoubleArray;
	typedef void (AlgoGen::*func)(const Metric&, double& , double& , double&, double&, double&);

	AlgoGen();
	AlgoGen(const std::string& file);
	~AlgoGen();

	void run();
	void stop();
	void gatherSheep(const Metric& from, double& perfTotal, double& perfNum, double& actienTotal, double& actionNum, double& entityTotal);
	void gatherWolf(const Metric& from, double& perfTotal, double& perfNum, double& actienTotal, double& actionNum, double& entityTotal);
private:
	void initRandomEnv();
	double evaluateEnv(Environnement& env);
	double evaluateEntity(Environnement& env, func gatherFunction);
	double evaluateWolf(Environnement& env);
	EnvPtr make_environnement();
	EnvPtr make_environnement(const EnvironnementGenetic& genetic);
	void destroy_environnement(Environnement* env);

	void dump();
	bool initFromFile();

	EnvironnementArray _envList;
	DoubleArray _perfList;
	boost::thread_group _thread_pool;
	boost::object_pool<Environnement> _envPool;
	bool _run;
	boost::mutex _mut;
	std::string _file;
};

#endif /* !ALGOGEN */