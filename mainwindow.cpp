#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QCheckBox>
#include <QMap>


MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , bridge(nullptr)
    , lights(nullptr)
    , groups(nullptr)
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

    QVBoxLayout* roomLayout = new QVBoxLayout;
    globalLayout->addLayout(roomLayout);

    QTimer* rainbowTimer = new QTimer(this);
    QMap<QString, QCheckBox*> roomMap;

    connect(connectButton, &QPushButton::clicked, this,
            [this, ipEdit, usernameEdit, rainbowTimer, statusLabel, &roomMap, roomLayout] ()
    {
        bridge = new HueBridge(ipEdit->text(), usernameEdit->text());

        HueBridge::ConnectionStatus status;
        if (bridge->testConnection(status)) {
            statusLabel->setText("<b><FONT COLOR = '#006400'>Connected</b>");

            lights = new HueLightList(HueLight::discoverLights(bridge));
            groups = new HueGroupList(HueGroup::discoverGroups(bridge));

            for (auto light : *lights) {
                light->setBrightness(254);
                light->setSaturation(1);
                light->setAlert(HueAbstractObject::Breathe15Sec);
            }

            int index = 0;
            for (auto group : *groups) {
                QCheckBox* roomCheck = new QCheckBox(group->name().getName());
                roomLayout->addWidget(roomCheck);
            }

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

            lights = nullptr;
            rainbowTimer->stop();
        }
    });

    rainbowTimer->setInterval(1000);
    connect(rainbowTimer, &QTimer::timeout, this,
            [this] ()
    {
        static int rainbowHue[3] = {
            65535,  // red
            21845,  // green
            43690   // blue
                            };
        static int offset = 0;

        if (lights == nullptr)
            return;

        int index = 0;
        for (auto light : *lights)
            light->setHue(rainbowHue[(index++ + offset) % 3]);

        offset++;
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
