#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTimer>

#include "../HueLib/HueLib/huelib.h"


MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QVBoxLayout* globalLayout = new QVBoxLayout;

    QFormLayout* connectLayout = new QFormLayout;
    QLineEdit* ipEdit = new QLineEdit("10.0.0.37");
    QLineEdit* usernameEdit = new QLineEdit("rqHvMLAJK3bcGy0mzXAStU3zq6D2CGGMc3i9eVxS");
    connectLayout->addRow(tr("&IP"), ipEdit );
    connectLayout->addRow(tr("&Username"), usernameEdit);
    globalLayout->addLayout(connectLayout);

    QLabel* statusLabel = new QLabel("");
    globalLayout->addWidget(statusLabel);

    QPushButton* connectButton = new QPushButton("&Connect");
    globalLayout->addWidget(connectButton);

    connect(connectButton, &QPushButton::clicked, this,
            [this, ipEdit, usernameEdit, statusLabel] ()
    {
        HueBridge* bridge = new HueBridge(ipEdit->text(),usernameEdit->text());

        HueBridge::ConnectionStatus status;

        if (bridge->testConnection(status)) {
            statusLabel->setText("<b><FONT COLOR = '#006400'>Connected</b>");

            HueLightList* lights = new HueLightList(HueLight::discoverLights(bridge));

            QTimer* rainbowTimer = new QTimer(this);
            connect(rainbowTimer, &QTimer::timeout, this,
                    [lights] ()
            {
                static int rainbowHue[3] = {
                    65535,  // red
                    21845,  // green
                    43690   // blue
                                    };
                static int offset = 0;

                int index = 0;
                for( auto light : *lights ) {
                    light->setHue(rainbowHue[(index++ + offset) % 3]);
                }
                offset++;
            });

            rainbowTimer->setInterval(1000);
            rainbowTimer->start();

        }
        else {
            switch (status) {
            case HueBridge::HttpError:
            case HueBridge::TimedOut:
                statusLabel->setText("<b><FONT COLOR = '#ff0000'>Incorrect IP address</b>");
                break;
            case HueBridge::JsonError:
                statusLabel->setText("<b><FONT COLOR = '#ff0000'>Incorrect username</b>");
                break;
            case HueBridge::Unknown:
            default:
                statusLabel->setText("<b><FONT COLOR = '#ff0000'>Unknown error</b>");
                break;
            }
        }
    });

    globalLayout->addStretch();

    QWidget* window = new QWidget;
    window->setLayout(globalLayout);

    setCentralWidget(window);
    resize(width(), minimumHeight());
    setWindowTitle("Always, I wanna be with you");

}

MainWindow::~MainWindow()
{
    delete ui;
}
