#include <QApplication>
#include <QDebug>
#include <QMainWindow>
#include <QMessageBox>
#include <QTranslator>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QListWidgetItem>
#include <cassert>
#include <fstream>
#include <map>
#include <qobject.h>
#include <qobjectdefs.h>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#ifdef __linux__
#define HOSTS "/etc/hosts"
#endif
#ifdef _WIN32
#define TIME_MULTIPLIER 400
#define HOSTS "C:/Windows/System32/drivers/etc/hosts"
#endif
#define VERSION "3.0.0"
#define CONFIG_FNAME "custom.txt"
#define FILEERRORMSG                                                           \
  "Couldn't write your file! Select another location or launch the app with "  \
  "admin privileges"
