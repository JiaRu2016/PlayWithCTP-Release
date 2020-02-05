#pragma once

#include "include/ThostFtdcMdApi.h"
#include "include/ThostFtdcTraderApi.h"
#include "include/DataCollect.h"
#include "include/ThostFtdcUserApiStruct.h"
#include "include/ThostFtdcUserApiDataType.h"

#include <unistd.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <map>
#include <deque>
#include <vector>
#include <string>

#include "mappings.h"
#include "mykls.hpp"
#include "myutils.hpp"
#include "config.h"


class TraderHandler: public CThostFtdcTraderSpi {
private:
    CThostFtdcTraderApi* pApi;
    // (FrontID, SessionID, OrderRef)
    int FrontID;     // FrontID
    int SessionID;   // SessionID  
    char OrderRef = '0';  // todo  --> int
    int OrderActionRef = 0;
    int nRequestID = 0;
    
    const char* Front = CONFIG_tdFront;
    const char* BrokerID = CONFIG_BrokerID;
    const char* InvestorID = CONFIG_InvestorID;
    const char* Password = CONFIG_Password;

    std::deque<MyPosition*> mypos = {};
    int order_key = 0;
    std::map<int /* key */, MyOrder*> myorders;
public:
    SyncEvent event_front_connected;
    SyncEvent event_loggedin;
    SyncEvent event_settlement_info_confirmed;

public:
    void connect();
    void login();
    void settlement_info_confirm();
    void qry_position(); 
    void qry_position_detail();
    void qry_order();

    // TODO tmp use serveral params. User MyOrder() obj instead 
    void order_insert(std::string InstrumentID, std::string ExchangeID, char Direction, char CombOffsetFlag, double LimitPrice, int VolumeTotalOriginal);
    void order_action(int key); 
    void cancel_all(std::string instrument_id, std::string exchange_id);
    void cancel_all();

    void order_insert_console();
    void order_action_console();
    void run_console();

    ///////////////////////////// overwrite OnXXX() methods ////////////////////////////////////
    // 登录相关
    void onFrontConnected();
    void OnFrontDisconnected(int nReason);
    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    // 错误响应
    void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    // 查持仓相关
    void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    
    // 查报单相关
    void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    // 报单相关
    void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo);
	void OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo);
    void OnRtnOrder(CThostFtdcOrderField *pOrder);
	void OnRtnTrade(CThostFtdcTradeField *pTrade);

    template<class Field> void req(int (CThostFtdcTraderApi::*ReqXXX)(Field*,int), Field* field, int nRequestID, const char* fanme);
    void on_rsp(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast, const char* fname);
    void on_err_rtn(CThostFtdcRspInfoField* pRspInfo, const char* fname);
    void print_p_rsp_info(CThostFtdcRspInfoField* pRspInfo);
};

///////////////////////////////////////////////////////////////////////////////////////
//  req, on_rsp, on_err_rtn
///////////////////////////////////////////////////////////////////////////////////////

template<class Field> void TraderHandler::req(int (CThostFtdcTraderApi::*ReqXXX)(Field*,int) , Field* field, int nRequestID, const char* fname) {
    int res = (pApi->*ReqXXX)(field, nRequestID);
    if (res == 0) { 
        std::cout << "<== " << fname << " SUCCESS." << std::endl;
    } else {
        std::cout << "<== " << fname << " ERROE ❌ ErrorCode = " << res << "  ErrorMapped = " << REQ_ERROR_MAP[res] << std::endl;
    }
}

void TraderHandler::on_rsp(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast, const char* fname) {
    printf("==> %s \t nRequestID=%d, bIsLast=%d\n", fname, nRequestID, bIsLast);
    print_p_rsp_info(pRspInfo);
};

void TraderHandler::on_err_rtn(CThostFtdcRspInfoField* pRspInfo, const char* fname) {
    printf("==> %s ❌\n", fname);
    print_p_rsp_info(pRspInfo);
};

void TraderHandler::print_p_rsp_info(CThostFtdcRspInfoField* pRspInfo) {
    if (pRspInfo) {
        printf(
            "\tErrorID = %d => \"%s\" \tErrorMsg = %s\n", 
            pRspInfo->ErrorID,
            RSP_ERROR_MAP[pRspInfo->ErrorID].c_str(),
            pRspInfo->ErrorMsg
        );
        // TODO: gb2312 --> utf8.  
        // tmp: open `logfile` in vscode specifying encodeing.
        std::ofstream logfile;
        logfile.open ("log", std::ios_base::app);
        logfile << "pRspInfo->ErrorMsg\t" << pRspInfo->ErrorMsg << "\n";
        logfile.close();
    } else {
        printf("\tWarning: InfoField is null_ptr\n");
    }
};

///////////////////////////////////////////////////////////////////////////////////////
//  onRspError
///////////////////////////////////////////////////////////////////////////////////////

void TraderHandler::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    on_rsp(pRspInfo, nRequestID, bIsLast, "OnRspError ❌");
};

///////////////////////////////////////////////////////////////////////////////////////
//  handler methods
///////////////////////////////////////////////////////////////////////////////////////

void TraderHandler::connect() {
    pApi = CThostFtdcTraderApi::CreateFtdcTraderApi("./flow_trader/");
    pApi->RegisterSpi(this);
    pApi->SubscribePublicTopic(THOST_TERT_QUICK);
    pApi->SubscribePrivateTopic(THOST_TERT_QUICK);
    pApi->RegisterFront((char*)Front);
    pApi->Init();
    printf("==> trader ApiVersion: %s\n", pApi->GetApiVersion()); //pApi->GetTradingDay() 只有当登录成功后才会取到正确的值

    usleep(500000);  // onFrontConnected() is not actually callback-ed ...
    event_front_connected.Set();  // ... so usleep() and set it, pretending connected.
    event_front_connected.Wait();
};

void TraderHandler::login() {
    CThostFtdcReqUserLoginField field = {0};
    strcpy(field.BrokerID, BrokerID);
    strcpy(field.UserID, InvestorID);
    strcpy(field.Password, Password);

    req(&CThostFtdcTraderApi::ReqUserLogin, &field, ++nRequestID, "ReqUserLogin");
    event_loggedin.Wait();
};

void TraderHandler::settlement_info_confirm() {
    CThostFtdcSettlementInfoConfirmField field = {0};
    strcpy(field.BrokerID, BrokerID); 
    strcpy(field.InvestorID, InvestorID);

    req(&CThostFtdcTraderApi::ReqSettlementInfoConfirm, &field, ++nRequestID, "ReqSettlementInfoConfirm");
    event_settlement_info_confirmed.Wait();
};

void TraderHandler::qry_position() {
    mypos.clear();
    CThostFtdcQryInvestorPositionField field = {0};
    strcpy(field.BrokerID, BrokerID);
    strcpy(field.InvestorID, InvestorID);

    req(&CThostFtdcTraderApi::ReqQryInvestorPosition, &field, ++nRequestID, "ReqQryInvestorPosition");
};


void TraderHandler::qry_position_detail() {
    CThostFtdcQryInvestorPositionDetailField field = {0};
    strcpy(field.BrokerID, BrokerID);
    strcpy(field.InvestorID, InvestorID);

    req(&CThostFtdcTraderApi::ReqQryInvestorPositionDetail, &field, ++nRequestID, "ReqQryInvestorPositionDetail");
};

void TraderHandler::qry_order() {
    myorders.clear();
    CThostFtdcQryOrderField field = {0};
    strcpy(field.BrokerID, BrokerID);
    strcpy(field.InvestorID, InvestorID);

    req(&CThostFtdcTraderApi::ReqQryOrder, &field, ++nRequestID, "ReqQryOrder");
};

void TraderHandler::run_console() {
    while (true) {
        std::cout << "-----------------------" << std::endl;
        std::cout << "|  1. ReqOrderInsert  |" << std::endl;
        std::cout << "|  2. ReqOrderAction  |" << std::endl;
        std::cout << "|  3. ReqQryPosition  |" << std::endl;
        std::cout << "|  4. ReqQryOrder     |" << std::endl;
        std::cout << "-----------------------" << std::endl;
        int num;
        std::cin >> num;
        
        switch (num)
        {
            case 1: order_insert_console(); break;
            case 2: order_action_console(); break;
            case 3: qry_position(); break;
            case 4: qry_order(); break;
            default: std::cout << "Invalid number." << std::endl; break;
        }
        usleep(1500000);
    }
};
void TraderHandler::order_insert_console() {
    std::cout << "************ ReqOrderInsert ************ OrderRef = " << OrderRef << std::endl;
    std::string InstrumentID;
    std::string ExchangeID;
    char Direction;
    char CombOffsetFlag;
    double LimitPrice;
    int VolumeTotalOriginal;
    std::cout << "Enter InstrumentID ExchagneID: ";
    std::cin >> InstrumentID >> ExchangeID;
    std::cout << "Enter Direction(B/S), CombOffsetFlag(O/C):  ";
    std::cin >> Direction >> CombOffsetFlag;
    std::cout << "Enter LimitPrice: ";
    std::cin >> LimitPrice;
    std::cout << "Enter VolumeTotalOriginal: ";
    std::cin >> VolumeTotalOriginal;
    std::cout << "************ done ************" << std::endl;

    order_insert(InstrumentID, ExchangeID, Direction, CombOffsetFlag, LimitPrice, VolumeTotalOriginal);
};

void TraderHandler::order_insert(std::string InstrumentID, std::string ExchangeID, char Direction, char CombOffsetFlag, double LimitPrice, int VolumeTotalOriginal) 
{
    ++OrderRef;

    CThostFtdcInputOrderField input_order_field = {0};
    
    strcpy(input_order_field.BrokerID, BrokerID);
    strcpy(input_order_field.InvestorID, InvestorID);

    input_order_field.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;  ///投机 MUST
    input_order_field.VolumeCondition = THOST_FTDC_VC_AV;  ///任意数量 MUST
    input_order_field.ContingentCondition = THOST_FTDC_CC_Immediately;  ///触发条件=立即 MUST
    input_order_field.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;  ///强平原因=非强平 MUST
    input_order_field.IsAutoSuspend = 0;
    input_order_field.MinVolume = 1;

    input_order_field.OrderRef[0] = OrderRef;  // todo
    strcpy(input_order_field.InstrumentID, InstrumentID.c_str());
    strcpy(input_order_field.ExchangeID, ExchangeID.c_str());
    input_order_field.LimitPrice = LimitPrice;
    input_order_field.VolumeTotalOriginal = VolumeTotalOriginal;

    input_order_field.OrderPriceType = THOST_FTDC_OPT_LimitPrice; 
    input_order_field.TimeCondition = THOST_FTDC_TC_GFD;  ///当日有效 MUST 

    switch (Direction)
    {
        case 'B': input_order_field.Direction = THOST_FTDC_D_Buy; break;
        case 'S': input_order_field.Direction = THOST_FTDC_D_Sell; break;
        default: std::cout << "invalied `Direction`" << std::endl;  break;
    }
    switch (CombOffsetFlag)
    {
        case 'O': input_order_field.CombOffsetFlag[0] = THOST_FTDC_OF_Open; break;
        case 'C': input_order_field.CombOffsetFlag[0] = THOST_FTDC_OF_Close; break;  // Close --> CloseYesterday
        case 'T': input_order_field.CombOffsetFlag[0] = THOST_FTDC_OF_CloseToday; break;
        case 'Y': input_order_field.CombOffsetFlag[0] = THOST_FTDC_OF_CloseYesterday; break;
        default: std::cout << "invalied `Direction`" << std::endl;  break;
    }

    req(&CThostFtdcTraderApi::ReqOrderInsert, &input_order_field, ++nRequestID, "ReqOrderInsert");

};


void TraderHandler::order_action_console() {
    std::cout << "************ ReqOrderAction ************ OrderActionRef = " << OrderActionRef << std::endl;
    int key;
    std::cout << "key: ";
    std::cin >> key;
    std::cout << "************ done ************" << std::endl;

    order_action(key);
};

void TraderHandler::order_action(int key) {
    ++OrderActionRef;

    CThostFtdcInputOrderActionField field = {0};
    strcpy(field.BrokerID, BrokerID);
    strcpy(field.InvestorID, InvestorID);
    field.ActionFlag = THOST_FTDC_AF_Delete;

    MyOrder* porder = myorders.at(key);
    // 撤单两种方式
    // 1. OrderSysID + exchangeID
    // 2. FrontID + SessionID + OrderRef  DO NOT WORK
    strcpy(field.OrderSysID, porder->OrderSysID);
    strcpy(field.ExchangeID, porder->ExchangeID);

    req(&CThostFtdcTraderApi::ReqOrderAction, &field, ++nRequestID, "ReqOrderInsert");
};

void TraderHandler::cancel_all(std::string instrument_id, std::string exchange_id) {
    for (auto item: myorders) {
        bool b = \
            item.second->InstrumentID  == instrument_id && 
            item.second->ExchangeID == exchange_id &&
            (
                item.second->OrderStatus == "_NoTradeQueueing" || 
                item.second->OrderStatus == "_PartTradedQueueing"
            );
        if (b) {
            order_action(item.first);
        }
    }
};

void TraderHandler::cancel_all() {
    for (auto item: myorders) {
        bool b = item.second->OrderStatus == "_NoTradeQueueing" || item.second->OrderStatus == "_PartTradedQueueing";
        if (b) {
            order_action(item.first);
        }
    }
};

///////////////////////////////////////////////////////////////////////////////////////
//  overwrite OnXXX() methods 
///////////////////////////////////////////////////////////////////////////////////////

void TraderHandler::onFrontConnected() {
    printf("==> OnFrontConnected.\n");
    // 实际未被调用？？
    event_front_connected.Set();
};

void TraderHandler::OnFrontDisconnected(int nReason) {
    printf("==> OnFrontDisconnected  交易服务器断开 api会自动重连...\n");
    event_front_connected.Clear();
};

void TraderHandler::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    on_rsp(pRspInfo, nRequestID, bIsLast, "OnRspUserLogin");
    printf(
        "\tRspUserLogin: TradingDay: %s, LoginTime: %s\n", 
        pRspUserLogin->TradingDay,
        pRspUserLogin->LoginTime
    );
    FrontID = pRspUserLogin->FrontID;
    SessionID = pRspUserLogin->SessionID;
    if (pRspInfo && pRspInfo->ErrorID == 0) {
        event_loggedin.Set();
    }
};

void TraderHandler::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    on_rsp(pRspInfo, nRequestID, bIsLast, "OnRspSettlementInfoConfirm");
    printf(
        "\tpSettlementInfoConfirm: ConfirmDate: %s, ConfirmTime: %s\n", 
        pSettlementInfoConfirm->ConfirmDate,
        pSettlementInfoConfirm->ConfirmTime
    );
    if (pRspInfo && pRspInfo->ErrorID == 0) {
        event_settlement_info_confirmed.Set();
    }
};

void TraderHandler::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    on_rsp(pRspInfo, nRequestID, bIsLast, "OnRspQryInvestorPosition");
    if (pInvestorPosition) {
        mypos.push_back(new MyPosition(pInvestorPosition));
    } else {
        printf("\tWarning: pInvestorPosition is null_ptr\n");
    }
    if (bIsLast) {
        print_container_items<std::deque<MyPosition*>, MyPosition>(mypos); 
    }
};

void TraderHandler::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    on_rsp(pRspInfo, nRequestID, bIsLast, "OnRspQryInvestorPositionDetail");
    /* deleted */
};

void TraderHandler::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    on_rsp(pRspInfo, nRequestID, bIsLast, "OnRspQryOrder");
    if (pOrder) {
        MyOrder* pMyOrder = new MyOrder(pOrder);
        ++order_key;
        pMyOrder->key = order_key;
        myorders[order_key] = pMyOrder;
    } else { 
        printf("\tpOrder is nullptr\n");
    }
    if (bIsLast) {
        std::vector<MyOrder*> v;
        v.reserve(myorders.size());
        for (auto pair: myorders) {
            v.push_back(pair.second);
        }
        print_container_items<std::vector<MyOrder*>, MyOrder>(v);
    }
};

void TraderHandler::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    on_rsp(pRspInfo, nRequestID, bIsLast, "OnRspOrderInsert");
    if (pInputOrder) {
        printf(
            "\tpInputOrder->OrderRef=%s\n", 
            pInputOrder->OrderRef
        );
    } else {
        printf("\tWarning: pInputOrder is null_ptr \n");
    }
};

void TraderHandler::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo) {
    on_err_rtn(pRspInfo, "OnErrRtnOrderInsert");
    if (pInputOrder) {
        printf(
            "\tpInputOrder.OrderRef=%s\n", 
            pInputOrder->OrderRef
        );
    } else {
        printf("\tWarning: pInputOrder is null_ptr \n");
    } 
};

void TraderHandler::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    on_rsp(pRspInfo, nRequestID, bIsLast, "OnRspOrderAction");
};

void TraderHandler::OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo) {
    on_err_rtn(pRspInfo, "OnErrRtnOrderAction");
};

void TraderHandler::OnRtnOrder(CThostFtdcOrderField *pOrder) {
    printf("==> OnRtnOrder 🔶 \n");
    if (pOrder) {	
        MyOrder* pmyorder = new MyOrder(pOrder);
        pmyorder->print_onerow();
        
        // TODO: gb2312 --> utf8.  
        // tmp: open `logfile` in vscode specifying encodeing.
        std::ofstream logfile;
        logfile.open ("log", std::ios_base::app);
        logfile << "==> OnRtnOrder 🔶 pOrder->StatusMsg\t" << pOrder->StatusMsg << "\n";
        logfile.close();

    } else {
        printf("\tWarning: pOrder is null_ptr \n");
    }
};

void TraderHandler::OnRtnTrade(CThostFtdcTradeField *pTrade) {
    printf("==> OnRtnTrade ✅ \n");
    if (pTrade) {
	    std::cout << "\tpTrade->InstrumentID " << pTrade->InstrumentID << std::endl;
	    std::cout << "\tpTrade->OrderRef " << pTrade->OrderRef << std::endl;
	    std::cout << "\tpTrade->Direction " << DIRECTION_MAP[pTrade->Direction] << std::endl;
	    std::cout << "\tpTrade->OffsetFlag " << OFFSET_FLAG_MAP[pTrade->OffsetFlag] << std::endl;
	    std::cout << "\tpTrade->Price " << pTrade->Price << std::endl;
	    std::cout << "\tpTrade->Volume " << pTrade->Volume << std::endl;
	    std::cout << "\tpTrade->TradeDate " << pTrade->TradeDate << std::endl;
	    std::cout << "\tpTrade->TradeTime " << pTrade->TradeTime << std::endl;
    } else {
        printf("\tWarning: pTrade is null_ptr \n");
    }
};
