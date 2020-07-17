#include "entry.h"
#include "ui_entry.h"

Entry::Entry(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Entry)
{
    ui->setupUi(this);
}

Entry::~Entry()
{
    delete ui;
}
