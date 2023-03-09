#pragma once
#include <QDialog>
#include <QCoreApplication>
#include <QPushButton>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <ui_exampleDialog.h>
#include <filesystem>
#include <QtGui>
#include <QtWidgets>
#include <vector>
#include <QVideoWidget>

namespace Ui {
	class exampleDialog;
}

class Overlay;

class ExampleDialog : public QDialog
{
    Q_OBJECT
    int d_mode;
	std::filesystem::path d_examplesPath = std::filesystem::current_path() / "examples";
    Ui::exampleDialog* ui;
    std::vector<QVideoWidget*> vw;
    std::vector<QMediaPlaylist*> pl;
    std::vector<QMediaPlayer*> mp;

    const char* d_valLabels[5] = { "Very negative", "Slightly negative", "Neutral", "Slightly positive", "Very positive" };
    const char* d_aroLabels[5] = { "Very passive", "Slightly passive", "Neutral", "Slightly active", "Very active" };
    const char* d_emotLabels[8] = { "Neutral", "Anger", "Disgust", "Enjoyment", "Fear", "Sadness", "Surprise", "Unclear" };
    std::vector <QPushButton*> d_labBtns;
    QButtonGroup* d_labGroup;
    char const* d_labCol[8] = { "#dedede", "#ffc7c7", "#b8abff", "#b6fdb1", "#ffc675", "#ffc1e6", "#e7ff7d",  "#f4f4f4" };


public:
    ExampleDialog(int valence, int arousal, int maxExamples, QWidget* parent = 0);
    ExampleDialog(int label, int maxExamples, QWidget* parent = 0);
	~ExampleDialog();

private:
    void makePlayers(int numberOfPlayers);
    void deleteVideos();
    void loadVideos();
    int selectedLabel();
    void setVisibility();
    void allowInput(bool val);
};