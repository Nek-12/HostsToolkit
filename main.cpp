#include "mainwindow.h"
#include <QApplication>


int main(int argc, char *argv[]) try {
    if (std::string(HOSTS).empty()) throw std::runtime_error("Your OS is not supported yet.");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    if (argc > 1)
        w.load_file(argv[1]);
    return a.exec();
}
catch (const std::exception& exc) {
    std::cerr << exc.what() << std::endl
    << "Contact the developer on github: https://github.com/Nek-12/HostsToolkit" << std::endl;
    return (EXIT_FAILURE);
}
catch (...) {
    std::cerr << "Unknown error. Unable to continue." << std::endl;
    return (EXIT_FAILURE);
}