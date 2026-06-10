#include <QCoreApplication>
#include <QSettings>
#include <QDebug>
#include <iostream>
#include <string>

#include "extract_worker.h"
#include "rebuild_worker.h"

void printUsage() {
    std::cout << "===================================================================\n";
    std::cout << " Fmod Bank Tools - Console Edition - v2.2.3.28\n";
    std::cout << "===================================================================\n";
    std::cout << "Usage:\n";
    std::cout << "  Fmod_Bank_Tools_CLI <mode> [options]\n\n";
    std::cout << "Modes (Required):\n";
    std::cout << "  -e                  Extract mode\n";
    std::cout << "  -r                  Rebuild mode\n\n";
    std::cout << "Options (Optional, overrides config.ini):\n";
    std::cout << "  -bd                 Bank directory location\n";
    std::cout << "  -wd                 Wav directory location\n";
    std::cout << "  -rd                 Rebuild directory location\n";
    std::cout << "  -cd                 Cache directory location\n";
    std::cout << "  -f <format>         Compression format (vorbis, pcm, fadpcm)\n";
    std::cout << "  -q <quality>        Compression quality (1 to 100)\n";
    std::cout << "  -t <threads>        Number of CPU threads to use (default 2)\n";
    std::cout << "  -h                  Get help\n\n";
    std::cout << "Example For Extracting:\n";
    std::cout << "  Fmod_Bank_Tools_CLI -e -bd D:\\Fmod_Bank_Tools_CLI\\bank -wd D:\\Fmod_Bank_Tools_CLI\\wav -rd D:\\Fmod_Bank_Tools_CLI\\build -cd D:\\Fmod_Bank_Tools_CLI\\fsbcache \n";
    std::cout << "Example For Rebuiling:\n";
    std::cout << "  Fmod_Bank_Tools_CLI -r -f vorbis -q 80 -t 4\n";
    std::cout << "===================================================================\n";
}

enum ArgType { Unknown, Extract, Rebuild, BankDir, WavDir, RebuildDir, CacheDir, Format, Quality, Threads, Help };

// Helper to convert string to enum
ArgType getArgType(const std::string& arg) {
    static const std::unordered_map<std::string, ArgType> argMap = {
        {"-e", Extract}, {"-r", Rebuild}, {"-bd", BankDir}, {"-wd", WavDir},
        {"-rd", RebuildDir}, {"-cd", CacheDir}, {"-f", Format},
        {"-q", Quality}, {"-t", Threads}, {"-h", Help}
    };
    auto it = argMap.find(arg);
    return (it != argMap.end()) ? it->second : Unknown;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (argc < 2) {
        printUsage();
        return 1;
    }

    // 1. Parse Command Line Arguments
    std::string mode = "";
    std::string bankDir = "";
    std::string wavDir = "";
    std::string rebuildDir = "";
    std::string cacheDir = "";
    std::string customFormat = "";
    int customQuality = -1;
    int customThreads = -1;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        switch (getArgType(arg)) {
        case Extract:    mode = "extract"; break;
        case Rebuild:    mode = "rebuild"; break;

        case BankDir:    if (++i < argc) bankDir = argv[i];    break;
        case WavDir:     if (++i < argc) wavDir = argv[i];     break;
        case RebuildDir: if (++i < argc) rebuildDir = argv[i]; break;
        case CacheDir:   if (++i < argc) cacheDir = argv[i];   break;
        case Format:     if (++i < argc) customFormat = argv[i]; break;

        case Quality:    if (++i < argc) customQuality = std::stoi(argv[i]); break;
        case Threads:    if (++i < argc) customThreads = std::stoi(argv[i]); break;

        case Help:       printUsage(); return 0;

        case Unknown:
        default:
            std::cout << "Error: Unknown or incomplete argument '" << arg << "'\n";
            printUsage();
            return 1;
        }
    }

    if (mode.empty()) {
        std::cout << "Error: You must specify a mode (-e to extract or -r to rebuild).\n\n";
        printUsage();
        return 1;
    }

    // 2. Inject any custom console arguments directly into the Directorys configuration
    QString config = QCoreApplication::applicationDirPath() + "/config.ini";
    QSettings settings(config, QSettings::IniFormat);

    settings.beginGroup("Directorys");

    if (!bankDir.empty()) {
        std::cout << "[Config Overridden] BankDir set to: " << bankDir << "\n";
        settings.setValue("BankDir", QString::fromStdString(bankDir));
    }
    if (!wavDir.empty()) {
        std::cout << "[Config Overridden] WavDir set to: " << wavDir << "\n";
        settings.setValue("WavDir", QString::fromStdString(wavDir));
    }
    if (!rebuildDir.empty()) {
        std::cout << "[Config Overridden] RebuildDir set to: " << rebuildDir << "\n";
        settings.setValue("RebuildDir", QString::fromStdString(rebuildDir));
    }
    if (!cacheDir.empty()) {
        std::cout << "[Config Overridden] CacheDir set to: " << cacheDir << "\n";
        settings.setValue("CacheDir", QString::fromStdString(cacheDir));
    }

    // 3. Initialize default directory configurations, if empty in config.ini
    if (settings.value("BankDir").toString().isEmpty()) 
        settings.setValue("BankDir", QCoreApplication::applicationDirPath() + "/bank");
    if (settings.value("WavDir").toString().isEmpty()) 
        settings.setValue("WavDir", QCoreApplication::applicationDirPath() + "/wav");
    if (settings.value("RebuildDir").toString().isEmpty()) 
        settings.setValue("RebuildDir", QCoreApplication::applicationDirPath() + "/build");
    if (settings.value("CacheDir").toString().isEmpty()) 
        settings.setValue("CacheDir", QCoreApplication::applicationDirPath() + "/fsbcache");
    settings.endGroup();

    // 4. Inject any custom console arguments directly into the Options configuration
    settings.beginGroup("Options");
    
    if (!customFormat.empty()) {
        std::cout << "[Config Overridden] Format set to: " << customFormat << "\n";
        settings.setValue("Format", QString::fromStdString(customFormat).toLower());
    }
    if (customQuality != -1) {
        std::cout << "[Config Overridden] Quality set to: " << customQuality << "\n";
        settings.setValue("Quality", customQuality);
    }
    if (customThreads != -1) {
        std::cout << "[Config Overridden] CPU Threads set to: " << customThreads << "\n";
        settings.setValue("CPUThreads", customThreads);
    }
    
    settings.endGroup();

    // 5. Run the Worker process based on the selected mode
    if (mode == "extract") {
        std::cout << "Starting Extraction process...\n\n";
        ExtractWorker extractWorker;
        
        QObject::connect(&extractWorker, &ExtractWorker::taskFinished, [&a](QString result){
            std::cout << result.toStdString() << std::endl;
            a.quit();
        });
        
        QObject::connect(&extractWorker, &ExtractWorker::updateConsole, [](QString log){
            std::cout << log.toStdString() << std::endl;
        });

        extractWorker.extract_fsb();
        return a.exec();
    } 
    else if (mode == "rebuild") {
        std::cout << "Starting Rebuild process...\n";
        RebuildWorker rebuildWorker;

        QObject::connect(&rebuildWorker, &RebuildWorker::taskFinished, [&a](QString result){
            std::cout << result.toStdString() << std::endl;
            a.quit();
        });

        QObject::connect(&rebuildWorker, &RebuildWorker::updateConsole, [](QString log){
            std::cout << log.toStdString() << std::endl;
        });

        rebuildWorker.rebuild_bank();
        return a.exec(); 
    }

    return 0;
}
