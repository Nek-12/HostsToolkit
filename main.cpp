#include "mainwindow.h"
#include <QApplication>
#include "main.h"

void Data::update() {
    
}

int main(int argc, char *argv[]) try {
    assert(HOSTS != "");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
catch (const std::exception& exc) {
    std::cerr << exc.what() << std::endl;
    return (EXIT_FAILURE);
}
catch (...) {
    std::cerr << "Unknown error. Unable to continue." << std::endl;
    return (EXIT_FAILURE);
}