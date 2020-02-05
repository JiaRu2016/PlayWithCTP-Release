#include "include/ThostFtdcUserApiStruct.h"
#include "include/ThostFtdcUserApiDataType.h"

#include <iostream>
#include <iomanip>
#include <cstring>

#include "mappings.h"


///////////////////////////////////////////////////////////////////////////////////////
//  MyOrder
///////////////////////////////////////////////////////////////////////////////////////

class MyOrder {
public:
    int key;  // 本地维护的订单key；
    int FrontID;
    int SessionID;
    TThostFtdcOrderRefType OrderRef;  // char[13]
    TThostFtdcOrderSysIDType OrderSysID; // char[21] // 推荐使用OrderSysID撤单 需要加上ExchangeID
    TThostFtdcExchangeIDType ExchangeID; // char[9]

    std::string InstrumentID;
	std::string	OrderPriceType;  ///报单价格条件
	std::string	Direction; ///买卖方向
	std::string	CombOffsetFlag; ///组合开平标志
	int	VolumeTotalOriginal; ///数量
	double	LimitPrice; ///价格
	// std::string	CombHedgeFlag; ///组合投机套保标志
	// std::string	TimeCondition; ///有效期类型
	// std::string	VolumeCondition; ///成交量类型
	// std::string	ContingentCondition; ///触发条件
    // std::string ForceCloseReason;  ///强平原因
    std::string OrderStatus;
    std::string OrderSubmitStatus;

public:
    MyOrder(CThostFtdcOrderField*);
    ~MyOrder();
    void print() const;
    void print_onerow() const;
    static void print_header();
};

MyOrder::MyOrder(CThostFtdcOrderField* pOrder) {
    strcpy(OrderSysID, pOrder->OrderSysID);
    strcpy(ExchangeID, pOrder->ExchangeID);
    FrontID = pOrder->FrontID;
    SessionID = pOrder->SessionID;
    strcpy(OrderRef, pOrder->OrderRef);
    InstrumentID = std::string(pOrder->InstrumentID);
    OrderPriceType = ORDER_PRICE_TYPE_MAP[pOrder->OrderPriceType];
    Direction = DIRECTION_MAP[pOrder->Direction];
    CombOffsetFlag = OFFSET_FLAG_MAP[pOrder->CombOffsetFlag[0]];
    VolumeTotalOriginal = pOrder->VolumeTotalOriginal; 
    LimitPrice = pOrder->LimitPrice; 
    OrderStatus = ORDER_STATUS_MAP[pOrder->OrderStatus];
    OrderSubmitStatus = ORDER_SUBMIT_STATUS_MAP[pOrder->OrderSubmitStatus];
};

void MyOrder::print() const {
    std::cout << "\t---------> key: " << key << "<----------" << std::endl;
    std::cout << "\tFrontID: " << FrontID << std::endl;
    std::cout << "\tSessionID: " << SessionID << std::endl;
    std::cout << "\tOrderRef: " << OrderRef << std::endl;
    std::cout << "\tOrderSysID: " << OrderSysID << std::endl;
    std::cout << "\tInstrumentID: " << InstrumentID << std::endl;
    std::cout << "\tOrderPriceType: " << OrderPriceType << std::endl;
    std::cout << "\tDirection: " << Direction << std::endl;
    std::cout << "\tCombOffsetFlag: " << CombOffsetFlag << std::endl;
    std::cout << "\tVolumeTotalOriginal: " << VolumeTotalOriginal << std::endl;
    std::cout << "\tLimitPrice: " << LimitPrice << std::endl;
    std::cout << "\tOrderStatus: " << OrderStatus << std::endl;
    std::cout << "\tOrderSubmitStatus: " << OrderSubmitStatus << std::endl;
};

void MyOrder::print_onerow() const {
    std::cout <<
        std::setw(10) << key <<
        // std::setw(10) << FrontID <<
        // std::setw(15) << SessionID <<
        // std::setw(10) << OrderRef <<
        // std::setw(15) << OrderSysID << 
        std::setw(15) << InstrumentID <<
        // std::setw(15) << OrderPriceType <<
        std::setw(10) << Direction <<
        std::setw(15) << CombOffsetFlag <<
        std::setw(25) << VolumeTotalOriginal <<
        std::setw(15) << LimitPrice <<
        std::setw(20) << OrderStatus <<
        std::setw(20) << OrderSubmitStatus <<
    std::endl;
};

void MyOrder::print_header() {
    std::cout <<
        std::setw(10) << "key" <<
        // std::setw(10) << "FrontID" <<
        // std::setw(15) << "SessionID" <<
        // std::setw(10) << "OrderRef" <<
        // std::setw(15) << "OrderSysID" << 
        std::setw(15) << "InstrumentID" <<
        // std::setw(15) << "OrderPriceType" <<
        std::setw(10) << "Direction" <<
        std::setw(15) << "CombOffsetFlag" <<
        std::setw(25) << "VolumeTotalOriginal" <<
        std::setw(15) << "LimitPrice" <<
        std::setw(20) << "OrderStatus" <<
        std::setw(20) << "OrderSubmitStatus" <<
    std::endl;
};

///////////////////////////////////////////////////////////////////////////////////////
//  MyPosition
///////////////////////////////////////////////////////////////////////////////////////

class MyPosition {
private:
    std::string InstrumentID;   // 合约代码
    std::string /* TThostFtdcPosiDirectionType */ PosiDirection;  // 持仓多空方向
    int /* TThostFtdcVolumeType */  Position;  // 今日持仓
    int /* TThostFtdcVolumeType */  YdPosition;  // 上日持仓
    int /* TThostFtdcVolumeType */ 	TodayPosition; ///今日持仓
    double /* TThostFtdcMoneyType */ PositionCost;   // 持仓成本
    double /* TThostFtdcMoneyType */ UseMargin;  ///占用的保证金
    double /* TThostFtdcMoneyType */ FrozenMargin; ///冻结的保证金
    double /* TThostFtdcRatioType */ MarginRateByMoney; ///保证金率
    double /* TThostFtdcMoneyType */ Commission; ///手续费
    double /* TThostFtdcMoneyType */ CloseProfit; ///平仓盈亏
    double /* TThostFtdcMoneyType */ PositionProfit; ///持仓盈亏

public:    
    MyPosition(CThostFtdcInvestorPositionField*);
    ~MyPosition();
    void print() const;
    void print_onerow() const;
    static void print_header();
};

MyPosition::MyPosition(CThostFtdcInvestorPositionField* pInvestorPosition) {
    InstrumentID = std::string(pInvestorPosition->InstrumentID);
    PosiDirection = POS_DIRECTION_MAP[pInvestorPosition->PosiDirection];
    Position = pInvestorPosition->Position;
    YdPosition = pInvestorPosition->YdPosition;
    TodayPosition = pInvestorPosition->TodayPosition;
    PositionCost = pInvestorPosition->PositionCost;
    UseMargin = pInvestorPosition->UseMargin;
    FrozenMargin = pInvestorPosition->FrozenMargin;
    MarginRateByMoney = pInvestorPosition->MarginRateByMoney;
    Commission = pInvestorPosition->Commission;
    CloseProfit = pInvestorPosition->CloseProfit;
    PositionProfit = pInvestorPosition->PositionProfit;
}; 

void MyPosition::print() const {
    std::cout << "合约代码 InstrumentID: " << InstrumentID << std::endl;
    std::cout << "持仓多空方向 PosiDirection: " << PosiDirection << std::endl;
    std::cout << "今日持仓 Position: " << Position << std::endl;
    std::cout << "上日持仓 YdPosition: " << YdPosition << std::endl;
    std::cout << "今日持仓 TodayPosition: " << TodayPosition << std::endl;
    std::cout << "持仓成本 PositionCost: " << PositionCost << std::endl;
    std::cout << "占用的保证金 UseMargin: " << UseMargin << std::endl;
    std::cout << "冻结的保证金 FrozenMargin: " << FrozenMargin << std::endl;
    std::cout << "保证金率 MarginRateByMoney: " << MarginRateByMoney << std::endl;
    std::cout << "手续费 Commission: " << Commission << std::endl;
    std::cout << "平仓盈亏 CloseProfit: " << CloseProfit << std::endl;
    std::cout << "持仓盈亏 PositionProfit: " << PositionProfit << std::endl;
};

void MyPosition::print_onerow() const {
    std::cout <<
        std::setw(15) << InstrumentID << 
        std::setw(15) << PosiDirection << 
        std::setw(10) << Position << 
        std::setw(15) << YdPosition << 
        std::setw(15) << TodayPosition << 
        std::setw(15) << PositionCost << 
        std::setw(10) << UseMargin << 
        std::setw(15) << FrozenMargin << 
        std::setw(20) << MarginRateByMoney << 
        std::setw(15) << Commission << 
        std::setw(15) << CloseProfit << 
        std::setw(15) << PositionProfit << 
    std::endl;
};

void MyPosition::print_header() {
    std::cout <<
        std::setw(15) << "InstrumentID" << 
        std::setw(15) << "PosiDirection" << 
        std::setw(10) << "Position" << 
        std::setw(15) << "YdPosition" << 
        std::setw(15) << "TodayPosition" << 
        std::setw(15) << "PositionCost" << 
        std::setw(10) << "UseMargin" << 
        std::setw(15) << "FrozenMargin" << 
        std::setw(20) << "MarginRateByMoney" << 
        std::setw(15) << "Commission" << 
        std::setw(15) << "CloseProfit" << 
        std::setw(15) << "PositionProfit" << 
    std::endl;  
};

///////////////////////////////////////////////////////////////////////////////////////
//  generic print functions
///////////////////////////////////////////////////////////////////////////////////////

template<class Container, class Item> 
void print_container_items(const Container& container) {
    std::string line(170, '-');
    std::cout << line << std::endl;
    if (container.empty()) {
        std::cout << "Nothing" << std::endl;
    } else {
        Item::print_header();
        std::cout << line << std::endl;
        for (Item* pitem : container) {
            pitem->print_onerow();
        }
    }
    std::cout << line << std::endl;
};

