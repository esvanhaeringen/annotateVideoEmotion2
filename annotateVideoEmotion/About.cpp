#include "About.h"
#include <QIcon>
#include <qpixmap.h>
#include <qbrush.h>
#include <qgraphicspixmapitem>

using namespace std;

About::About(QWidget* parent)
	: ui(new Ui::About)
{
	setWindowIcon(QIcon(":/annotateVideoEmotion/icons/icon.ico"));
	ui->setupUi(this);

	scene = new QGraphicsScene(ui->icon);
	scene->setSceneRect(50, 0, 32, 32);
	ui->icon->setScene(scene);
	
	image = new QGraphicsPixmapItem(QPixmap(":/annotateVideoEmotion/icons/icon.png"));
	scene->addItem(image);
	ui->icon->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
	ui->icon->show();
	ui->icon->centerOn(16, 16);
	ui->icon->setStyleSheet("background: transparent");

	ui->appName->setText("annotateVideoEmotion");
	ui->version->setText("Version 2.0.0");
	ui->author->setText("Erik van Haeringen");
	ui->link->setText("<a href=\"https://github.com/esvanhaeringen/annotateVideoEmotion2\">https://github.com/esvanhaeringen/annotateVideoEmotion2\</a>");
	ui->link->setTextFormat(Qt::RichText);
	ui->link->setTextInteractionFlags(Qt::TextBrowserInteraction);
	ui->link->setOpenExternalLinks(true);
}
