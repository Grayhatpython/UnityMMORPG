#pragma once

#include <pdh.h>
#pragma comment(lib, "pdh.lib")

class Monitoring
{
public:
	~Monitoring();

public:
	bool Initialize();
	void Update();

	//	TEMP
	std::weak_ptr<class PacketSession> _owner;

private:
	PDH_HQUERY				_hQuery;
	PDH_HCOUNTER			_hCounter;
	PDH_FMT_COUNTERVALUE	_counterValue;


};

