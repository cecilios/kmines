/*
    Copyright 2007 Dmitry Suzdalev <dimsuz@gmail.com>
    Copyright 2010 Brian Croom <brian.s.croom@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "mainwindow.h"
#include "settings.h"

#include <KGameClock>
#include <KgDifficulty>
#include <KStandardGameAction>
#include <KActionCollection>
#include <KToggleAction>
#include <KStatusBar>
#include <KScoreDialog>
#include <KConfigDialog>
#include <KgThemeProvider>
#include <KgThemeSelector>
#include <KMessageBox>
#include <KLocale>
#include <QDesktopWidget>

#include "canvaswidget.h"
#include "ui_customgame.h"
#include "ui_generalopts.h"

#define MINIMAL_FREE 10

static KgThemeProvider* provider()
{
    KgThemeProvider* prov = new KgThemeProvider;
    prov->discoverThemes("appdata", QLatin1String("themes"));
    return prov;
}

/*
 * Classes for config dlg pages
 */
class CustomGameConfig : public QWidget
{
    Q_OBJECT

public:
    CustomGameConfig(QWidget *parent)
        : QWidget(parent)
    {
        ui.setupUi(this);
        connect(ui.kcfg_CustomWidth, SIGNAL(valueChanged(int)), this, SLOT(updateMaxMines()));
        connect(ui.kcfg_CustomHeight, SIGNAL(valueChanged(int)), this, SLOT(updateMaxMines()));
    }

private slots:
    void updateMaxMines()
    {
        int width = ui.kcfg_CustomWidth->value();
        int height = ui.kcfg_CustomHeight->value();
        int max = qMax(1, width * height - MINIMAL_FREE);
        ui.kcfg_CustomMines->setMaximum(max);
    }

private:
    Ui::CustomGameConfig ui;
};

class GeneralOptsConfig : public QWidget
{
public:
    GeneralOptsConfig(QWidget *parent)
        : QWidget(parent)
    {
        ui.setupUi(this);
    }

private:
    Ui::GeneralOptsConfig ui;
};

/*
 * Main window
 */

KMinesMainWindow::KMinesMainWindow() :
    m_provider(provider()),
    m_canvas(new CanvasWidget(this))
{
    m_provider->setDeclarativeEngine("themeProvider", m_canvas->engine());
    m_canvas->init();

    connect(m_canvas, SIGNAL(minesCountChanged(int,int)), SLOT(onMinesCountChanged(int,int)));
    connect(m_canvas, SIGNAL(gameOver(bool)), SLOT(onGameOver(bool)));
    connect(m_canvas, SIGNAL(firstClickDone()), SLOT(onFirstClick()));

    m_gameClock = new KGameClock(this, KGameClock::MinSecOnly);
    connect(m_gameClock, SIGNAL(timeChanged(QString)), SLOT(advanceTime(QString)));

    statusBar()->insertItem( i18n("Mines: 0/0"), 0 );
    statusBar()->insertItem( i18n("Time: 00:00"), 1);

    setCentralWidget(m_canvas);

    setupActions();

    newGame();
}

void KMinesMainWindow::setupActions()
{
    KStandardGameAction::gameNew(this, SLOT(newGame()), actionCollection());
    KStandardGameAction::highscores(this, SLOT(showHighscores()), actionCollection());

    KStandardGameAction::quit(this, SLOT(close()), actionCollection());
    KStandardAction::preferences( this, SLOT(configureSettings()), actionCollection() );
    m_actionPause = KStandardGameAction::pause( this, SLOT(pauseGame(bool)), actionCollection() );

    Kg::difficulty()->addStandardLevelRange(
        KgDifficultyLevel::Easy, KgDifficultyLevel::Hard
    );
    Kg::difficulty()->addLevel(new KgDifficultyLevel(1000,
        QByteArray( "Custom" ), i18n( "Custom" )
    ));
    KgDifficultyGUI::init(this);
    connect(Kg::difficulty(), SIGNAL(currentLevelChanged(const KgDifficultyLevel*)), SLOT(newGame()));

    setupGUI(qApp->desktop()->availableGeometry().size()*0.4);
}

void KMinesMainWindow::onMinesCountChanged(int count, int total)
{
    statusBar()->changeItem( i18n("Mines: %1/%2", count, total), 0 );
}

void KMinesMainWindow::newGame()
{
    m_gameClock->restart();
    m_gameClock->pause(); // start only with the 1st click

    // some things to manage pause
    if( m_actionPause->isChecked() )
    {
            m_canvas->setGamePaused(false);
            m_actionPause->setChecked(false);
    }
    m_actionPause->setEnabled(false);

    Kg::difficulty()->setGameRunning(false);
    switch(Kg::difficultyLevel())
    {
        case KgDifficultyLevel::Easy:
            m_canvas->startNewGame(9, 9, 10);
            break;
        case KgDifficultyLevel::Medium:
            m_canvas->startNewGame(16,16,40);
            break;
        case KgDifficultyLevel::Hard:
            m_canvas->startNewGame(16,30,99);
            break;
        case KgDifficultyLevel::Custom:
            m_canvas->startNewGame(Settings::customHeight(),
                                  Settings::customWidth(),
                                  Settings::customMines());
        default:
            //unsupported
            break;
    }
    statusBar()->changeItem( i18n("Time: 00:00"), 1);
}

void KMinesMainWindow::onGameOver(bool won)
{
    m_gameClock->pause();
    m_actionPause->setEnabled(false);
    Kg::difficulty()->setGameRunning(false);
    if(won)
    {
        KScoreDialog scoreDialog(KScoreDialog::Name | KScoreDialog::Time, this);
        scoreDialog.initFromDifficulty(Kg::difficulty());
        scoreDialog.hideField(KScoreDialog::Score);

        KScoreDialog::FieldInfo scoreInfo;
        // score-in-seconds will be hidden
        scoreInfo[KScoreDialog::Score].setNum(m_gameClock->seconds());
        //score-as-time will be shown
        scoreInfo[KScoreDialog::Time] = m_gameClock->timeString();

        // we keep highscores as number of seconds
        if( scoreDialog.addScore(scoreInfo, KScoreDialog::LessIsMore) != 0 )
            scoreDialog.exec();
    }
}

void KMinesMainWindow::advanceTime(const QString& timeStr)
{
    statusBar()->changeItem( i18n("Time: %1", timeStr), 1 );
}

void KMinesMainWindow::onFirstClick()
{
    // enable pause action
    m_actionPause->setEnabled(true);
    // start clock
    m_gameClock->resume();
    Kg::difficulty()->setGameRunning(true);
}

void KMinesMainWindow::showHighscores()
{
    KScoreDialog scoreDialog(KScoreDialog::Name | KScoreDialog::Time, this);
    scoreDialog.initFromDifficulty(Kg::difficulty());
    scoreDialog.hideField(KScoreDialog::Score);
    scoreDialog.exec();
}

void KMinesMainWindow::configureSettings()
{
    if ( KConfigDialog::showDialog( QLatin1String(  "settings" ) ) )
        return;
    KConfigDialog *dialog = new KConfigDialog( this, QLatin1String( "settings" ), Settings::self() );
    dialog->addPage( new GeneralOptsConfig( dialog ), i18n("General"), QLatin1String( "games-config-options" ));
    dialog->addPage( new KgThemeSelector( m_provider ), i18n( "Theme" ), QLatin1String( "games-config-theme" ));
    dialog->addPage( new CustomGameConfig( dialog ), i18n("Custom Game"), QLatin1String( "games-config-custom" ));
    connect( dialog, SIGNAL(settingsChanged(QString)), m_canvas, SLOT(updateUseQuestionMarks()) );
    dialog->setHelp(QString(),QLatin1String( "kmines" ));
    dialog->show();
}

void KMinesMainWindow::pauseGame(bool paused)
{
    m_canvas->setGamePaused( paused );
    if( paused )
        m_gameClock->pause();
    else
        m_gameClock->resume();
}

#include "mainwindow.moc"
#include "moc_mainwindow.cpp"
