#ifndef PIXELLOGGER_H_
#define PIXELLOGGER_H_

#include <iostream>

class Logger { // singleton
private:
    Logger(const int num){
        unique_id = num;
    }

    Logger* oneAndOnlyInstance;
    int unique_id;
public:
    Logger* get_instance(const int num){
        if(oneAndOnlyInstance == nullptr){
            std::cout<< "creating a new logger instance" << std::endl;
            oneAndOnlyInstance = new Logger(num);
        }
        std::cout<< "returning instance with unique ID: " << unique_id << std::endl;
        return oneAndOnlyInstance;
    }
};
#endif // PIXELLOGGER_H_
