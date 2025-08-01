/*
*  This file is part of openauto project.
*  Copyright (C) 2018 f1x.studio (Michal Szwaj)
*
*  openauto is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 3 of the License, or
*  (at your option) any later version.

*  openauto is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with openauto. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <QFile>
#include <QMainWindow>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>
#include <memory>

#include <QDir>
#include <QDirIterator>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMediaMetaData>
#include <QMediaPlayer>

#include <QAudioProbe>
#include <QMediaPlaylist>
#include <QMediaService>
#include <QVideoProbe>

#include <QFileDialog>

#include <qmediaplayer.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>

#include <QFileSystemWatcher>
#include <QKeyEvent>

#include <QBluetoothLocalDevice>
// #include <QtBluetooth>

namespace Ui {
class MainWindow;
}

namespace f1x {
namespace openauto {
namespace autoapp {
namespace ui {

class MainWindow : public QMainWindow {
    Q_OBJECT
  public:
    explicit MainWindow(configuration::IConfiguration::Pointer configuration,
                        QWidget *parent = nullptr);
    ~MainWindow() override;
    QMediaPlayer *player;
    QFileSystemWatcher *watcher;
    QFileSystemWatcher *watcher_tmp;

  signals:
    void exit();
    void reboot();
    void openSettings();
    void toggleCursor();
    void TriggerScriptDay();
    void TriggerScriptNight();
    void cameraShow();
    void cameraHide();
    void cameraStop();
    void cameraSave();
    void cameraRecord();
    void cameraPosYUp();
    void cameraPosYDown();
    void cameraZoomPlus();
    void cameraZoomMinus();
    void cameraFlipX();
    void cameraFlipY();
    void openConnectDialog();
    void openWifiDialog();
    void openUpdateDialog();
    void showBrightnessSlider();
    void showVolumeSlider();
    void showAlphaSlider();
    void showRearCam();
    void hideRearCam();
    void TriggerAppStart();
    void TriggerAppStop();
    void CloseAllDialogs();

  private slots:
    void on_horizontalSliderBrightness_valueChanged(int value);
    void on_horizontalSliderVolume_valueChanged(int value);
    void updateAlpha();

  private slots:
    void on_pushButtonBrightness_clicked();
    void on_pushButtonBrightness2_clicked();
    void on_pushButtonVolume_clicked();
    void on_pushButtonVolume2_clicked();
    void switchGuiToDay();
    void switchGuiToNight();
    void showTime();
    void cameraControlShow();
    void cameraControlHide();
    void toggleExit();
    void createDebuglog();
    void setPairable();
    void toggleMuteButton();
    void toggleGUI();
    void setMute();
    void setUnMute();
    void customButtonPressed1();
    void customButtonPressed2();
    void customButtonPressed3();
    void customButtonPressed4();
    void customButtonPressed5();
    void customButtonPressed6();
    void playerShow();
    void playerHide();
    void updateBG();

    void on_horizontalSliderProgressPlayer_sliderMoved(int position);
    void on_horizontalSliderVolumePlayer_sliderMoved(int position);
    void on_pushButtonList_clicked();
    void on_pushButtonPlayerStop_clicked();
    void on_pushButtonPlayerPause_clicked();
    void on_positionChanged(qint64 position);
    void on_durationChanged(qint64 position);
    void on_mp3List_itemClicked(QListWidgetItem *item);
    void metaDataChanged();
    void on_pushButtonPlayerPlayList_clicked();
    void on_pushButtonPlayerNextBig_clicked();
    void on_pushButtonPlayerPrevBig_clicked();
    void on_pushButtonPlayerPrevAlbum_clicked();
    void on_pushButtonPlayerNextAlbum_clicked();
    void on_pushButtonBackToPlayer_clicked();
    void on_comboBoxAlbum_currentIndexChanged(const QString &arg1);
    void on_mp3List_currentRowChanged(int currentRow);
    void on_StateChanged(QMediaPlayer::State state);
    void scanFolders();
    void scanFiles();
    void tmpChanged();
    void setTrigger();
    void setRetryUSBConnect();
    void resetRetryUSBMessage();
    void updateNetworkInfo();
    bool check_file_exist(const char *filename);
    void hostModeStateChanged(QBluetoothLocalDevice::HostMode);

    // void on_AlbumCoverListView_clicked(const QModelIndex &index);
    void on_AlbumCoverListView_clicked(const QModelIndex &index);

    void on_pushButtonAlbum_clicked();

  private:
    Ui::MainWindow *ui_;
    configuration::IConfiguration::Pointer configuration_;

    QString brightnessFilename = "/sys/class/backlight/rpi_backlight/brightness";
    QString brightnessFilenameAlt = "/tmp/custombrightness";
    QFile *brightnessFile;
    QFile *brightnessFileAlt;
    char brightness_str[6];
    char volume_str[6];
    int alpha_current_str;
    QString bversion;
    QString bdate;

    char nightModeFile[32] = "/tmp/night_mode_enabled";
    char devModeFile[32] = "/tmp/dev_mode_enabled";
    char wifiButtonFile[32] = "/etc/button_wifi_visible";
    char cameraButtonFile[32] = "/etc/button_camera_visible";
    char brightnessButtonFile[32] = "/etc/button_brightness_visible";
    char debugModeFile[32] = "/tmp/usb_debug_mode";
    char lsFile[32] = "/etc/cs_lightsensor";

    char custom_button_file_c1[26] = "/boot/crankshaft/button_1";
    char custom_button_file_c2[26] = "/boot/crankshaft/button_2";
    char custom_button_file_c3[26] = "/boot/crankshaft/button_3";
    char custom_button_file_c4[26] = "/boot/crankshaft/button_4";
    char custom_button_file_c5[26] = "/boot/crankshaft/button_5";
    char custom_button_file_c6[26] = "/boot/crankshaft/button_6";

    QString custom_button_command_c1;
    QString custom_button_command_c2;
    QString custom_button_command_c3;
    QString custom_button_command_c4;
    QString custom_button_command_c5;
    QString custom_button_command_c6;

    QString custom_button_color_c1 = "186,189,192";
    QString custom_button_color_c2 = "186,189,192";
    QString custom_button_color_c3 = "186,189,192";
    QString custom_button_color_c4 = "186,189,192";
    QString custom_button_color_c5 = "186,189,192";
    QString custom_button_color_c6 = "186,189,192";

    QString selectedMp3file;
    QString musicfolder = "/media/CSSTORAGE/Music";
    QString albumfolder = "/";
    QString date_text;

    QMediaPlaylist *playlist;

    bool customBrightnessControl = false;

    bool wifiButtonForce = false;
    bool cameraButtonForce = false;
    bool brightnessButtonForce = false;

    bool nightModeEnabled = false;
    bool DayNightModeState = false;

    bool devModeEnabled = false;

    bool wallpaperDayFileExists = false;
    bool wallpaperNightFileExists = false;
    bool wallpaperClassicDayFileExists = false;
    bool wallpaperClassicNightFileExists = false;
    bool wallpaperEQFileExists = false;

    bool exitMenuVisible = false;

    bool rearCamEnabled = false;
    bool rearCamVisible = false;

    bool dashCamRecording = false;
    bool systemDebugmode = false;

    bool bluetoothEnabled = false;

    bool toggleMute = false;
    bool oldGUIStyle = false;
    bool UseBigClock = false;
    bool NoClock = false;

    bool c1ButtonForce = false;
    bool c2ButtonForce = false;
    bool c3ButtonForce = false;
    bool c4ButtonForce = false;
    bool c5ButtonForce = false;
    bool c6ButtonForce = false;

    bool hotspotActive = false;
    int currentPlaylistIndex = 0;
    bool background_set = false;
    bool mediacontentchanged = true;

    bool lightsensor = false;
    bool holidaybg = false;

    bool csmtupdate = false;
    bool udevupdate = false;
    bool openautoupdate = false;
    bool systemupdate = false;

    int camera_ycorection;
    int camera_zoom;

    QBluetoothLocalDevice *localDevice;

  protected:
    void keyPressEvent(QKeyEvent *event) override;
};

}  // namespace ui
}  // namespace autoapp
}  // namespace openauto
}  // namespace f1x
