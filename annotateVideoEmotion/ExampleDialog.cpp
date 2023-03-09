#include "ExampleDialog.h"
#include <string>
#include <sstream>
#include <filesystem>
#include <QComboBox>

using namespace std;
namespace fs = filesystem;

ExampleDialog::ExampleDialog(int valence, int arousal, int maxExamples, QWidget* parent)
    :ui(new Ui::exampleDialog)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    d_mode = 0;
    ui->setupUi(this);
    setVisibility();
    setWindowTitle(QStringLiteral("Example clips of Valence: %1 | Arousal: %2").arg(d_valLabels[valence]).arg(d_aroLabels[arousal]));  
    ui->arousalCB->setCurrentIndex(arousal);
    ui->valenceCB->setCurrentIndex(valence);

    connect(ui->arousalCB, &QComboBox::currentTextChanged, this, &ExampleDialog::loadVideos);
    connect(ui->valenceCB, &QComboBox::currentTextChanged, this, &ExampleDialog::loadVideos);

    makePlayers(maxExamples);
    loadVideos();
    QApplication::restoreOverrideCursor();
}

ExampleDialog::ExampleDialog(int label, int maxExamples, QWidget* parent)
    :ui(new Ui::exampleDialog)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    d_mode = 1;
    ui->setupUi(this);
    setVisibility();
    setWindowTitle(QStringLiteral("Example clips of %1").arg(d_emotLabels[label]));
    d_labGroup = new QButtonGroup(this);
    d_labGroup->setExclusive(true);
    for (size_t i = 0; i < d_labBtns.size(); ++i)
    {
        d_labGroup->addButton(d_labBtns[i]);
        connect(d_labBtns[i], &QPushButton::clicked, this, &ExampleDialog::loadVideos);
    }
    
    makePlayers(maxExamples);
    d_labBtns[label]->click();
    loadVideos();
    QApplication::restoreOverrideCursor();
}

ExampleDialog::~ExampleDialog()
{
    delete ui;
    deleteVideos();
}

void ExampleDialog::setVisibility()
{
    ui->labelLb->setVisible(d_mode == 1);
    d_labBtns = { ui->l0, ui->l1, ui->l2, ui->l3, ui->l4, ui->l5, ui->l6, ui->l7 };
    for (size_t i = 0; i < d_labBtns.size(); ++i)
        d_labBtns[i]->setVisible(d_mode == 1);
    ui->arousalLb->setVisible(d_mode == 0);
    ui->arousalCB->setVisible(d_mode == 0);
    ui->valenceLb->setVisible(d_mode == 0);
    ui->valenceCB->setVisible(d_mode == 0);
}

void ExampleDialog::deleteVideos()
{
    for (int idx = 0; idx < vw.size(); ++idx)
    {
        vw[idx]->~QVideoWidget();
        pl[idx]->~QMediaPlaylist();
        mp[idx]->~QMediaPlayer();
    }
    vw.clear();
    pl.clear();
    mp.clear();
}

void ExampleDialog::makePlayers(int numberOfPlayers)
{
    for (int idx = 0; idx < numberOfPlayers; ++idx)
    {
        vw.push_back(new QVideoWidget(this));
        ui->gridLayout->addWidget(vw[idx], int(idx / 3), idx % 3);
        pl.push_back(new QMediaPlaylist(this));
        pl[idx]->setPlaybackMode(QMediaPlaylist::PlaybackMode::Loop);
        mp.push_back(new QMediaPlayer(this));
        mp[idx]->setVideoOutput(vw[idx]);
        mp[idx]->setVolume(0);
        mp[idx]->setPlaylist(pl[idx]);
    }
}

void ExampleDialog::loadVideos()
{
    
    //allowInput(false);
    for (int idx = 0; idx < pl.size(); ++idx)
        pl[idx]->clear();

    stringstream folder;
    if (d_mode == 0)
    {
        setWindowTitle(QStringLiteral("Example clips of Valence: %1 | Arousal: %2").arg(d_valLabels[ui->valenceCB->currentIndex()]).arg(d_aroLabels[ui->arousalCB->currentIndex()]));
        folder << "A" << ui->arousalCB->currentIndex() << "V" << ui->valenceCB->currentIndex();
    }
    else
    {
        int selected = selectedLabel();
        setWindowTitle(QStringLiteral("Example clips of %1").arg(d_emotLabels[selected]));
        folder << d_emotLabels[selected];
    }
    int vidCounter = 0;
    for (const auto& entry : fs::directory_iterator(d_examplesPath / folder.str()))
    {
        if (vidCounter < vw.size() && entry.path().extension() == ".avi")
        {  
            try
            {
                pl[vidCounter]->addMedia(QUrl::fromLocalFile(QString::fromStdString(entry.path().string())));
                mp[vidCounter]->play();
                vidCounter += 1;
            }
            catch (int e)
            {
                qDebug() << "Error " << e << " loading " << QString::fromStdString(entry.path().string());
            }
        }
    }
    //allowInput(true);
}

int ExampleDialog::selectedLabel()
{
    vector<QPushButton*>::iterator labItr = std::find(d_labBtns.begin(), d_labBtns.end(), d_labGroup->checkedButton());
    if (labItr == d_labBtns.cend())
        return 0;
    else
        return std::distance(d_labBtns.begin(), labItr);
}

void ExampleDialog::allowInput(bool val)
{
    if (val)
        QApplication::restoreOverrideCursor(); 
    else
        QApplication::setOverrideCursor(Qt::WaitCursor);
    if (d_mode == 0)
    {
        ui->arousalCB->setEnabled(val);
        ui->valenceCB->setEnabled(val);
    }
    //else if (d_mode == 1)
    //    for (size_t i = 0; i < d_labBtns.size(); ++i)
    //        d_labBtns[i]->setEnabled(val);
}