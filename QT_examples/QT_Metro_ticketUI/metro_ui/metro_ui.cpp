#include "metro_ui.h"
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QCompleter>
#include <QDebug>
#include <QGestureEvent>

metro_ui::metro_ui(QWidget *parent)
    : QMainWindow(parent)
{
    this->resize(280, 374);
    setWindowIcon(QIcon(":/metro_ui/assets/metro_icon.png"));

    button = new QPushButton("Submit", this);
    button->setFixedSize(200, 40);
    button->setEnabled(false); // Initially disable the button
    connect(button, &QPushButton::clicked, this, &metro_ui::submitForm);

    // Read locations/streets from an external file
    QStringList locations;
    QFile file(":/metro_ui/assets/vienna_streets.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            locations.append(line);
        }
        file.close();
    } else {
        qDebug() << "Failed to open locations file.";
    }

    departureBox = new QLineEdit(this);
    arrivalBox = new QLineEdit(this);

    QCompleter *completer = new QCompleter(locations, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    departureBox->setCompleter(completer);
    arrivalBox->setCompleter(completer);

    // call current time function for DateTime Widget
    dateTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime(), this);

    passengerCount = new QSpinBox(this);
    passengerCount->setRange(1, 10);

    passengerInfo = new QLineEdit(this);

    ticketType = new QComboBox(this);
    QStringList ticketTypes = {"Adult", "Child", "Senior"};
    ticketType->addItems(ticketTypes);

    QVBoxLayout *vLayout = new QVBoxLayout;

    QHBoxLayout *titleLayout = new QHBoxLayout;

    QLabel *labelTitle = new QLabel("Ticket System");

    QLabel *labelImage = new QLabel;

    QPixmap pixmap(":/metro_ui/assets/metro.png");

    // Resize the pixmap to the desired size
    int width = 100;
    int height = 100;
    QPixmap scaledPixmap = pixmap.scaled(width, height, Qt::KeepAspectRatio);

    // Set the scaled pixmap to the QLabel
    labelImage->setPixmap(scaledPixmap);

    titleLayout->addWidget(labelTitle);
    titleLayout->addWidget(labelImage);

    vLayout->addLayout(titleLayout);

    QFont fontRegular;
    fontRegular.setBold(true);

    QFont fontTitle;
    fontTitle.setBold(true);
    fontTitle.setPointSize(22);
    labelTitle->setStyleSheet("color: red;");

    QHBoxLayout *hLayoutDeparture = new QHBoxLayout;
    QLabel *labelDeparture = new QLabel("From:");
    hLayoutDeparture->addWidget(labelDeparture);
    hLayoutDeparture->addWidget(departureBox);

    QHBoxLayout *hLayoutArrival = new QHBoxLayout;
    QLabel *labelArrival = new QLabel("To:");
    hLayoutArrival->addWidget(labelArrival);
    hLayoutArrival->addWidget(arrivalBox);

    labelTitle->setFont(fontTitle);
    labelDeparture->setFont(fontRegular);
    labelDeparture->setStyleSheet("color: white;");
    labelArrival->setFont(fontRegular);
    labelArrival->setStyleSheet("color: white;");

    vLayout->addLayout(hLayoutDeparture);
    vLayout->addLayout(hLayoutArrival);
   
    // Add spacer item with height -15
    QSpacerItem *newVSpacer = new QSpacerItem(0, -15, QSizePolicy::Expanding, QSizePolicy::Minimum);

    // Widgets for the QVBoxLayout
    QLabel *labelTime = new QLabel("Date and Time:");
    QLabel *labelNrPsg = new QLabel("Number of Passengers:");
    QLabel *labelPsgInfo = new QLabel("Passenger Information:");
    QLabel *labelType = new QLabel("Ticket Type:");

    // Increase spacing between widgets
    vLayout->setSpacing(15);

    vLayout->addWidget(labelTime);
    labelTime->setMargin(0);
    labelTime->setFont(fontRegular);
    labelTime->setStyleSheet("color: white;");
    vLayout->addItem(newVSpacer);
    vLayout->addWidget(dateTimeEdit);

    vLayout->addWidget(labelNrPsg);
    labelNrPsg->setFont(fontRegular);
    labelNrPsg->setStyleSheet("color: white;");
    vLayout->addItem(newVSpacer);
    vLayout->addWidget(passengerCount);

    vLayout->addWidget(labelPsgInfo);
    labelPsgInfo->setFont(fontRegular);
    labelPsgInfo->setStyleSheet("color: white;");
    vLayout->addItem(newVSpacer);
    vLayout->addWidget(passengerInfo);

    vLayout->addWidget(labelType);
    labelType->setFont(fontRegular);
    labelType->setStyleSheet("color: white;");
    vLayout->addItem(newVSpacer);
    vLayout->addWidget(ticketType);

    vLayout->addWidget(button);

    // Set the layout on the window
    QWidget *window = new QWidget;
    window->setLayout(vLayout);
    setCentralWidget(window);

    // Set background
    QImage backgroundImage(":/metro_ui/assets/bg_blue.jpg");
    window->setAutoFillBackground(true);

    // Create a QBrush with the image to prevent tiling
    QBrush brush(QPixmap::fromImage(backgroundImage));

    // Create a QPalette with the brush and set on the window
    QPalette palette;
    palette.setBrush(QPalette::Window, brush);
    window->setPalette(palette);

    // Connect signals to the slot to enable/disable the submit button
    connect(departureBox, &QLineEdit::textChanged, this, &metro_ui::checkFields);
    connect(arrivalBox, &QLineEdit::textChanged, this, &metro_ui::checkFields);
    connect(dateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &metro_ui::checkFields);
    connect(passengerCount, QOverload<int>::of(&QSpinBox::valueChanged), this, &metro_ui::checkFields);
    connect(passengerInfo, &QLineEdit::textChanged, this, &metro_ui::checkFields);
    connect(ticketType, &QComboBox::currentTextChanged, this, &metro_ui::checkFields);

    checkFields();
}

metro_ui::~metro_ui()
{}

// receiver function Info Box after submition
void metro_ui::submitForm()
{
    QString departure = departureBox->text();
    QString arrival = arrivalBox->text();
    QString dateTime = dateTimeEdit->dateTime().toString();
    int passengers = passengerCount->value();
    QString passengerDetails = passengerInfo->text();
    QString ticket = ticketType->currentText();

    QMessageBox::information(this, "Ticket", "Departure: " + departure + "\nArrival: " + arrival + "\nDate and Time: " + dateTime + "\nNumber of Passengers: " + QString::number(passengers) + "\nPassenger Information: " + passengerDetails + "\nTicket Type: " + ticket);
}

// check-function for button activation
void metro_ui::checkFields()
{
    bool allFieldsFilled = 
        !departureBox->text().isEmpty() &&
        !arrivalBox->text().isEmpty() &&
        dateTimeEdit->dateTime().isValid() &&
        passengerCount->value() > 0 &&
        !passengerInfo->text().isEmpty() &&
        !ticketType->currentText().isEmpty();

    button->setEnabled(allFieldsFilled);
}


