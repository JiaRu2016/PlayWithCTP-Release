#include "Td.hpp"

int main() {
    TraderHandler* tdhandler = new TraderHandler();
    tdhandler->connect();
    tdhandler->login();
    tdhandler->settlement_info_confirm();
    tdhandler->qry_position();
    tdhandler->qry_position_detail();
    tdhandler->qry_order();
    usleep(800000);
    tdhandler->run_console();

    return 0;
}