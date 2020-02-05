#pragma once

#include <vector>
#include <string>

// CTP standard
const char* CONFIG_mdFront = "tcp://180.168.146.187:10110";
const char* CONFIG_tdFront = "tcp://180.168.146.187:10100";
// 7 * 24
// const char* CONFIG_mdFront = "tcp://180.168.146.187:10131";
// const char* CONFIG_tdFront = "tcp://180.168.146.187:10130";
const char* CONFIG_BrokerID = "9999";
const char* CONFIG_InvestorID = "your investor id";  // simnow 投资者编号
const char* CONFIG_Password = "you password";        // simnow 密码

const std::vector<std::string> CONFIG_instruments = {"IF2002","rb2005","AP005","j2005", "sc2003"};
