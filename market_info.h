#pragma once

#include <string>
#include <map>
#include <iostream>

static const std::map<std::string /* ProductID */, std::string /* ExchangeID */> exchange_id_map = {

    { "IC",  "CFFEX" },
    { "IF",  "CFFEX" },
    { "IH",  "CFFEX" },
    { "T" ,  "CFFEX" },
    { "TF",  "CFFEX" },
    { "TS",  "CFFEX" },

    { "ag", "SHFE" },  
    { "au", "SHFE" },  
    { "rb", "SHFE" },  
    { "sc", "SHFE" },
    { "cu", "SHFE" },  
    { "al", "SHFE" },  
    { "zn", "SHFE" }, 
    { "pb", "SHFE" },    
    { "ni", "SHFE" },                                                
    { "sn", "SHFE" },

    { "j" , "DCE" },
    { "jm", "DCE" },
    { "i" , "DCE" },
    { "jd", "DCE" },
    { "c" , "DCE" },
    { "cs", "DCE" },
    { "m" , "DCE" },

    {"AP", "CZC"},
    {"SR", "CZC"},
    {"TA", "CZC"},
    {"MA", "CZC"},
    {"CF", "CZC"},
};


bool is_alphabet(const char& c) {
    return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}

std::string get_exchange_id(std::string instrument_id) {
    std::string instrument = std::string(1, instrument_id[0]);
    if ( is_alphabet(instrument_id[1]) ) {
        instrument += instrument_id[1];
    }
    try {
        return exchange_id_map.at(instrument);
    } catch (std::out_of_range) {
        return "";
    }
}
