#include "Md.hpp"
#include "Td.hpp"

#include <queue>
#include <string>
#include <map>
#include <vector>
#include <stdlib.h>
#include <thread>
#include <functional>

#include "market_info.h"

using namespace std::placeholders;


///////////////////////////////////////////////////////////////////////////////////////
//  SECTION  event engine
///////////////////////////////////////////////////////////////////////////////////////

enum class EventType {
    TARGET_POSITION_EVENT,
    TICK_EVENT,
    BAR_EVENT,
};

template<class EventDataType>
class Event {
public:
    EventType type;
    EventDataType data;
    Event(EventType type_, EventDataType data_): type(type_), data(data_) { };
};


template<class EventDataType>
class EventEngine {
private:
    std::thread run_thread;
    std::queue<Event<EventDataType> > q;
    std::map<EventType, std::vector<std::function<void(EventDataType*)> > > process_func_map;  //void(*)(EventDataType*) 
public:
    EventEngine() {};
    //~EventEngine() {};
    void register_process_function(EventType type, std::function<void(EventDataType*)> pfunc) {  //void (*pfunc)(EventDataType*)
        process_func_map[type].push_back(pfunc);
    };
    void add_event(EventType type, EventDataType data) {
        auto event = Event<EventDataType>(type, data);
        q.push(event);
    };
    void run() {
        while (true) {
            if (q.empty()) {
                usleep(500e3);
                printf("(EventEngine.run) q is empty, sleep 100ms.\n");  // ANCHOR DEBUG
            } else {
                auto event = q.front();
                for (auto pfunc: process_func_map[event.type]) {
                    pfunc(&event.data);
                }
                q.pop();
                printf(" (EventEngine.run) pop one.\n");  // ANCHOR DEBUG
            }
        }
    };
    void start() {
        run_thread = std::thread(&EventEngine<EventDataType>::run, this);
    };
    void join() {
        run_thread.join();
    }
};



///////////////////////////////////////////////////////////////////////////////////////
//  SECTION  DataSource 
///////////////////////////////////////////////////////////////////////////////////////

class MarketData { };

class Tick : public MarketData {
public:
    std::string InstrumentID;
    std::string ExchangeID;
    std::string UpdateTime;
    int UpdateMillisec;
    double LastPrice;
    double BidPrice1;
    double AskPrice1;
    int BidVolume1;
    int AskVolume1;
    
    Tick(CThostFtdcDepthMarketDataField *pDepthMarketData)
        : InstrumentID(pDepthMarketData->InstrumentID)
        , UpdateTime(pDepthMarketData->UpdateTime)
        , UpdateMillisec(pDepthMarketData->UpdateMillisec)
        , LastPrice(pDepthMarketData->LastPrice)
        , BidPrice1(pDepthMarketData->BidPrice1)
        , AskPrice1(pDepthMarketData->AskPrice1)
        , BidVolume1(pDepthMarketData->BidVolume1)
        , AskVolume1(pDepthMarketData->AskVolume1)
        {
            ExchangeID = get_exchange_id(InstrumentID);
        };

    void print() {
        printf(
            "\033[1;31m<Tick> %s %d [%s.%s] %.2f --- %.2f ---- %.2f \033[0m\n",
            UpdateTime.c_str(), UpdateMillisec, 
            InstrumentID.c_str(), ExchangeID.c_str(), 
            AskPrice1, LastPrice, BidPrice1
        );
    }
};

class Bar : public MarketData {
public:
    std::string InstrumentID;
    std::string UpdateTime;
    double Open;
    double High;
    double Low;
    double Close;
    int Volume;
    int OpenInterest;
};

class DataSource : public MdHandler /* try inherit more data source eg.wind, webcrul */ {
public:
    EventEngine<Tick>* p_eengine_market_data;
public:
    DataSource() {};
    ~DataSource();
    void start();
    void set_engine(EventEngine<Tick>* engine) { p_eengine_market_data = engine; };
    void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);
};

void DataSource::start() {
    connect();
    login();
    subscribe();
}

void DataSource::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) {
    Tick* tick = new Tick(pDepthMarketData);
    p_eengine_market_data->add_event(EventType::TICK_EVENT, *tick);
    tick->print();
};

///////////////////////////////////////////////////////////////////////////////////////
//  SECTION  Strategy && Algo 
///////////////////////////////////////////////////////////////////////////////////////


class TargetPosition {
public:
    std::string InstrumentID;
    std::string ExchangeID;
    int TargetPos;

    TargetPosition(std::string instrument_id, std::string exchange_id, int pos)
        : InstrumentID(instrument_id)
        , ExchangeID(exchange_id)
        , TargetPos(pos)
        {};
};


class Algo : public TraderHandler {
public:
    std::map<std::string /* instrument_id */, int /* current pos */> current_position = {};
    std::map<std::string /* instrument_id */, Tick*> current_price = {};
public:
    //Algo() { };
    // ~Algo();
    void process_target_position(TargetPosition* pos);
    void update_current_price(Tick* tick);
    double get_current_price(std::string instrument_id);

    //void OnRtnOrder(CThostFtdcOrderField *pOrder);
    void OnRtnTrade(CThostFtdcTradeField *pTrade);
};

void Algo::process_target_position(TargetPosition* pos) {

    std::string instrument_id = pos->InstrumentID;
    std::string exchange_id = pos->ExchangeID;
    int old_pos = current_position[instrument_id];  // in `current_position` pos is **singned** integer
    int pos_increment = pos->TargetPos - old_pos;
    char direction;
    double price;

    if (pos_increment > 0) {
        direction = 'B';
    } else if (pos_increment < 0) {
        direction = 'S';
    } else {
        cancel_all(instrument_id, exchange_id);
        return;
    }
    try {
        price = get_current_price(instrument_id); // TODO  ask_1, bid_1, tmp: last_price.
    } catch (const std::out_of_range& e) {
        // ANCHOR 
        printf("Algo::get_current_price out_of_range error. instrument_id = %s.\n", instrument_id.c_str());
        return;
    }
    
    
    cancel_all(instrument_id, exchange_id);

    unsigned int volume_to_close = (old_pos * pos_increment >= 0) ? 0 : std::max(abs(old_pos), abs(pos_increment));
    unsigned int volume_to_open = (old_pos * pos_increment >= 0) ? abs(pos_increment) : std::max(0, abs(pos_increment) - abs(old_pos));

    if (volume_to_close) {
        if (exchange_id == "SHFE") {
            // TODO 
        } else {
            char offset = 'C';
            order_insert(instrument_id, exchange_id, direction, offset, price, volume_to_close);
        }
    } 
    if (volume_to_open) {
        char offset = 'O';
        order_insert(instrument_id, exchange_id, direction, offset, price, volume_to_open);
    }
};

void Algo::update_current_price(Tick* tick) {
    std::string instrument_id = std::string(tick->InstrumentID);
    current_price[instrument_id] = tick;
};

double Algo::get_current_price(std::string instrument_id) {
    return current_price.at(instrument_id)->LastPrice;
};

void Algo::OnRtnTrade(CThostFtdcTradeField *pTrade) {
    TraderHandler::OnRtnTrade(pTrade);
    std::string instrument_id = std::string(pTrade->InstrumentID);
    int sign = 0;
    switch (pTrade->Direction)
    {
        case THOST_FTDC_D_Buy: sign = 1; break;
        case THOST_FTDC_D_Sell: sign = 2; break;
        default: break;
    }
    current_position[instrument_id] += sign * pTrade->Volume;
};

class Strategy {
private:
    EventEngine<TargetPosition>* p_engine;
protected:
    void target(std::string instrument_id, std::string exchange_id, int pos);   // function that set target_postion
    // TODO context obj that can bind anything to it

public:
    Strategy() {};
    ~Strategy();
    void set_engine(EventEngine<TargetPosition>* engine_) { p_engine = engine_; };
    virtual void init() = 0;
    virtual void on_tick(Tick* tick) = 0;
    virtual void on_bar(Bar* Bar) = 0;
};

void Strategy::target(std::string instrument_id, std::string exchange_id, int pos) {
    TargetPosition* target_pos = new TargetPosition(instrument_id, exchange_id, pos);
    p_engine->add_event(EventType::TARGET_POSITION_EVENT, *target_pos);
};

///////////////////////////////////////////////////////////////////////////////////////
class RandomStrategy: public Strategy {
public:
    RandomStrategy() {};
    
    void init() { 

    };
    
    void on_bar(Bar* bar) {

    };

    void on_tick(Tick* tick) {
        int pos = rand() % 9 - 4;  // [-4, 4]
        target(tick->InstrumentID, tick->ExchangeID, pos);
    };
};

///////////////////////////////////////////////////////////////////////////////////////
//  SECTION  event engine
///////////////////////////////////////////////////////////////////////////////////////

int main() {

    DataSource* data_source = new DataSource();
    Algo* algo = new Algo();
    RandomStrategy* random_strategy = new RandomStrategy();

    // 2 Event Engine
    EventEngine<Tick>* eengine_market_data = new EventEngine<Tick>();

    eengine_market_data->register_process_function(
        EventType::TICK_EVENT, 
        // NOTE: figure out why these work / not work ?
        //random_strategy->on_tick
        //std::bind(Strategy::on_tick, random_strategy, _1);
        [random_strategy] (Tick* tick) { random_strategy->on_tick(tick); }
    );

    eengine_market_data->register_process_function(
        EventType::TICK_EVENT, 
        //algo->update_current_price
        //std::bind(Algo::update_current_price, algo, _1);
        [algo] (Tick* tick) { algo->update_current_price(tick); }
    );

    EventEngine<TargetPosition>* eengine_target_position = new EventEngine<TargetPosition>();
    eengine_target_position->register_process_function(
        EventType::TARGET_POSITION_EVENT, 
        //algo->process_target_position
        // std::bind(Algo::process_target_position, algo, _1)
        [algo] (TargetPosition* pos) { algo->process_target_position(pos); }
    );

    data_source->set_engine(eengine_market_data);
    random_strategy->set_engine(eengine_target_position);

    data_source->connect();
    data_source->login();
    data_source->subscribe();
    algo->connect();
    algo->login();
    algo->settlement_info_confirm();
    algo->qry_order();

    printf("--------------- 1\n");

    eengine_market_data->start();
    eengine_target_position->start();

    printf("--------------- 2\n");

    eengine_market_data->join();
    eengine_target_position->join();

}