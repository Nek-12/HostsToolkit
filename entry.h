#pragma once
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <cassert>
#include <sstream>
#ifdef __linux__
#define HOSTS "/etc/hosts"
#endif
#ifdef _WIN32
#define TIME_MULTIPLIER 80000/180
#define HOSTS "C:/Windows/System32/Drivers/etc/hosts"
#endif

#include <QWidget>

namespace Ui {
class Entry;
}

class Entry : public QWidget
{
    Q_OBJECT

public:
    explicit Entry(QWidget *parent = nullptr);
    ~Entry() override;

private:
    Ui::Entry *ui;
};

