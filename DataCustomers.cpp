#include"httplib.h"

#include<string>
#include<vector>
#include<map>

#include<algorithm>

#include<iostream>
#include<fstream>
#include<sstream>

#include<time.h>
#include<chrono>

using vectMapsString = std::vector<std::map<std::string, std::string>>;
using mapVects = std::map<std::string, std::vector<std::string>>;

using Request = httplib::Request;
using Response = httplib::Response;

struct Data; //прототип структыры для возможностти объявления вектора

std::vector<Data> result; //объявим вектор с данными в глобальной области видимости, для возможности обращения к нему по запросу

//структура для хранения данных об абоненте
struct Data
{   
    std::string m_number;
    std::string m_SNM; //Surname, Name, Midname
    std::string m_activity;

    Data(std::string number = "",std::string SNM = "") : m_number(number), m_SNM(SNM)
    {
        bool activity = rand() % (2);
        if(activity == 0)
            m_activity = "Номер отключен";
        else
            m_activity = "Номер используется";
    }
};

//Класс для получения времени выполнения каждого запроса и чтения файла с последующей записью в лог
class Log
{
//private
    std::ofstream logFile;
    std::chrono::time_point<std::chrono::system_clock> start, end;
    std::ios_base::openmode m_mode;
    
public:
    Log(std::string logName, std::ios_base::openmode mode = std::ios::app) : m_mode(mode)
    {
        logFile.open("log.txt", mode);
        start = std::chrono::system_clock::now();
         
        if (mode == std::ios::app)
        {
            logFile << currentTime() << logName << " ";
        }
    }

    ~Log()
    {
        if (m_mode != std::ios::app)
            logFile.close();
        else
        {
            end = std::chrono::system_clock::now();
            double duration = std::chrono::duration_cast<std::chrono::microseconds> (end - start).count();
            logFile << "(Длительность: "<< duration / 1000.0 << " ms)\n";
            logFile.close();
        }
    }

    std::string currentTime()
    {
        std::time_t t = std::time(nullptr);
        std::tm* now = std::localtime(&t);
        char time[128];
        std::strftime(time, sizeof(time), "%Y/%m/%d %X", now);
        return "["+ std::string(time) + "] ";
    }

};

//функция для чтения csv файла и записи данных из файла в vector map-ов
void readCSV(const std::string filename, vectMapsString &input)
{
    std::string inputString;
    std::ifstream inputStream;
    char sep = ',';
    inputStream.open(filename);
    
    getline(inputStream, inputString); // чтение заголовка
    while(!inputStream.eof())
    {
        getline(inputStream, inputString);
        auto it = inputString.find(sep);
        std::map<std::string, std::string> m;
        auto val = inputString.substr(it + 1);
        auto key = inputString.substr(0, it);
        if (it != std::string::npos)
            m[key] = val;
        input.push_back(m);
    }
    inputStream.close();
}

//перегрузка функции для чтения csv файла и записи данных в map векторов
void readCSV(const std::string filename, mapVects &input)
{
    std::string inputString;
    std::ifstream inputStream;
    char sep = ',';
    inputStream.open(filename);
    getline(inputStream, inputString); // чтение заголовка
    while(!inputStream.eof())
    {
        getline(inputStream, inputString);
        auto it = inputString.find(sep);
        auto val = inputString.substr(0,it);
        auto key = inputString.substr(it + 1);
        if (it != std::string::npos)
            input[key].push_back(val);  

    }
    inputStream.close();
}

//перегрузка функции для чтения csv базы данных абонентов
void readCSV(const std::string filename, std::vector<Data> &input)
{
    Log lg("Чтение базы данных абонентов");
    std::string inputString;
    std::ifstream inputStream;
    char sep = ',';
    inputStream.open(filename);
    while(!inputStream.eof())
    {
        Data d;
        getline(inputStream, inputString);
        std::stringstream stream(inputString); // Преобразование строки в поток
        getline(stream, d.m_number,sep);
        getline(stream,d.m_SNM,sep);
        getline(stream,d.m_activity,sep);
        
        input.push_back(d);  

    }
    inputStream.close();
}

//генерирование ФИО
std::string randSNM(vectMapsString &names, mapVects &midnames, mapVects &surnames)
{
    int idName = rand() % (names.size() - 1);
    std::string name, midname, surname;
    std::map<std::string, std::string> nameElem = names[idName];
    auto iter = nameElem.begin(); 
    name = iter->first;
    if(iter->second != "u\r")
    {
        int idMid = rand() % (midnames[iter->second].size() - 1);
        midname = midnames[iter->second][idMid];
    }

    int idSur = rand() % (surnames[iter->second].size() - 1);
    surname = surnames[iter->second][idSur];

    return surname + " " + name + " " + midname;
}

// произвольный номер
std::string randNumber()
{
    std::string num = "89";

    int cnt = 0;
    while(cnt !=9)
        {
            int n = rand() % (10);
            num += std::to_string(n);
            ++cnt;
        }

    return num;

}

//заполненние файла произвольными значениями
void generateData(long long MaxSize) 
{
    vectMapsString names;
    mapVects midnames;
    mapVects surnames;

    readCSV("names.csv", names);
    readCSV("midnames.csv", midnames);
    readCSV("surnames.csv", surnames);

    std::ofstream resultStream;
    resultStream.open("result.csv", std::ios::out);
    for (long long size = 0; size <= MaxSize;)
    {
        Data d(randNumber(), randSNM(names,midnames,surnames));
        resultStream << d.m_number << "," << d.m_SNM << "," << d.m_activity << std::endl;
        size += sizeof(d);
        //std::cout << size << "\t";
    }
    resultStream.close();
}

// Тестирование успешного старта сервера
void responseStart(const Request& req, Response& res) 
{
    //SimpleTimer st;
    Log lg("Проверка работы хоста");
    std::string str = u8R"(Успешный старт хоста)";
    res.set_content(str, "text/plain; charset=UTF-8");
}

// Формирование запроса на получение ФИО и признака активности по номеру
void responseGetData(const Request& req, Response& res) 
{
    //SimpleTimer st;
    Log lg("Запрос на получение информации по номеру");

    std::string str = u8R"()";
    for (auto item: req.params)
    {
        auto iter = std::find_if(result.begin(), result.end(), [&](const Data &s)-> bool
            { 
            return s.m_number == item.first;
            } );
            str += iter->m_SNM + "\t" + iter->m_activity + "\n";
    }
    res.set_content(str, "text/plain; charset=UTF-8");
}

// Формирование запроса на получение номеров и ФИО всех активных абонентов
void responseGetAll(const Request& req, Response& res) 
{
    //SimpleTimer st;
    Log lg("Запрос на получение списка активных абонентов");
    std::string str = u8R"()";
    for (auto iter = result.begin(); iter != result.end(); ++iter)
    {
        if (iter->m_activity == "Номер используется")
            str += iter->m_number + "\t" + iter->m_SNM + "\n";
    }
    res.set_content(str, "text/plain; charset=UTF-8");
}

int main()
{
    srand(time(NULL));

    long long fileSize = 1*1024*1024*1024;

    generateData(fileSize);

    httplib::Server svr;
     
    Log lg("Запуск сервера", std::ios::out);

    readCSV("result.csv", result);

    svr.Get("/", responseStart); 
    svr.Get("/show", responseGetData);
    svr.Get("/active", responseGetAll); 
    svr.Get("/stop", [&](const Request& req, Response& res) { svr.stop(); });

    svr.listen("localhost", 8080);      

    return 0;
}


