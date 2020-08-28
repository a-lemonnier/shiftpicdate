#include <iostream>
#include <cmath>

#include <vector>

#include <algorithm>

#include <string>
#include <iomanip>
#include <sstream>
#include <locale>

#include <chrono>

#include <thread>
#include <future>

// ----------------------------------------------------
#include <exiv2/exiv2.hpp>

#include <boost/program_options.hpp>

#if __has_include (<boost/timer/timer.hpp>)
#include <boost/timer/timer.hpp> 
#define HAS_BOOST_TIMER /**< boost::timer availability */
#endif

#if __has_include (<filesystem>)
#include <filesystem>
#define FS_STD /**< std::filesystem availability (C++17) */
namespace fs = std::filesystem;
#elif __has_include (<experimental/filesystem>) && !__has_include (<filesystem>)
#include <experimental/filesystem>
#define FS_STDEXP /**< std::experimental::filesystem availability */
namespace fs = std::experimental::filesystem;
#elif __has_include(<boost/filesystem.hpp>) && !__has_include (<filesystem>) && !__has_include (<experimental/filesystem>)
#include <boost/filesystem.hpp>
#define FS_BOOST /**< boost::filesystem availability */
namespace fs = boost::filesystem;
#else
#error "No filesystem header found"
#endif
// ----------------------------------------------------

// ----------------------------------------------------
#define EXTENSION_LIST {".JPEG", ".jpeg", \
                        ".JPG",  ".jpg",  \
                        ".CR2",  ".cr2"  }

bool bQuiet;
bool bIsDST;
static const std::string dateTimeFormat{ "%Y:%m:%d %H:%M:%S" };
// ----------------------------------------------------

// Proto ----------------------------------------------
std::string stoyear(long long t);
inline void shift(std::vector<std::string> vList, long long Diff);
inline void shift_q(std::vector<std::string> vList, long long Diff);
inline bool test_ext(const std::string &sS);
// ----------------------------------------------------

int main(int argc, char **argv) {
    
#ifdef HAS_BOOST_TIMER    
boost::timer::cpu_timer btTimer;
#endif    
    
// Parse cmd line
// ----------------------------------------------------  
    namespace po = boost::program_options;
    po::options_description description("Usage");
    
    description.add_options()
    ("help,h"                                                   , "Display this help message.")
    ("quiet,q"                                                  , "toggle verbosity off.")
    ("filename,f",   po::value<std::string>()                   , "Specify one file.")
    ("directory,D",  po::value<std::string>()                   , "Directory where the pictures are.")
    ("thread,t",     po::value<unsigned int>()->default_value(4), "Number of threads.\n")
    ("second,S",     po::value<long long>()->default_value(0)   , " ")
    ("minute,M",     po::value<long long>()->default_value(0)   , " ")
    ("hour,H",       po::value<long long>()->default_value(0)   , " ")
    ("day,d",        po::value<long long>()->default_value(0)   , " ")
    ("month,m",      po::value<long long>()->default_value(0)   , "Based on 30 days/month.")
    ("year,y",       po::value<long long>()->default_value(0)   , "Based on 365 days/year.")
    ("DST"                                                      , "Enable DST.");
    
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(description).run(), vm);
    po::notify(vm);
    
    if (vm.count("help") || vm.size()<1 || !(vm.count("filename") ^ vm.count("directory"))) {
        std::cout << description;
        std::cout << std::endl;
        return EXIT_SUCCESS;
    }
    
    // ----------------------------------------------------
    
    bQuiet=!vm.count("quiet");
    bIsDST=vm.count("DST");
    
    // ---------------------------------------------------- 
    
    const long long Diff=vm["second"].as<long long>()
                        +vm["minute"].as<long long>()*60
                        +vm["hour"].as<long long>()*3600
                        +vm["day"].as<long long>()*3600*24
                        +vm["month"].as<long long>()*3600*24*30
                        +vm["year"].as<long long>()*3600*24*365;
    
    
    // ---------------------------------------------------- 
    
    if (vm.count("directory")) {
        fs::path pFolder(vm["directory"].as<std::string>());
        
        if (!pFolder.empty()) {
            
            std::vector<std::string> vList;
            for(auto &sFile: fs::directory_iterator(vm["directory"].as<std::string>()))
                if (test_ext(sFile.path().filename().string()))
                    vList.push_back(sFile.path().string());
                
            std::sort(vList.begin(), vList.end());
            
            int max_thread = std::min(std::thread::hardware_concurrency(), static_cast<unsigned>(vm["thread"].as<unsigned int>())) ;
            
            const std::size_t size_divided=vList.size()/max_thread;
            
            std::vector<std::vector<std::string> > vvsList_divided;
            
            if (static_cast<int>(vList.size())>max_thread) {
                for(int i=0; i< max_thread-1; i++) 
                    vvsList_divided.emplace_back(std::vector<std::string>(vList.begin()+i*size_divided, vList.begin() + size_divided*(i+1)));
                
                // 1 extra thread for extra files : rest of division
                vvsList_divided.emplace_back(std::vector<std::string>(vList.begin()+(max_thread-1)*size_divided, vList.end()));
            }
            else 
                    vvsList_divided.emplace_back(vList);
            
            
            std::launch lFlag=std::launch::async;
            std::vector<std::future<void> > vfThread;
            
            if (bQuiet)
                for(auto t_list: vvsList_divided) 
                    vfThread.emplace_back(std::async(lFlag, shift, t_list, Diff));
            else
                for(auto t_list: vvsList_divided) 
                    vfThread.emplace_back(std::async(lFlag, shift_q, t_list, Diff));
                
            std::cout << "- Number of files: " << vList.size() << ".\n";
            std::cout << "- Number of threads: " << max_thread << ".\n";
            std::cout << "- Shift date by: " << Diff << "s -> " << stoyear(Diff) << ".\n";
            
            if (static_cast<int>(vList.size())>max_thread)
                std::cout << "- Start " << max_thread << " threads.\n";
            std::for_each(vfThread.begin(), vfThread.end(), [](std::future<void> &th) { th.get(); });
        }
        else
            std::cerr << "Folder does not exist.\n";
    }
    else {
        std::cout << "- Shift date by: " << Diff << "s -> " << stoyear(Diff) << ".\n";
        
        fs::path pFile(vm["filename"].as<std::string>());
        
        if (!pFile.empty()) 
            shift({pFile.string()}, Diff);
        else
            std::cerr << "File does not exist.\n";
    }
    
#ifdef HAS_BOOST_TIMER
std::cout << "\n" << btTimer.format();
#endif
    
    return EXIT_SUCCESS;
}

// ----------------------------------------------------
// ----------------------------------------------------
// ----------------------------------------------------

std::string stoyear(long long t) {
    std::stringstream ssS;
    
    long long M=60;
    long long H=60*M;
    long long D=24*H;
    long long Y=365*D;
    
    long long t0=-t;
    
    if (t<0) 
        ssS << "-"
        << (  t0 / Y)               << "y "
        << (  t0 % Y) / D           << "d "
        << (( t0 % Y) % D) / H      << "h "
        << (((t0 % Y) % D) % H) / M << "m "
        << (((t0 % Y) % D) % H) % M << "s"; 
    else
        ssS << (  t0 / Y)           << "y "
        << (  t0 % Y) / D           << "d "
        << (( t0 % Y) % D) / H      << "h "
        << (((t0 % Y) % D) % H) / M << "m "
        << (((t0 % Y) % D) % H) % M << "s"; 
    
    return ssS.str();
}

void shift(std::vector<std::string> vList, long long Diff) {
    
    for(const auto &sFile: vList)
        if (test_ext(sFile)) {
            
                std::cout << "-> " << sFile << "\n";
                
                auto pImg=Exiv2::ImageFactory::open(sFile);

                pImg->readMetadata();
                
                Exiv2::ExifData &exifData = pImg->exifData();
                
                std::stringstream ssS(exifData["Exif.Image.DateTime"].toString());
                
                std::tm dt={ };
                if (bIsDST) dt.tm_isdst=bIsDST;
                
                ssS >> std::get_time(&dt, dateTimeFormat.c_str());
                
                std::time_t iEpoch=std::mktime(&dt);
                                
                iEpoch+=Diff;
                
                ssS.clear();
                ssS << std::put_time(std::localtime(&iEpoch), (dateTimeFormat).c_str());
                        
                exifData["Exif.Image.DateTime"]=ssS.str();
                    
                pImg->setExifData(exifData);
                pImg->writeMetadata();
                pImg->readMetadata();
            }
            else
                std::cerr << "/!\\ File does not exists.\n";
}

void shift_q(std::vector<std::string> vList, long long Diff) {
    
    for(const auto &sFile: vList)
        if (test_ext(sFile)) {
            
            if (fs::exists(sFile)) {
                
                auto pImg=Exiv2::ImageFactory::open(sFile);
                
                   pImg->readMetadata();
                
                Exiv2::ExifData &exifData = pImg->exifData();
                
                std::stringstream ssS(exifData["Exif.Image.DateTime"].toString());
                std::tm dt={ };
                
                if (bIsDST) dt.tm_isdst=bIsDST;
                
                ssS >> std::get_time(&dt, dateTimeFormat.c_str());
                
                std::time_t iEpoch=std::mktime(&dt);
                                
                iEpoch+=Diff;
                
                ssS.clear();
                ssS << std::put_time(std::localtime(&iEpoch), (dateTimeFormat).c_str());
                        
                exifData["Exif.Image.DateTime"]=ssS.str();
                    
                pImg->setExifData(exifData);
                pImg->writeMetadata();
                pImg->readMetadata();
            }
            else
                std::cerr << "/!\\ File does not exists.\n";
        }
}

bool test_ext(const std::string &sS) { 
    bool res=false;
    for(const auto &ext: EXTENSION_LIST)
        res|=(sS.find(ext)!=std::string::npos);
    return res;
}
