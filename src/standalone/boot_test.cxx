/*
    This version performs a regwrite on a dummy address in memory
    also performs a reg read to ensure it works
*/

#include <stdio.h>
#include <ApolloSM/ApolloSM.hh>
#include <ApolloSM/ApolloSM_Exceptions.hh>
#include <uhal/uhal.hpp>
#include <vector>
#include <string>
#include <boost/tokenizer.hpp>
#include <unistd.h> // usleep, execl
#include <signal.h>
#include <time.h>

#include <syslog.h>  ///for syslog

#include <boost/program_options.hpp>
#include <standalone/optionParsing.hh>
#include <standalone/optionParsing_bool.hh>
#include <standalone/daemon.hh>

#include <fstream>
#include <iostream>

// =========================================================================
// set up for boost program_options
#define DEFAULT_CONFIG_FILE "/home/cms/ami/boot_test"
namespace po = boost::program_options;
// =========================================================================

int main(/*int argc, char** argv*/) {

    // map of all parsed options
    std::map<std::string, std::vector<std::string> > allOptions;

    po::options_description cfg_options("RegWrite Options");
    try {
        // get parsed options from cfg file
        std::ifstream configFile(DEFAULT_CONFIG_FILE);
        po::parsed_options cfg_parsed = po::parse_config_file(configFile, cfg_options, true);

        std::cout << "parsing cfg file" << std::endl;

        // store parsed cfg options into allOptions
        for(size_t iCfg = 0; iCfg < cfg_parsed.options.size(); iCfg++) { //iterate through all parsed cfg options

            // in final production code, we will have parsed all the other command line options and cfg options
            // so here, check instead to ensure that string_key is equal to "reg_write"
            /*
                if(cfg_parsed.options[iCfg].string_key == "reg_write")
            */
            // and change the map name from allOptions to something else.
            // this code will all happen after the initial command line and cfg file options are parsed 
            
            if(cfg_parsed.options[iCfg].string_key == "reg_write") {    //Ignore everything else, thye've already been parsed
                std::string name = cfg_parsed.options[iCfg].string_key; //get name "string_key" of option
                std::string value = ""; //Set value empty to start

                std::cout << name << " " << value << std::endl;

                if(cfg_parsed.options[iCfg].value.size()) { //if value of option is not empty
                    for(size_t i = 0; i < cfg_parsed.options[iCfg].value.size(); i++) {//iterate through all values of this option, think vector
                        value +=  cfg_parsed.options[iCfg].value[i];
                    } 
                }
                allOptions[name].push_back(value);
            }
        }
    } catch (std::exception &e) {
        fprintf(stderr, "ERROR storing config file arguments: %s\n", e.what());
    }

    // create map of reg_write args
    std::map<std::string, uint32_t> regwrite_map;
    std::vector<std::string> regWrites = allOptions["reg_write"];
    for (auto RW = regWrites.begin(); RW != regWrites.end(); RW++) {
        std::string addr = (*RW).substr(0, (*RW).find(" "));    // address is first word before first space (" ")
        uint32_t val = std::stoul((*RW).substr((*RW).find(" "), (*RW).size() -1), nullptr, 16);  // value to write is everything after space (" ")

        std::cout << addr << " " << val << std::endl;

        regwrite_map.insert({addr, val});
    }

    // initialize ApolloSM
    std::vector<std::string> arg;
    arg.push_back("/opt/address_table/connections.xml");
    ApolloSM * SM = NULL;
    SM = new ApolloSM();
    if (NULL == SM) {
        //syslog(LOG_ERR, "Failed to create a new ApolloSM\n");
        std::cout << "Failed to create a new ApolloSM" << std::endl;
    }
    else {
        //syslog(LOG_INFO, "Created new ApolloSM\n");
        std::cout << "Created new ApolloSM" << std::endl;
    }
    SM->Connect(arg);
    std::cout << "ApolloSM connected" << std::endl;

    // iterate over the reg_write map of {address, value} pairs
    std::map<std::string, uint32_t>::iterator it = regwrite_map.begin();

    std::cout << "iterating over address/val map" << std::endl;
    while (it != regwrite_map.end()) {
        // access key (address in memory to which we want to write)
        std::string ADDR = it->first;
        // access value (value to write to it)
        uint32_t VAL = it->second;
        std::cout << ADDR << " " << VAL << std::endl;

        // perform regWrite
        SM->RegWriteRegister(ADDR,VAL);
        // uncomment this if you want to test the regRead
        SM->RegReadRegister(ADDR);


        it++;
    }
}