#include <QApplication>
#include <QTranslator>
#include "src/app.h"
#include <fstream>

void crash(const std::string& msg = "", const std::exception& e = std::runtime_error("Unknown error")) noexcept {
    std::ofstream f("CRASH.txt");
    f << e.what() << std::endl << msg << std::endl;
}

int main(int argc, char *argv[]) try {
    if (std::string(HOSTS).empty()) throw std::runtime_error("Your OS is not supported yet.");
    QApplication a(argc, argv);
    QTranslator qtTranslator;
    qtTranslator.load(QLocale::system(), QStringLiteral("qtbase_"));
    a.installTranslator(&qtTranslator);
    App app{};
    if (QCoreApplication::arguments().size() > 1)
        a.load_file(QCoreApplication::arguments().at(1).toStdString());
    return a.exec();
}
catch (const std::exception& exc) {
    crash("Contact the developer on github: https://github.com/Nek-12/HostsTools",exc);
    return(EXIT_FAILURE);
}
catch (...) {
    crash("Contact the developer on github: https://github.com/Nek-12/HostsTools");
    return (EXIT_FAILURE);
}