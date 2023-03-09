#ifndef annotateVideoEmotion_H_
#define annotateVideoEmotion_H_

#include <QMainWindow>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QPushButton>
#include <string>
#include <filesystem>
#include <vector>
#include "About.h"

namespace Ui {
    class annotateVideoEmotion;
}

class VlcInstance;
class VlcMedia;
class VlcMediaPlayer;

class EqualizerDialog;

class annotateVideoEmotion : public QMainWindow
{
    Q_OBJECT
    About* about;
    bool d_ready = false;
    std::string d_annotator;
    std::filesystem::path d_sourcePath = std::filesystem::current_path()/"source";
    std::vector<std::string> d_file;
    std::vector<int> d_dur;
    std::vector<int> d_idc;
    std::vector<std::string> d_clip;
    std::vector<int> d_step;
    std::vector<int> d_nSteps;
    std::vector<int> d_val; 
    std::vector<int> d_aro;
    std::vector<int> d_lab;
    std::vector<QLabel*> d_subSliderLabels;

    int d_total = 0;
    int d_prog = 0;
    int d_current = 0;
    char const* d_descVal[5] = { "Very negative", "Slightly negative", "Neutral", "Slightly positive", "Very positive" };
    char const* d_descAro[5] = { "Very passive", "Slightly passive", "Neutral", "Slightly active", "Very active" };
    bool fullWasPlaying = false;
    bool subWasPlaying = false;
    bool sliderWasPressed = false;
    std::vector <QPushButton*> d_valBtns;
    QButtonGroup* valGroup;
    std::vector <QPushButton*> d_aroBtns;
    QButtonGroup* aroGroup;
    std::vector <QPushButton*> d_labBtns;
    QButtonGroup* labGroup;
    char const* d_valCol[5] = { "#ffc7c7", "#ffe6e6", "#dedede", "#e8ffe6", "#b6fdb1" };
    char const* d_aroCol[5] = { "#b8abff", "#e0d8ff", "#dedede", "#fffab0", "#ffe06f" };
    char const* d_labCol[8] = { "#dedede", "#ffc7c7", "#b8abff", "#b6fdb1", "#ffc675", "#ffc1e6", "#e7ff7d",  "#f4f4f4"};

public:
    explicit annotateVideoEmotion(QWidget* parent = 0);
    ~annotateVideoEmotion();

private slots:
    void loadVideo(QString url);
    void valAroReference();
    void labelReference();
    void copyPrevious();
    void previous();
    void next();
    void move(int pos);
    void fullPlayPause();
    void subPlayPause();
    void subPlay();
    void locationResult();
    void changeAnnotator();
    void subDurationChanged(float dur);
    void fullDurationChanged(float dur);
    void subPositionChanged(float pos);
    void fullPositionChanged(float pos);
    void sliderPressed();
    void sliderReleased();
    void sliderValueChanged();
    void subMoveEdit();
    void totalMoveEdit();
    void resizeTable();

private:
    Ui::annotateVideoEmotion* ui;
    QMediaPlayer* mpFull;
    QMediaPlaylist* plFull;
    QMediaPlayer* mpSub;
    QMediaPlaylist* plSub;

    void deletePlayer();
    void reset();
    bool fileExists(const std::filesystem::path name);
    void setState();
    void videoSpeed();
    bool saveResult();
    bool writeFile();
    void updateStats(); 
    void updateTable();
    void updateTableHeaders(int col, QStringList row);
    void updateSubSliderLabels(int col);
    void setSelection();
    int selectedValence();
    int selectedArousal();
    int selectedLabel();
};

#endif