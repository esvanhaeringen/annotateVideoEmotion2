#pragma once
#include <ui_about.h>
#include <QDialog>
#include <string>

namespace Ui {
	class About;
}

class About: public QDialog
{
	Q_OBJECT
	
public:
	About(QWidget* parent = 0);

private:
	Ui::About* ui;
	QGraphicsScene* scene;
	QGraphicsPixmapItem* image;
};

