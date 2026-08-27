#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace ns3;

namespace
{

struct SimulationConfig
{
    uint32_t nSta = 50;
    uint32_t cwMin = 15;
    uint32_t cwMax = 1023;
    double simulationTime = 10.0;
    double startTime = 1.0;
    std::string offeredRate = "1Mbps";
    uint32_t packetSize = 1200;
    uint32_t seed = 1;
    uint64_t run = 1;
    std::string csvOutput;
};

struct SimulationResult
{
    uint32_t nSta = 0;
    uint32_t cwMin = 0;
    uint32_t cwMax = 0;
    std::string offeredRate;
    uint32_t seed = 0;
    uint64_t run = 0;

    uint64_t txPackets = 0;
    uint64_t rxPackets = 0;
    uint64_t rxBytes = 0;

    double throughputMbps = 0.0;
    double packetLossPercent = 0.0;
    double averageDelayMs = 0.0;
};

// 入力されたシミュレーション条件を確認
void
ValidateConfig(const SimulationConfig& config)
{
    if (config.nSta == 0)
    {
        throw std::invalid_argument("nSta must be greater than 0");
    }
    // APとSTAにIPアドレスを割り当てられる範囲に制限
    if (config.nSta > 250)
    {
        throw std::invalid_argument("nSta must be 250 or less for the configured IPv4 subnet");
    }
    if (config.cwMin > config.cwMax)
    {
        throw std::invalid_argument("cwMin must be less than or equal to cwMax");
    }
    if (config.simulationTime <= 0.0)
    {
        throw std::invalid_argument("simulationTime must be greater than 0");
    }
    if (config.startTime < 0.0 || config.startTime >= config.simulationTime)
    {
        throw std::invalid_argument("startTime must satisfy 0 <= startTime < simulationTime");
    }
    if (config.packetSize == 0)
    {
        throw std::invalid_argument("packetSize must be greater than 0");
    }
    if (config.seed == 0)
    {
        throw std::invalid_argument("seed must be greater than 0");
    }
    if (config.run == 0)
    {
        throw std::invalid_argument("run must be greater than 0");
    }
}

// APとSTAの位置を設定
void
ConfigurePositions(NodeContainer apNode, NodeContainer staNodes)
{
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();

    positionAlloc->Add(Vector(0.0, 0.0, 0.0));

    // 電波減衰よりも端末間の競合を評価しやすいよう、STAをAPの近くに配置
    for (uint32_t i = 0; i < staNodes.GetN(); ++i)
    {
        const double x = 1.0 + (i % 10) * 0.3;
        const double y = 1.0 + (i / 10) * 0.3;
        positionAlloc->Add(Vector(x, y, 0.0));
    }

    mobility.SetPositionAllocator(positionAlloc);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    NodeContainer allNodes;
    allNodes.Add(apNode);
    allNodes.Add(staNodes);
    mobility.Install(allNodes);
}

// 各端末のCWを設定
void
SetContentionWindow(const NetDeviceContainer& devices, uint32_t cwMin, uint32_t cwMax)
{
    for (uint32_t i = 0; i < devices.GetN(); ++i)
    {
        Ptr<WifiNetDevice> device = DynamicCast<WifiNetDevice>(devices.Get(i));
        if (!device)
        {
            throw std::runtime_error("Failed to cast NetDevice to WifiNetDevice");
        }

        Ptr<WifiMac> wifiMac = device->GetMac();
        PointerValue ptr;
        wifiMac->GetAttribute("Txop", ptr);

        Ptr<Txop> txop = ptr.Get<Txop>();
        if (!txop)
        {
            throw std::runtime_error("Failed to obtain Txop from WifiMac");
        }

        txop->SetMinCw(cwMin);
        txop->SetMaxCw(cwMax);
    }
}

// 通信結果を集計して評価値を計算
SimulationResult
CollectResults(const SimulationConfig& config,
               const Ipv4Address& apAddress,
               FlowMonitorHelper& flowHelper,
               Ptr<FlowMonitor> monitor)
{
    monitor->CheckForLostPackets();

    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(flowHelper.GetClassifier());
    if (!classifier)
    {
        throw std::runtime_error("Failed to obtain IPv4 flow classifier");
    }

    SimulationResult result;
    result.nSta = config.nSta;
    result.cwMin = config.cwMin;
    result.cwMax = config.cwMax;
    result.offeredRate = config.offeredRate;
    result.seed = config.seed;
    result.run = config.run;

    double totalDelaySeconds = 0.0;

    for (const auto& flow : monitor->GetFlowStats())
    {
        const Ipv4FlowClassifier::FiveTuple tuple = classifier->FindFlow(flow.first);

        // STAからAPへのUDP通信のみを集計
        if (tuple.destinationAddress != apAddress || tuple.protocol != 17)
        {
            continue;
        }

        result.txPackets += flow.second.txPackets;
        result.rxPackets += flow.second.rxPackets;
        result.rxBytes += flow.second.rxBytes;
        totalDelaySeconds += flow.second.delaySum.GetSeconds();
    }

    const double activeTime = config.simulationTime - config.startTime;
    result.throughputMbps =
        (static_cast<double>(result.rxBytes) * 8.0) / activeTime / 1'000'000.0;

    if (result.txPackets > 0)
    {
        result.packetLossPercent =
            100.0 * static_cast<double>(result.txPackets - result.rxPackets) /
            static_cast<double>(result.txPackets);
    }

    if (result.rxPackets > 0)
    {
        result.averageDelayMs =
            (totalDelaySeconds / static_cast<double>(result.rxPackets)) * 1000.0;
    }

    return result;
}

// シミュレーション結果を表示
void
PrintResult(const SimulationResult& result)
{
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "\n===== SIMULATION RESULT =====\n";
    std::cout << "Stations            : " << result.nSta << "\n";
    std::cout << "CWmin               : " << result.cwMin << "\n";
    std::cout << "CWmax               : " << result.cwMax << "\n";
    std::cout << "Offered rate / STA  : " << result.offeredRate << "\n";
    std::cout << "Seed / Run          : " << result.seed << " / " << result.run << "\n";
    std::cout << "Tx packets          : " << result.txPackets << "\n";
    std::cout << "Rx packets          : " << result.rxPackets << "\n";
    std::cout << "Throughput          : " << result.throughputMbps << " Mbps\n";
    std::cout << "Packet loss         : " << result.packetLossPercent << " %\n";
    std::cout << "Average delay       : " << result.averageDelayMs << " ms\n";

    // 集計しやすい形式でも結果を出力
    std::cout << "RESULT_CSV," << result.nSta << ',' << result.cwMin << ',' << result.cwMax
              << ',' << result.offeredRate << ',' << result.seed << ',' << result.run << ','
              << result.txPackets << ',' << result.rxPackets << ',' << result.throughputMbps << ','
              << result.packetLossPercent << ',' << result.averageDelayMs << "\n";
}

// シミュレーション結果をCSVに保存
void
AppendCsv(const std::string& path, const SimulationResult& result)
{
    if (path.empty())
    {
        return;
    }

    bool writeHeader = false;
    {
        std::ifstream existing(path);
        writeHeader = !existing.good() || existing.peek() == std::ifstream::traits_type::eof();
    }

    std::ofstream output(path, std::ios::app);
    if (!output)
    {
        throw std::runtime_error("Failed to open CSV output: " + path);
    }

    output << std::fixed << std::setprecision(6);
    if (writeHeader)
    {
        output << "STA,CWmin,CWmax,OfferedRate,Seed,Run,TxPackets,RxPackets,"
                  "ThroughputMbps,PacketLossPercent,AverageDelayMs\n";
    }

    output << result.nSta << ',' << result.cwMin << ',' << result.cwMax << ','
           << result.offeredRate << ',' << result.seed << ',' << result.run << ','
           << result.txPackets << ',' << result.rxPackets << ',' << result.throughputMbps << ','
           << result.packetLossPercent << ',' << result.averageDelayMs << '\n';
}

// Wi-Fiシミュレーションを実行
SimulationResult
RunSimulation(const SimulationConfig& config)
{
    RngSeedManager::SetSeed(config.seed);
    RngSeedManager::SetRun(config.run);

    NodeContainer staNodes;
    staNodes.Create(config.nSta);

    NodeContainer apNode;
    apNode.Create(1);

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211a);

    // CWの影響を比較しやすくするため通信速度を固定
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode",
                                 StringValue("OfdmRate54Mbps"),
                                 "ControlMode",
                                 StringValue("OfdmRate24Mbps"));

    WifiMacHelper mac;
    const Ssid ssid("cw-experiment");

    mac.SetType("ns3::StaWifiMac",
                "Ssid",
                SsidValue(ssid),
                "ActiveProbing",
                BooleanValue(false));
    NetDeviceContainer staDevices = wifi.Install(phy, mac, staNodes);

    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    NetDeviceContainer apDevice = wifi.Install(phy, mac, apNode);

    ConfigurePositions(apNode, staNodes);

    InternetStackHelper stack;
    stack.Install(apNode);
    stack.Install(staNodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.0.0", "255.255.255.0");
    address.Assign(staDevices);
    Ipv4InterfaceContainer apInterface = address.Assign(apDevice);

    const uint16_t port = 5000;
    UdpServerHelper server(port);
    ApplicationContainer serverApp = server.Install(apNode.Get(0));
    serverApp.Start(Seconds(0.5));
    serverApp.Stop(Seconds(config.simulationTime));

    const DataRate offeredRate(config.offeredRate);
    if (offeredRate.GetBitRate() == 0)
    {
        throw std::invalid_argument("offeredRate must be greater than 0");
    }

    const double interval =
        (static_cast<double>(config.packetSize) * 8.0) / offeredRate.GetBitRate();

    for (uint32_t i = 0; i < config.nSta; ++i)
    {
        UdpClientHelper client(apInterface.GetAddress(0), port);
        client.SetAttribute("MaxPackets", UintegerValue(0xffffffff));
        client.SetAttribute("Interval", TimeValue(Seconds(interval)));
        client.SetAttribute("PacketSize", UintegerValue(config.packetSize));

        ApplicationContainer clientApp = client.Install(staNodes.Get(i));
        clientApp.Start(Seconds(config.startTime));
        clientApp.Stop(Seconds(config.simulationTime));
    }

    // STAとAPに同じCWを設定
    SetContentionWindow(staDevices, config.cwMin, config.cwMax);
    SetContentionWindow(apDevice, config.cwMin, config.cwMax);

    FlowMonitorHelper flowHelper;
    Ptr<FlowMonitor> monitor = flowHelper.InstallAll();

    Simulator::Stop(Seconds(config.simulationTime));
    Simulator::Run();

    SimulationResult result =
        CollectResults(config, apInterface.GetAddress(0), flowHelper, monitor);

    Simulator::Destroy();
    return result;
}

} // 無名名前空間

int
main(int argc, char* argv[])
{
    SimulationConfig config;

    CommandLine cmd(__FILE__);
    cmd.AddValue("nSta", "Number of Wi-Fi stations", config.nSta);
    cmd.AddValue("cwMin", "Minimum contention window", config.cwMin);
    cmd.AddValue("cwMax", "Maximum contention window", config.cwMax);
    cmd.AddValue("simulationTime", "Simulation time in seconds", config.simulationTime);
    cmd.AddValue("startTime", "UDP traffic start time in seconds", config.startTime);
    cmd.AddValue("offeredRate", "UDP offered rate per STA", config.offeredRate);
    cmd.AddValue("packetSize", "UDP packet size in bytes", config.packetSize);
    cmd.AddValue("seed", "ns-3 random seed", config.seed);
    cmd.AddValue("run", "ns-3 run number", config.run);
    cmd.AddValue("csvOutput", "Optional CSV file to append one result row", config.csvOutput);
    cmd.Parse(argc, argv);

    try
    {
        ValidateConfig(config);
        const SimulationResult result = RunSimulation(config);
        PrintResult(result);
        AppendCsv(config.csvOutput, result);
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << '\n';
        Simulator::Destroy();
        return 1;
    }

    return 0;
}
