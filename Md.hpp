#include "include/ThostFtdcMdApi.h"
#include "include/ThostFtdcTraderApi.h"
#include "include/DataCollect.h"

#include <stdio.h>
#include <cstring>
#include <unistd.h>
#include <vector>
#include <string>
#include <iostream>

#include "mappings.h"
#include "myutils.hpp"
#include "config.h"


class MdHandler : public CThostFtdcMdSpi {
private:
    CThostFtdcMdApi* pApi;
    int nRequestID = 0;
    const char* Front = CONFIG_mdFront;
    const char* BrokerID = CONFIG_BrokerID;
    const char* InvestorID = CONFIG_InvestorID;
    const char* Password = CONFIG_Password;
public:
    const std::vector<std::string> instruments = CONFIG_instruments;
    SyncEvent event_front_connected;
    SyncEvent event_loggedin;

public:
    MdHandler() {};
    ~MdHandler() {};
    void connect();
    void login();
    void subscribe();
    void run();
public:
    void OnFrontConnected();
    void OnFrontDisconnected(int nReason);
    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);
    void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
};


///////////////////////////////////////////////////////////////////////////////////////
//  SECTION handler methods
///////////////////////////////////////////////////////////////////////////////////////

void MdHandler::connect() {
    pApi = CThostFtdcMdApi::CreateFtdcMdApi("./flow_md/", false, false);
    pApi->RegisterSpi(this);
    pApi->RegisterFront((char*)Front);
    pApi->Init();

    event_front_connected.Wait();
};

void MdHandler::login() {
    CThostFtdcReqUserLoginField loginfield;
    //memset(&loginfield, 0, sizeof(loginfield));
    strcpy(loginfield.Password, Password);
    strcpy(loginfield.BrokerID, BrokerID);
    strcpy(loginfield.UserID, Password);

    int res = pApi->ReqUserLogin(&loginfield, 42);
    const char* txt;
    switch (res)
    {
        case 0: txt = "登录请求发送成功"; break;
        case -1: txt = "网络连接失败"; break;
        case -2: txt = "未处理请求超过许可数"; break;
        case -3: txt = "每秒发送请求数超过许可数"; break;
        default: break;
    }
    printf("<== Api::ReqUserLogin: %s\n", txt);

    event_loggedin.Wait();
}

void MdHandler::subscribe() {
    const int n = instruments.size();
    char **ppInstrumentID = new char*[n];
    for (int i = 0; i < n; i++) {
        ppInstrumentID[i] = (char*)(instruments[i].c_str());
    }

    int res = pApi->SubscribeMarketData(ppInstrumentID, n);
    const char* txt;
    switch (res) {
        case 0: txt = "订阅请求发送成功"; break;
        case -1: txt = "网络连接失败"; break;
        case -2: txt = "未处理请求超过许可数"; break;
        case -3: txt = "每秒发送请求数超过许可数"; break;
        default: break;
    }
    printf("<== Api::SubscribeMarketData: %s\n", txt);
}

void MdHandler::run() {
    while (true) {
        sleep(9000000);
    }
}

///////////////////////////////////////////////////////////////////////////////////////
//  SECTION overwrite OnXXX()
///////////////////////////////////////////////////////////////////////////////////////

void MdHandler::OnFrontConnected() {
    printf("==> OnFrontConnected. 行情前置连接成功 \n");
    event_front_connected.Set();
};

void MdHandler::OnFrontDisconnected(int nReason) {
    const char* txt = "[unknown reason]";
    switch (nReason)
    {
        case 0x1001: txt = "网络读失败"; break;
        case 0x1002: txt = "网络写失败"; break;
        case 0x2001: txt = "接收心跳超时"; break;
        case 0x2002: txt = "发送心跳失败"; break;
        case 0x2003: txt = "收到错误报文"; break;
        default: break;
    }
    printf("==> OnFrontDisonnected, 行情前置断开 nReason is: %d \t %s\n", nReason, txt);
    event_front_connected.Clear();
};

void MdHandler::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    printf(
        "==> OnRspUserLogin: 行情服务器登录 %s\tnRequestID=%d, bIsLast=%d\n", 
        pRspInfo->ErrorID == 0 ? "成功" : "失败",
        nRequestID,
        bIsLast
    );
    printf(
        "\t==> InfoField: %d %s\n",
        pRspInfo->ErrorID, 
        pRspInfo->ErrorMsg
    );
    printf(
        "\t==> RspUserLogin: TradingDay: %s, LoginTime: %s\n", 
        pRspUserLogin->TradingDay,
        pRspUserLogin->LoginTime
    );
    if (pRspInfo && pRspInfo->ErrorID == 0) {
        event_loggedin.Set();
    }
};

void MdHandler::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
    printf(
        "==> OnRspSubMarketData: 订阅行情 %s : %s\tnRequestID=%d, bIsLast=%d\n", 
        pRspInfo->ErrorID == 0 ? "成功": "失败",
        (char*)pSpecificInstrument,
        nRequestID,
        bIsLast
    );
};

void MdHandler::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData){
    printf(
        "----->> OnRtnDepthMarketData: 行情推送ing... \t (ActionDay: %s) %s %s %d [%s] %f\n",
        pDepthMarketData->ActionDay,
        pDepthMarketData->TradingDay,
        pDepthMarketData->UpdateTime,
        pDepthMarketData->UpdateMillisec,
        pDepthMarketData->InstrumentID,
        pDepthMarketData->LastPrice
    );
};


void MdHandler::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    printf("==> 发生错误 ❌ \nOnRspError:\n");
    printf(
        "\tInfoField: %d %s\n",
        pRspInfo->ErrorID, 
        pRspInfo->ErrorMsg
    );
};


// //////////////////////////////////////////////////////////////////////////////////////////////////////
// // SECTION main
// //////////////////////////////////////////////////////////////////////////////////////////////////////

// int main() {
//     MdHandler* mdhandler = new MdHandler();
//     mdhandler->connect();
//     mdhandler->login();
//     mdhandler->subscribe();
//     mdhandler->run();

//     return 0;
// };