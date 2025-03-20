#include "pch.h"
#include "Monitoring.h"
#include "ClientPacketHandler.h"

Monitoring::~Monitoring()
{
    if (_hCounter) 
        ::PdhRemoveCounter(_hCounter);
    if (_hQuery) 
        ::PdhCloseQuery(_hQuery);
}

bool Monitoring::Initialize()
{
    PDH_STATUS status = ::PdhOpenQuery(NULL, 0, &_hQuery);
    if (status != ERROR_SUCCESS) return false;

    // Add counter for overall CPU usage (for all processors)
    status = ::PdhAddEnglishCounter(_hQuery,
        L"\\Processor(_Total)\\% Processor Time",
        0,
        &_hCounter);

    if (status != ERROR_SUCCESS) {
        ::PdhCloseQuery(_hQuery);
        _hQuery = NULL;
        return false;
    }

    // Collect initial data
    ::PdhCollectQueryData(_hQuery);
    return true;
}

void Monitoring::Update()
{
    float cpuUsage = 0.0f;

    PDH_STATUS status = ::PdhCollectQueryData(_hQuery);

    status = ::PdhGetFormattedCounterValue(_hCounter,
        PDH_FMT_DOUBLE,
        NULL,
        &_counterValue);

    if (status != ERROR_SUCCESS)
        cpuUsage = 0.0f;

    cpuUsage = static_cast<float>(_counterValue.doubleValue);

    Protocol::S_CPUINFO cpuInfoPacket;
    auto cpuInfo = cpuInfoPacket.mutable_cpuinfo();
    cpuInfo->set_servertype(Protocol::SERVER_TYPE_GAME_SERVER);
    cpuInfo->set_cpuusage(cpuUsage);

    int64 timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    cpuInfo->set_timestamp(timestamp);
    cpuInfo->set_servername(u8"GameServer");

    auto  sendBuffer = ClientPacketHandler::MakeSendBuffer(cpuInfoPacket);
    _owner.lock()->Send(sendBuffer);

    std::cout << "���� ID: " << Protocol::SERVER_TYPE_GAME_SERVER << ", CPU ��뷮: " << cpuUsage
        << "%, Ÿ�ӽ�����: " << timestamp << std::endl;

    // 1�ʸ��� ����
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

