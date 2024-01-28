#include <iostream>
#include <fstream>
#include <string>
#include <random>

int main(void)
{
    double p = 0; //决定有p的可能性该数据后没有插入数据
    long long m = 100; //决定这个数据后产生的插入数据的最大值

    std::ifstream infile;
    infile.open("new_stock.txt", std::ios::in);
    if(!infile.is_open())
    {
        std::cout << "open read file failed" << std::endl;
        return 0;
    }
    std::string buildfilename = "./data/" + std::to_string(p) + "_" + std::to_string(m) + "_build.txt";
    std::string insertfilename = "./data/" + std::to_string(p) + "_" + std::to_string(m) + "_insert.txt";
    std::ofstream buildfile(buildfilename);
    std::ofstream insertfile(insertfilename);
    if(!buildfile.is_open() || !insertfile.is_open())
    {
        std::cout << "open write file failed" << std::endl;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis1(0.1, 1);
    std::uniform_int_distribution<> dis2(1, m);

    int num = 0;
    std::string buf;
    while(std::getline(infile, buf)){
        if(num > 0)
        {
            num--;
            insertfile << buf << std::endl;
        }
        else{
            buildfile << buf << std::endl;
            int randomp = dis1(gen);
            if(randomp <= p)
                continue;
            else{
                int randomnum = dis2(gen);
                num = randomnum;
            }
        }    
        
    }
    infile.close();
    buildfile.close();
    insertfile.close();
}