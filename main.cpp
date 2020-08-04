#include <QApplication>
#include <QTranslator>
#include "app.h"


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
    qDebug() << "Instantiating app\n";
    App app{};
    qDebug() << "Checking command line args\n";
    if (QCoreApplication::arguments().size() > 1)
        app.add_file(QCoreApplication::arguments().at(1));
    app.show();
    qDebug() << "App window shown\n";
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
