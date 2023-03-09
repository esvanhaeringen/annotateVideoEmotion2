#pragma once
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QComboBox>
#include <QDebug>
#include <QPushButton>

#include "annotateVideoEmotion.h"
#include "ui_annotateVideoEmotion.h"
#include "ExampleDialog.h"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <random>
#include <shlwapi.h>

#include "MediaInfo/MediaInfoDLL.h"
#define MediaInfoNameSpace MediaInfoDLL;
using namespace MediaInfoNameSpace;
#include <comdef.h>

namespace fs = std::filesystem;
using namespace std;

annotateVideoEmotion::annotateVideoEmotion(QWidget* parent)
    : QMainWindow(parent),
    ui(new Ui::annotateVideoEmotion)
{
    setWindowIcon(QIcon(":/annotateVideoEmotion/icons/icon.ico"));
    ui->setupUi(this);
    about = new About(this);

    //setup buttons
    ui->valNone->setVisible(false);
    ui->aroNone->setVisible(false);
    ui->labNone->setVisible(false);
    d_valBtns = { ui->v0, ui->v1, ui->v2, ui->v3, ui->v4, ui->valNone };
    d_aroBtns = { ui->a0, ui->a1, ui->a2, ui->a3, ui->a4, ui->aroNone };
    d_labBtns = { ui->l0, ui->l1, ui->l2, ui->l3, ui->l4, ui->l5, ui->l6, ui->l7, ui->labNone };
    valGroup = new QButtonGroup(this);
    aroGroup = new QButtonGroup(this);
    labGroup = new QButtonGroup(this);
    valGroup->setExclusive(true);
    for (size_t i = 0; i < d_valBtns.size(); ++i)
        valGroup->addButton(d_valBtns[i]);
    aroGroup->setExclusive(true);
    for (size_t i = 0; i < d_aroBtns.size(); ++i)
        aroGroup->addButton(d_aroBtns[i]);
    labGroup->setExclusive(true);
    for (size_t i = 0; i < d_labBtns.size(); ++i)
        labGroup->addButton(d_labBtns[i]);

    //setup full player (left)
    plFull = new QMediaPlaylist(this);
    plFull->setPlaybackMode(QMediaPlaylist::PlaybackMode::Loop);
    mpFull = new QMediaPlayer(this);
    mpFull->setVideoOutput(ui->fullPlayer);
    mpFull->setVolume(0);
    mpFull->setNotifyInterval(100);
    mpFull->setPlaylist(plFull);
    ui->fullSlider->setTickInterval(10);

    //setup sub player (right)
    plSub = new QMediaPlaylist(this);
    plSub->setPlaybackMode(QMediaPlaylist::PlaybackMode::Loop);
    mpSub = new QMediaPlayer(this);
    mpSub->setVideoOutput(ui->subPlayer);
    mpSub->setVolume(0);
    mpSub->setNotifyInterval(100);
    mpSub->setPlaylist(plSub);
    ui->subSlider->setTickInterval(10);

    //setup table
    ui->fullTable->setMinimumHeight(10);
    ui->fullTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->fullTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    updateTableHeaders(25, { "", "V", "A", "L" });

    //Menu
    connect(ui->actionQuit_2, &QAction::triggered, this, QGuiApplication::quit);
    connect(ui->actionChangeAnnotator, &QAction::triggered, this, &annotateVideoEmotion::changeAnnotator);
    connect(ui->actionLocationResult, &QAction::triggered, this, &annotateVideoEmotion::locationResult);
    connect(ui->actionPrevious, &QAction::triggered, this, &annotateVideoEmotion::previous);
    connect(ui->actionNext_3, &QAction::triggered, this, &annotateVideoEmotion::next);
    connect(ui->actionAbout, &QAction::triggered, about, &About::show);

    //Buttons
    connect(ui->changeAnnotator, &QPushButton::clicked, this, &annotateVideoEmotion::changeAnnotator);
    connect(ui->locationResult, &QPushButton::clicked, this, &annotateVideoEmotion::locationResult);
    connect(ui->valAroReference, &QPushButton::clicked, this, &annotateVideoEmotion::valAroReference);
    connect(ui->labelReference, &QPushButton::clicked, this, &annotateVideoEmotion::labelReference);
    connect(ui->previous, &QPushButton::clicked, this, &annotateVideoEmotion::previous);
    connect(ui->next, &QPushButton::clicked, this, &annotateVideoEmotion::next);
    connect(ui->copyPrevious, &QPushButton::clicked, this, &annotateVideoEmotion::copyPrevious);
    connect(ui->fullPlayPause, &QPushButton::clicked, this, &annotateVideoEmotion::fullPlayPause);
    connect(ui->subPlayPause, &QPushButton::clicked, this, &annotateVideoEmotion::subPlayPause);

    //Video
    connect(ui->fullSpeed, &QComboBox::currentTextChanged, this, &annotateVideoEmotion::videoSpeed);
    connect(ui->subSpeed, &QComboBox::currentTextChanged, this, &annotateVideoEmotion::videoSpeed);
    connect(mpFull, &QMediaPlayer::durationChanged, this, &annotateVideoEmotion::fullDurationChanged);
    connect(mpFull, &QMediaPlayer::positionChanged, this, &annotateVideoEmotion::fullPositionChanged);
    connect(mpSub, &QMediaPlayer::durationChanged, this, &annotateVideoEmotion::subDurationChanged);
    connect(mpSub, &QMediaPlayer::positionChanged, this, &annotateVideoEmotion::subPositionChanged);

    //textedit
    connect(ui->subPosEdit, &QLineEdit::textChanged, this, &annotateVideoEmotion::subMoveEdit);
    connect(ui->totalPosEdit, &QLineEdit::textChanged, this, &annotateVideoEmotion::totalMoveEdit);

    //slider
    connect(ui->fullSlider, &QSlider::sliderPressed, this, &annotateVideoEmotion::sliderPressed);
    connect(ui->fullSlider, &QSlider::sliderReleased, this, &annotateVideoEmotion::sliderReleased);
    connect(ui->fullSlider, &QSlider::valueChanged, this, &annotateVideoEmotion::sliderValueChanged);
   
    //table
    connect(ui->fullTable->horizontalHeader(), &QHeaderView::sectionResized, this, &annotateVideoEmotion::resizeTable);
    setState();
}

void annotateVideoEmotion::resizeTable()
{
    const QSizeF _minSize = ui->fullSlider->minimumSizeHint();
    ui->fullSliderSpacer->changeSize(float(ui->fullTable->horizontalHeader()->width()) / float(ui->fullTable->model()->columnCount()) - (float(_minSize.width()) - 1.f) / 2.f, 5);
    ui->fullControls->invalidate();
    ui->tableContainer->setContentsMargins(QMargins(0, 0, float(_minSize.width() - 1) / 2, 0));
    ui->subSliderLabelContainer->setContentsMargins(QMargins(float(_minSize.width() - 1) / 2, 0, float(_minSize.width() - 1) / 2, 0));
}

void annotateVideoEmotion::subMoveEdit()
{
    
    if (ui->subPosEdit->text() != "")
    {
        int pos = stoi(ui->subPosEdit->text().toStdString());
        if (pos <= 0)
            return;
        if (d_current - d_step[d_current] + pos - 1 <= d_prog && pos <= d_nSteps[d_current])
            move(d_current - d_step[d_current] + pos - 1);
        else
        {
            QMessageBox messageBox;
            messageBox.information(0, "Input not allowed", QStringLiteral("You can only move to a subclip between 1 and %1 that is already scored or next in line to be scored").arg(d_nSteps[d_current]));
            ui->subPosEdit->setText(QString::number(d_step[d_current] + 1));
        }
    }
}

void annotateVideoEmotion::totalMoveEdit()
{
    if (ui->totalPosEdit->text() != "")
    {
        int pos = stoi(ui->totalPosEdit->text().toStdString());
        if (pos <= 0)
            return;
        if (pos - 1 <= d_prog)
            move(pos - 1);
        else
        {
            QMessageBox messageBox;
            messageBox.information(0, "Input not allowed", QStringLiteral("You can only move to a clip between 1 and %1 that is already scored or next in line to be scored").arg(d_nSteps.size()));
            ui->totalPosEdit->setText(QString::number(d_current + 1));
        }
    }
}

void annotateVideoEmotion::fullPlayPause()
{
    if (fullWasPlaying)
    {
        mpFull->pause();
        fullWasPlaying = false;
        ui->fullPlayPause->setText("Play");
    }
    else
    {
        if (subWasPlaying)
        {
            mpSub->pause();
            subWasPlaying = false;
            ui->subPlayPause->setText("Play");
        }
        mpFull->play();
        fullWasPlaying = true;
        ui->fullPlayPause->setText("Pause");
    }
}

void annotateVideoEmotion::subPlayPause()
{
    if (subWasPlaying)
    {
        mpSub->pause();
        subWasPlaying = false;
        ui->subPlayPause->setText("Play");
    }
    else
    {
        if (fullWasPlaying)
        {
            mpFull->pause();
            fullWasPlaying = false;
            ui->fullPlayPause->setText("Play");
        }
        mpSub->play();
        subWasPlaying = true;
        ui->subPlayPause->setText("Pause");
    }
}

void annotateVideoEmotion::subDurationChanged(float dur)
{
    ui->subSlider->setRange(0, dur / 100);
    ui->subSlider->setTickInterval(10);
}

void annotateVideoEmotion::fullDurationChanged(float dur)
{
    ui->fullSlider->setRange(0, dur / 100);
    ui->fullSlider->setTickInterval(10);
}

void annotateVideoEmotion::subPositionChanged(float pos)
{
    ui->subSlider->blockSignals(true);
    if (pos / 1000 > d_step[d_current] + 1 or pos / 1000 < d_step[d_current])
    {
        mpSub->setPosition(d_step[d_current] * 1000);
        ui->subSlider->setValue(d_step[d_current] * 10);
    }
    else
        ui->subSlider->setValue(pos / 100);
    ui->subSlider->blockSignals(false);
}

void annotateVideoEmotion::fullPositionChanged(float pos)
{
    if (!sliderWasPressed)
    {
        ui->fullSlider->blockSignals(true);
        ui->fullSlider->setValue(pos / 100);
        ui->fullSlider->blockSignals(false);
    }
}

void annotateVideoEmotion::sliderValueChanged()
{
    if (sliderWasPressed)
        if (mpFull->isSeekable())
            mpFull->setPosition(ui->fullSlider->value() * 100);
}

void annotateVideoEmotion::sliderPressed()
{
    if (mpFull->state() == mpFull->PlayingState)
    {
        fullWasPlaying = true;
        mpFull->pause();
    }
    else
        fullWasPlaying = false;
    sliderWasPressed = true;
}

void annotateVideoEmotion::sliderReleased()
{
    if (fullWasPlaying)
        mpFull->play();
    fullWasPlaying = false;
    sliderWasPressed = false;
}

annotateVideoEmotion::~annotateVideoEmotion()
{
    delete ui;
    deletePlayer();
}

void annotateVideoEmotion::deletePlayer()
{
    plFull->~QMediaPlaylist();
    mpFull->~QMediaPlayer();
    plSub->~QMediaPlaylist();
    mpSub->~QMediaPlayer();
}

void annotateVideoEmotion::locationResult()
{
    ShellExecute(NULL, NULL, d_sourcePath.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

void annotateVideoEmotion::videoSpeed()
{
    float speed[] = {1, 0.75, 0.5, 0.25, 0.1};
    mpFull->setPlaybackRate(speed[ui->fullSpeed->currentIndex()]);
    mpSub->setPlaybackRate(speed[ui->subSpeed->currentIndex()]);
}

void annotateVideoEmotion::subPlay()
{
    mpFull->pause();
    fullWasPlaying = false;
    ui->fullPlayPause->setText("Play");
    mpSub->play();
    subWasPlaying = true;
    ui->subPlayPause->setText("Pause");
}

void annotateVideoEmotion::changeAnnotator()
{
    bool ok;
    QInputDialog inputDialog;
    inputDialog.setInputMode(QInputDialog::TextInput);
    inputDialog.setWindowTitle("Set annotator");
    inputDialog.setLabelText("Enter the name of the annotator");
    QFont font;
    font.setPointSize(10);
    inputDialog.setFont(font);
    ok = inputDialog.exec();
    d_annotator = inputDialog.textValue().toStdString();
    if (!ok || d_annotator.empty())
        return; //stop when canceled or no name
    else
        reset();
    fs::path fileName("annotations_" + d_annotator + ".csv");
    if (fileExists(d_sourcePath/fileName))
    {
        std::ifstream file(d_sourcePath/fileName);
        int idx, step, nSteps, val, aro, lab;
        string clip;
        while (file >> idx >> clip >> step >> nSteps >> val >> aro >> lab)
        {
            d_idc.push_back(idx);
            d_clip.push_back(clip);
            d_step.push_back(step);
            d_nSteps.push_back(nSteps);
            d_val.push_back(val);
            d_aro.push_back(aro);
            d_lab.push_back(lab);
        }
        updateStats();
        //set to first not fully rated clip, otherwise last clip
        int const lastVal = std::distance(d_val.begin(), std::find(d_val.begin(), 
            d_val.end(), -1)); 
        int const lastAro = std::distance(d_aro.begin(), std::find(d_aro.begin(),
            d_aro.end(), -1));
        int const lastLab = std::distance(d_lab.begin(), std::find(d_lab.begin(),
            d_lab.end(), -1));
        d_current = min(min(lastVal, lastAro), lastLab);
        
        if (d_current == d_total)
        {
            QMessageBox messageBox;
            messageBox.information(0, "Complete", "All annotations are already complete for this user. To change ratings, adjust the settings for a clip and press previous or next.");
            d_current = 0;
        }
        updateTable();
        setSelection();
        loadVideo(QString::fromStdString((d_sourcePath / d_clip[d_current]).string()));
        subPlay();
    }
    else 
    {
        if (!fs::is_directory(fs::status(d_sourcePath))) //create source directory if it does not exist
            fs::create_directory(d_sourcePath);
        
        //find all avi files in source directory (not recursively)
        for (const auto& entry : std::filesystem::directory_iterator(d_sourcePath)) {
            if (entry.path().extension() == ".avi")
            {
                MediaInfo MI;
                MI.Open(String(entry.path().c_str()));
                int h, m, s, ms = 0;
                float duration;
                _bstr_t b(MI.Get(Stream_General, 0, __T("Duration/String3")).c_str());
                if (sscanf(b, "%d:%d:%d.%d", &h, &m, &s, &ms) >= 3)
                    duration = h * 3600 + m * 60 + s + float(ms)/1000;
                MI.Close();
                d_file.push_back(entry.path().filename().string());
                d_dur.push_back(int(duration));
            }
        }
        //shuffles clips with a seed based on the system time.
        sort(d_file.begin(), d_file.end());
        auto rng = std::default_random_engine{};
        int curTime = time(0);
        rng.seed(curTime);
        shuffle(begin(d_file), end(d_file), rng);
        
        int counter = 0;
        for (int idx = 0; idx < d_file.size(); ++idx)
            for (int step = 0; step < d_dur[idx]; ++step)
            {
                d_idc.push_back(counter);
                d_clip.push_back(d_file[idx]);
                d_step.push_back(step);
                d_nSteps.push_back(d_dur[idx]);
                d_val.push_back(-1);
                d_aro.push_back(-1);
                d_lab.push_back(-1);
                counter += 1;
            }

        std::ofstream file(d_sourcePath / fileName);
        for (int idx = 0; idx < d_clip.size(); ++idx)
            file << d_idc[idx] << ' ' << d_clip[idx] << ' ' << d_step[idx] << ' ' << d_nSteps[idx] << ' ' << d_val[idx] <<
            ' ' << d_aro[idx] << ' ' << d_lab[idx] << endl;
        d_current = 0;
        updateTable();
        updateStats();
        setSelection();
        loadVideo(QString::fromStdString((d_sourcePath / d_clip[d_current]).string()));
        subPlay();
    }

    d_ready = true;
    setState();
}

void annotateVideoEmotion::valAroReference()
{
    ExampleDialog* dialog = new ExampleDialog(        
        selectedValence() != -1 ? selectedValence() : 2,
        selectedArousal() != -1 ? selectedArousal() : 2,
        6);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void annotateVideoEmotion::labelReference()
{
    ExampleDialog* dialog = new ExampleDialog(
        selectedLabel() != -1 ? selectedLabel() : 0, 6);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void annotateVideoEmotion::reset()
{
    d_idc.clear();
    d_clip.clear();
    d_step.clear();
    d_nSteps.clear();
    d_val.clear();
    d_aro.clear();
    d_lab.clear();
    d_file.clear();
    d_dur.clear();
}

bool annotateVideoEmotion::fileExists(const fs::path name) 
{
    ifstream f(name.c_str());
    return f.good();
}

void annotateVideoEmotion::updateTable()
{
    updateTableHeaders(d_nSteps[d_current], { "", "V", "A", "L" });
    int value = 0;
    for (int idx = 0; idx < d_nSteps[d_current]; ++idx)
    {
        //VALENCE
        QTableWidgetItem* item = ui->fullTable->item(1, idx + 1);
        if (!item) {
            item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignCenter);
            ui->fullTable->setItem(1, idx + 1, item);
        }
        value = d_val[d_current - d_step[d_current] + idx];
        item->setText(QString::number(value));
        
        if (idx == d_step[d_current])
            item->setBackground(QColor("#a9fbff"));
        else
            if (value == -1)
                item->setBackground(Qt::gray);
            else
                item->setBackground(QColor(d_valCol[value]));
        //AROUSAL
        item = ui->fullTable->item(2, idx + 1);
        if (!item) {
            item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignCenter);
            ui->fullTable->setItem(2, idx + 1, item);
        }
        value = d_aro[d_current - d_step[d_current] + idx];
        item->setText(QString::number(value));

        if (idx == d_step[d_current])
            item->setBackground(QColor("#a9fbff"));
        else
            if (value == -1)
                item->setBackground(Qt::gray);
            else
                item->setBackground(QColor(d_aroCol[value]));
        //LABEL
        item = ui->fullTable->item(3, idx + 1);
        if (!item) {
            item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignCenter);
            ui->fullTable->setItem(3, idx + 1, item);
        }
        value = d_lab[d_current - d_step[d_current] + idx];
        item->setText(QString::number(value));

        if (idx == d_step[d_current])
            item->setBackground(QColor("#a9fbff"));
        else
            if (value == -1)
                item->setBackground(Qt::gray);
            else
                item->setBackground(QColor(d_labCol[value]));
    }
}

void annotateVideoEmotion::updateTableHeaders(int col, QStringList row)
{
    ui->fullTable->setColumnCount(col + 1);
    for (int idx = 1; idx < col + 1; ++idx)
    {
        QTableWidgetItem* item = ui->fullTable->item(0, idx);
        if (!item) {
            item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignCenter);
            ui->fullTable->setItem(0, idx, item);
        }
        item->setText(QString::number(idx));
        if (d_ready)
        {
            if (idx - 1 == d_step[d_current])
                item->setBackground(Qt::cyan);
            else
                item->setBackground(QColor(243, 239, 229));
        }
        else
        {
            item->setBackground(QColor(243, 239, 229));
        }
    }
    ui->fullTable->setRowCount(row.size());
    for (int idx = 0; idx < row.size(); ++idx)
    {
        QTableWidgetItem* item = ui->fullTable->item(idx, 0);
        if (!item) {
            item = new QTableWidgetItem();
            ui->fullTable->setItem(idx, 0, item);
            item->setBackground(QColor(243, 239, 229));
        }
        item->setText(row[idx]);
    }
    resizeTable();
    updateSubSliderLabels(col);
}


void annotateVideoEmotion::previous()
{
    saveResult();
    move(d_current - 1);
}

void annotateVideoEmotion::next()
{
    if (selectedValence() == -1 || selectedArousal() == -1 || selectedLabel() == -1)
    {
        QMessageBox messageBox;
        messageBox.critical(0, "Error", "Please select a score for valence and arousal and pick an emotion label.");
        return;
    }
    if (!saveResult())
        return;
    if (d_current == d_total - 1)
    {
        QMessageBox messageBox;
        fs::path fileName("annotations_" + d_annotator + ".csv.");
        messageBox.information(0, "Done!", QStringLiteral("The annotations are complete. They are saved in '%1'. Click 'Open source location' to open the folder of the annotations.").arg((d_sourcePath/fileName).c_str()));
        return;
    }
    move(d_current + 1);
}

void annotateVideoEmotion::copyPrevious()
{
    d_valBtns[d_val[d_current - 1]]->click();
    d_aroBtns[d_aro[d_current - 1]]->click();
    d_labBtns[d_lab[d_current - 1]]->click();
}

void annotateVideoEmotion::move(int pos)
{
    fs::path oldClip = d_sourcePath / d_clip[d_current];
    d_current = pos;
    updateStats();
    updateTable();
    setSelection();
    if (oldClip != d_sourcePath / d_clip[d_current])
        loadVideo(QString::fromStdString((d_sourcePath / d_clip[d_current]).string()));
    subPlay();
    setState();
}

bool annotateVideoEmotion::saveResult()
{
    d_val[d_current] = selectedValence();
    d_aro[d_current] = selectedArousal();
    d_lab[d_current] = selectedLabel();
    return writeFile();
}

int annotateVideoEmotion::selectedValence()
{
    vector<QPushButton*>::iterator valItr = std::find(d_valBtns.begin(), d_valBtns.end(), valGroup->checkedButton());
    if (valItr == d_valBtns.cend() || *valItr == d_valBtns.back())
        return -1;
    else
        return std::distance(d_valBtns.begin(), valItr);
}

int annotateVideoEmotion::selectedArousal()
{
    vector<QPushButton*>::iterator aroItr = std::find(d_aroBtns.begin(), d_aroBtns.end(), aroGroup->checkedButton());
    if (aroItr == d_aroBtns.cend() || *aroItr == d_aroBtns.back())
        return -1;
    else
        return std::distance(d_aroBtns.begin(), aroItr);
}

int annotateVideoEmotion::selectedLabel()
{
    vector<QPushButton*>::iterator labItr = std::find(d_labBtns.begin(), d_labBtns.end(), labGroup->checkedButton());
    if (labItr == d_labBtns.cend() || *labItr == d_labBtns.back())
        return -1;
    else
        return std::distance(d_labBtns.begin(), labItr);
}

bool annotateVideoEmotion::writeFile()
{
    try
    {
        fs::path tmpName(".annotations_" + d_annotator + ".csv");
        ofstream fileout(d_sourcePath / tmpName); //tmp file
        for (int idx = 0; idx < d_total; ++idx)
        {
            fileout << d_idc[idx] << ' ' << d_clip[idx] << ' ' << d_step[idx] << ' ' << d_nSteps[idx] << ' ' << d_val[idx] <<
                ' ' << d_aro[idx] << ' ' << d_lab[idx] << endl;
        }
        fileout.close();
        fs::path fileName("annotations_" + d_annotator + ".csv");
        fs::rename(d_sourcePath / tmpName, d_sourcePath / fileName); //overwrite file with tmp
        return true;
    }
    catch (const exception e)
    {
        qDebug() << e.what();
        QMessageBox::StandardButton reply;
        reply = QMessageBox::critical(this, "Error", "An error occured in writing the result to the annotation file. Make sure it is not in use by another application. Do you want to try again?", QMessageBox::Yes|QMessageBox::No);
        bool success = false;
        if (reply == QMessageBox::Yes)
            success = writeFile();
        return success;
    }
}

void annotateVideoEmotion::updateStats()
{
    d_total = d_idc.size();
    d_prog = d_total - count(d_val.begin(), d_val.end(), -1);
}


void annotateVideoEmotion::updateSubSliderLabels(int col)
{
    for (int idx = 0; idx < d_subSliderLabels.size(); ++idx)
    {
        ui->subSliderLabelContainer->removeWidget(d_subSliderLabels[idx]);
        delete d_subSliderLabels[idx];
    }
    d_subSliderLabels.clear();
    for (int idx = 0; idx < col; ++idx)
    {
        if (idx >= d_subSliderLabels.size())
        {
            d_subSliderLabels.push_back(new QLabel(QString::number(idx + 1), this));
            d_subSliderLabels[idx]->setAlignment(Qt::AlignCenter);
            ui->subSliderLabelContainer->addWidget(d_subSliderLabels[idx]);
        }
        if (d_ready)
        {
            if (idx == d_step[d_current])
                d_subSliderLabels[idx]->setStyleSheet("background-color:rgb(0,255,255); font-weight:bold;"); //
            else
                d_subSliderLabels[idx]->setStyleSheet("background-color:transparent; font-weight:normal;");
        }
        else
            d_subSliderLabels[idx]->setStyleSheet("background-color:transparent;");
    }
}

void annotateVideoEmotion::loadVideo(QString url)
{
    plFull->clear();
    plFull->addMedia(QUrl::fromLocalFile(url));
    mpFull->play();
    mpFull->pause();
    plSub->clear();
    plSub->addMedia(QUrl::fromLocalFile(url));
    mpFull->play();
    mpFull->pause();
}

void annotateVideoEmotion::setSelection()
{
    (d_val[d_current] == -1) ? d_valBtns[5]->click() : d_valBtns[d_val[d_current]]->click();
    (d_aro[d_current] == -1) ? d_aroBtns[5]->click() : d_aroBtns[d_aro[d_current]]->click();
    (d_lab[d_current] == -1) ? d_labBtns[8]->click() : d_labBtns[d_lab[d_current]]->click();
}

void annotateVideoEmotion::setState()
{
    if (d_ready)
    {
        //actions
        ui->actionChangeAnnotator->setText("Change annotator");
        ui->actionLocationResult->setEnabled(true);

        //buttons
        for (size_t i = 0; i < d_valBtns.size(); ++i)
            d_valBtns[i]->setEnabled(true);
        for (size_t i = 0; i < d_aroBtns.size(); ++i)
            d_aroBtns[i]->setEnabled(true);
        for (size_t i = 0; i < d_labBtns.size(); ++i)
            d_labBtns[i]->setEnabled(true);
        ui->changeAnnotator->setText("Change annotator");
        ui->locationResult->setEnabled(true);
        ui->valAroReference->setEnabled(true);
        ui->labelReference->setEnabled(true);
        if (d_current == 0)
        {
            ui->previous->setEnabled(false);
            ui->copyPrevious->setEnabled(false);
            ui->actionPrevious->setEnabled(false);
        } else
        {
            ui->previous->setEnabled(true);
            ui->copyPrevious->setEnabled(true);
            ui->actionPrevious->setEnabled(true);
        }
        ui->next->setEnabled(true);
        ui->actionNext_3->setEnabled(true);
        if (d_current == d_total - 1)
        {
            ui->next->setText("Save");
            ui->actionNext_3->setText("Save");
        } else
        {
            ui->next->setText("Next");
            ui->actionNext_3->setText("Next");
        }
        ui->fullPlayPause->setEnabled(true);
        ui->subPlayPause->setEnabled(true);
        if (fullWasPlaying)
            ui->fullPlayPause->setText("Pause");
        else
            ui->fullPlayPause->setText("Play");
        if (subWasPlaying)
            ui->subPlayPause->setText("Pause");
        else
            ui->subPlayPause->setText("Play");

        //sliders
        ui->fullSlider->setEnabled(true);
        ui->subSlider->setEnabled(true);

        //table
        ui->fullTable->setEnabled(true);

        //combobox
        ui->fullSpeed->setEnabled(true);
        ui->subSpeed->setEnabled(true);

        //textedit
        ui->subPosEdit->setEnabled(true);
        ui->subPosEdit->setText(QString::number(d_step[d_current] + 1));
        ui->subPosEdit->setValidator(new QIntValidator(1, d_nSteps[d_current], this));
        ui->totalPosEdit->setEnabled(true);
        ui->totalPosEdit->setText(QString::number(d_current + 1));
        ui->totalPosEdit->setValidator(new QIntValidator(1, d_nSteps.size(), this));

        //labels
        ui->progress->setText(QStringLiteral("%1/%2 (%3%)").arg(d_prog).arg(d_total).arg(int((float(d_prog)/float(d_total))*100)));
        ui->subPosLabel->setText(QStringLiteral("of %1 subclips for the current video, ").arg(d_nSteps[d_current]));
        ui->totalPosLabel->setText(QStringLiteral("of %1 total subclips").arg(d_nSteps.size()));
        ui->annotator->setText(QString::fromStdString(d_annotator));
        ui->lb_progress->setEnabled(true);
        ui->lb_currentClip->setEnabled(true);
        ui->lb_Annotator->setEnabled(true);
        ui->progress->setEnabled(true);
        ui->subPosLabel->setEnabled(true);
        ui->totalPosLabel->setEnabled(true);
        ui->annotator->setEnabled(true);
        ui->fullClipLabel->setEnabled(true);
        ui->subClipLabel->setEnabled(true);
        ui->lb_arousal->setEnabled(true);
        ui->lb_valence->setEnabled(true);
        ui->lb_catEmot->setEnabled(true);
        for (int idx = 0; idx < d_subSliderLabels.size(); ++idx)
            d_subSliderLabels[idx]->setEnabled(true);
    }
    else 
    {
        //actions
        ui->actionChangeAnnotator->setText("Set annotator");
        ui->actionLocationResult->setEnabled(false);
        ui->actionPrevious->setEnabled(false);
        ui->actionNext_3->setEnabled(false);

        //buttons
        for (size_t i = 0; i < d_valBtns.size(); ++i)
            d_valBtns[i]->setEnabled(false);
        for (size_t i = 0; i < d_aroBtns.size(); ++i)
            d_aroBtns[i]->setEnabled(false);
        for (size_t i = 0; i < d_labBtns.size(); ++i)
            d_labBtns[i]->setEnabled(false);
        ui->changeAnnotator->setText("Set annotator");
        ui->locationResult->setEnabled(false);
        ui->valAroReference->setEnabled(false);
        ui->labelReference->setEnabled(false);
        ui->previous->setEnabled(false);
        ui->copyPrevious->setEnabled(false);
        ui->next->setEnabled(false);
        ui->fullPlayPause->setText("Play");
        ui->subPlayPause->setText("Play");
        ui->fullPlayPause->setEnabled(false);
        ui->subPlayPause->setEnabled(false);

        //sliders
        ui->fullSlider->setEnabled(false);
        ui->subSlider->setEnabled(false);

        //table
        ui->fullTable->setEnabled(false);

        //combobox
        ui->fullSpeed->setEnabled(false);
        ui->subSpeed->setEnabled(false);

        //textedit
        ui->subPosEdit->setEnabled(false);
        ui->totalPosEdit->setEnabled(false);
        ui->subPosEdit->setText("NA");
        ui->totalPosEdit->setText("NA");

        //labels
        ui->progress->setText("-");
        ui->annotator->setText("-");
        ui->subPosLabel->setText("of NA subclips for the current video, ");
        ui->totalPosLabel->setText("of NA total subclips");
        ui->lb_progress->setEnabled(false);
        ui->lb_currentClip->setEnabled(false);
        ui->lb_Annotator->setEnabled(false);
        ui->progress->setEnabled(false);
        ui->subPosLabel->setEnabled(false);
        ui->totalPosLabel->setEnabled(false);
        ui->annotator->setEnabled(false);
        ui->fullClipLabel->setEnabled(false);
        ui->subClipLabel->setEnabled(false);
        ui->lb_arousal->setEnabled(false);
        ui->lb_valence->setEnabled(false);
        ui->lb_catEmot->setEnabled(false);
        for (int idx = 0; idx < d_subSliderLabels.size(); ++idx)
            d_subSliderLabels[idx]->setEnabled(false);
    }
}